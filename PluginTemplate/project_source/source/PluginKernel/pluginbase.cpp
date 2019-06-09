// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  pluginbase.cpp
//
/**
    \file   pluginbase.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class implementation file for ASPiK pluginbase object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "pluginbase.h"

/**
\brief PluginBase constructor

Operation:
- flush frame buffers
- pluginHostConnector reset
- NOTE: most initializations done in .h file
*/
PluginBase::PluginBase()
{
    memset(&inputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
    memset(&outputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
    memset(&auxInputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
    memset(&auxOutputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);

    pluginHostConnector = nullptr;
}

/**
\brief PluginBase destructor

Operation:
- destroy all plugin parameters and remove pointers from all lists
- clean out I/O combinations
- remove presets
*/
PluginBase::~PluginBase()
{
	if (pluginDescriptor.supportedIOCombinations)
		delete[] pluginDescriptor.supportedIOCombinations;
	pluginDescriptor.supportedIOCombinations = 0;

	if (pluginDescriptor.supportedAuxIOCombinations)
		delete[] pluginDescriptor.supportedAuxIOCombinations;
	pluginDescriptor.supportedAuxIOCombinations = 0;

    removeAllPresets();

    for(std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it) {
        delete *it;
    }
    pluginParameters.clear();
    pluginParameterMap.clear();
	delete [] pluginParameterArray;
	delete [] smoothablePluginParameters;
	delete [] outboundPluginParameters;
}

/**
\brief initialize object for a new run of audio; called just before audio streams

Operation:
- update sample-rate dependent objects
- PluginParameter will update its smoother (its only sample rate dependent member)
*/
bool PluginBase::reset(ResetInfo& resetInfo)
{
	// --- update param smoothers
	for (std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it)
	{
		PluginParameter* piParam = *it;
		if (piParam)
			piParam->updateSampleRate(resetInfo.sampleRate);
	}

	return true;
}

/**
\brief one-time initialize function called after object creation and before the first reset( ) call

Operation:
- saves structure for the plugin to use; you can also load WAV files or state information here
*/
bool PluginBase::initialize(PluginInfo& _pluginInfo)
{
	// --- save a copy for the plugin
	pluginInfo = _pluginInfo;

	return true;
}

/**
\brief initialize object for a new run of audio; called just before audio streams

Operation:
- iterate through parameters and copy their values into the bound variables you set up
- then, call the postUpdatePluginParameter method to do any post-update cooking required to use the variable for processing
*/
void PluginBase::syncInBoundVariables()
{
	ParameterUpdateInfo info;
	info.bufferProcUpdate = true;
	info.boundVariableUpdate = true;

	// --- rip through and synch em
	for (unsigned int i = 0; i < numPluginParameters; i++)
	{
		if (pluginParameterArray[i] && pluginParameterArray[i]->updateInBoundVariable())
		{
			postUpdatePluginParameter(pluginParameterArray[i]->getControlID(), pluginParameterArray[i]->getControlValue(), info);
		}
	}
}

/**
\brief THE buffer processing function.

Operation:
- break channel buffers into frames (one sample from each channel, in and out)
- call the pre-processing function on derived class to allow it to prepare for the audio buffer's arrival
- call the frame processing function that the derived class MUST implement repeatedly until the buffer is processed
- call the post-processing function on derived class to allow it to do any stuff that requires the buffer to be processed first
- NOTE: if you want the derived class to process buffers instead of frames, override and implement THIS function in the derived class

\param processBufferInfo - a structure of information about the current buffer to process; includes information from host (BPM, etc...)

\return true if operation succeeds, false otherwise
*/
bool PluginBase::processAudioBuffers(ProcessBufferInfo& processBufferInfo)
{
	memset(&inputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
	memset(&outputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
	memset(&auxInputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);
	memset(&auxOutputFrame, 0, sizeof(float)*MAX_CHANNEL_COUNT);

	double sampleInterval = 1.0 / audioProcDescriptor.sampleRate;

	if (pluginDescriptor.processFrames)
	{
		ProcessFrameInfo info;

		info.audioInputFrame = &inputFrame[0];
		info.audioOutputFrame = &outputFrame[0];
		info.auxAudioInputFrame = &auxInputFrame[0];
		info.auxAudioOutputFrame = &auxOutputFrame[0];

		info.channelIOConfig = processBufferInfo.channelIOConfig;
		info.auxChannelIOConfig = processBufferInfo.auxChannelIOConfig;

		info.numAudioInChannels = processBufferInfo.numAudioInChannels;
		info.numAudioOutChannels = processBufferInfo.numAudioOutChannels;
		info.numAuxAudioInChannels = processBufferInfo.numAuxAudioInChannels;
		info.numAuxAudioOutChannels = processBufferInfo.numAuxAudioOutChannels;

		info.hostInfo = processBufferInfo.hostInfo;
		info.midiEventQueue = processBufferInfo.midiEventQueue;

		// --- sync internal bound variables
		preProcessAudioBuffers(processBufferInfo);

		// --- build frames, one sample from each channel
		for (uint32_t frame = 0; frame<processBufferInfo.numFramesToProcess; frame++)
		{
			for (uint32_t i = 0; i<processBufferInfo.numAudioInChannels; i++)
			{
				inputFrame[i] = processBufferInfo.inputs[i][frame];
			}

			for (uint32_t i = 0; i<processBufferInfo.numAuxAudioInChannels; i++)
			{
				auxInputFrame[i] = processBufferInfo.auxInputs[i][frame];
			}

			info.currentFrame = frame;

			// -- process the frame of data
			processAudioFrame(info);

			for (uint32_t i = 0; i<processBufferInfo.numAudioOutChannels; i++)
			{
				processBufferInfo.outputs[i][frame] = outputFrame[i];
			}
			for (uint32_t i = 0; i<processBufferInfo.numAuxAudioOutChannels; i++)
			{
				processBufferInfo.auxOutputs[i][frame] = auxOutputFrame[i];
			}

			// --- update per-frame
			info.hostInfo->uAbsoluteFrameBufferIndex += 1;
			info.hostInfo->dAbsoluteFrameBufferTime += sampleInterval;
		}

		// --- generally not used
		postProcessAudioBuffers(processBufferInfo);

		return true; /// processed
	}

	return false; /// processed
}

/**
\brief copy newly updated metering variables into GUI parameters for display

Operation:
- simple loop to update meter variables
- to display custom data (waveforms, histograms, etc...) see Custom Views example in ASPiK SDK

\return true if at least one parameter was updated, false otherwise
*/
bool PluginBase::updateOutBoundVariables()
{
	bool updated = false;

	// --- rip through and synch em
	for (unsigned int i = 0; i < numOutboundPluginParameters; i++)
	{
		if (outboundPluginParameters[i])
		{
			outboundPluginParameters[i]->updateOutBoundVariable();
			updated = true; // sticky
		}
	}
	return updated;
}



/**
\brief combines parameter smoothing and VST3 sample accurate updates

NOTE:
- beware: this function can eat a lot of CPU if the sample accurate updates trigger complex cooking functions
- to combat CPU usage, you can set the VST3 sample granularity in initPluginDescriptors() apiSpecificInfo.vst3SampleAccurateGranularity
- you can also change the parameter smoothing granularity
- but in either case, the list MUST be iterated; this function is the reason for the old-fashioned C-array of pointers\n
  as it was found to be faster than any other list method for entire-list iteration (if you have a faster way, let me knmow!)
- the parameter is updated with the smoothed value
- the post-parameter update function is then called (complex cooking functions here will eat the CPU as well)
*/
void PluginBase::doSampleAccurateParameterUpdates()
{
	if (numSmoothablePluginParameters == 0)
		return;

	// --- do updates
	double value = 0;
	bool vstSAAutomated = false;
	ParameterUpdateInfo vst3Update(false, true); /// false = this is NOT called from smoothing operation, true: this is a VST sample accurate update
	vst3Update.isVSTSampleAccurateUpdate = true;

	ParameterUpdateInfo paramSmoothUpdate(true, false); /// true = this is called from smoothing operation, false = NOT VST sample accurate update
	paramSmoothUpdate.isSmoothing = true;

	// --- rip through the array
	for (unsigned int i = 0; i < numSmoothablePluginParameters; i++)
	{
		PluginParameter* piParam = smoothablePluginParameters[i];
		if (piParam)
		{
			// --- VST sample accurate stuff
			if (wantsVST3SampleAccurateAutomation())
			{
				// --- if we get here getParameterUpdateQueue() should be non-null
				//     NOTE you can disable sample accurate automation for each parameter when you set them up if needed
				if (piParam->getParameterUpdateQueue() && piParam->getEnableVSTSampleAccurateAutomation())
				{
					if (piParam->getParameterUpdateQueue()->getNextValue(value))
					{
						piParam->setControlValueNormalized(value, false, true); // false = do not apply taper, true = ignore smoothing (not needed here)
						vstSAAutomated = true;

						// --- now update the bound variable
						if (piParam->updateInBoundVariable())
						{
							vst3Update.boundVariableUpdate = true;
						}
						postUpdatePluginParameter(piParam->getControlID(), piParam->getControlValue(), vst3Update);
					}
				}
			}

			// --- do smoothing, but not if we did a sample accurate automation update!
			if (!vstSAAutomated && piParam->smoothParameterValue())
			{
				// --- update bound variable, if there is one
				if (piParam->updateInBoundVariable())
				{
					paramSmoothUpdate.boundVariableUpdate = true;
				}
				postUpdatePluginParameter(piParam->getControlID(), piParam->getControlValue(), paramSmoothUpdate);
			}
		}
	}
}

/**
\brief adds a new plugin parameter to the parameter map

NOTE:
- initializes parameter smoothing (if enabled for that parameter)

\param piParam pointer to a newly created plugin parameter
\param sampleRate the sample rate, needed to initialize the smoother

\return the index of the parameter in the vector or old-fashioned C-array
*/
int32_t PluginBase::addPluginParameter(PluginParameter* piParam, double sampleRate)
{
	// --- map for controlID-indexing
	pluginParameterMap.insert(std::make_pair(piParam->getControlID(), piParam));

	// --- vector for fast iteration and 0-indexing
	pluginParameters.push_back(piParam);

	// --- first intialization, this can change
	piParam->initParamSmoother(sampleRate);

	return (int32_t)pluginParameters.size() - 1;
}

/**
\brief adds an auxilliary attribute to the plugin parameter; you can have as many auxilliary attributes as you like for each parameter.

\param controlID control ID value of the parameter
\param auxAttribute a structure of information to store
*/
void PluginBase::setParamAuxAttribute(uint32_t controlID, const AuxParameterAttribute& auxAttribute)
{
	PluginParameter* piParam = getPluginParameterByControlID(controlID);
	if (!piParam) return;

	piParam->setAuxAttribute(auxAttribute.attributeID, auxAttribute);
}

/**
\brief get a parameter by type

\param startIndex current start index in array (vector or old-fashioned C-array)
\param controlType type of linked variable (float, double, meter,...)

\return a naked pointer to the PluginParameter object
*/
PluginParameter* PluginBase::getNextParameterOfType(int32_t& startIndex, controlVariableType controlType)
{
	PluginParameter* piParam = nullptr;
	for (uint32_t i = startIndex; i < getPluginParameterCount(); i++)
	{
		piParam = getPluginParameterByIndex(i);
		{
			if (piParam->getControlVariableType() == controlType)
			{
				startIndex = i + 1;
				return piParam;
			}
		}
	}

	startIndex = -1;
	return nullptr;
}

/**
\brief value-as-double

\param controlID control ID of parameter

\return the parameter's value formatted as a double
*/
double PluginBase::getPIParamValueDouble(int32_t controlID)
{
    PluginParameter* piParam = getPluginParameterByControlID(controlID);
    if(!piParam) return 0.0;

    return piParam->getControlValue();
}

/**
\brief value-as-float

\param controlID control ID of parameter

\return the parameter's value formatted as a float
*/
float PluginBase::getPIParamValueFloat(int32_t controlID)
{
    PluginParameter* piParam = getPluginParameterByControlID(controlID);
    if(!piParam) return 0.f;

    return (float)piParam->getControlValue();
}


/**
\brief value-as-int

\param controlID control ID of parameter

\return the parameter's value formatted as an int
*/
int PluginBase::getPIParamValueInt(int32_t controlID)
{
    PluginParameter* piParam = getPluginParameterByControlID(controlID);
    if(!piParam) return 0;

    return (int)piParam->getControlValue();
}

/**
\brief value-as-uint

\param controlID control ID of parameter

\return the parameter's value formatted as a uint
*/
uint32_t PluginBase::getPIParamValueUInt(int32_t controlID)
{
    PluginParameter* piParam = getPluginParameterByControlID(controlID);
    if(!piParam) return 0;

    return (uint32_t)piParam->getControlValue();
}

/**
\brief checks for at least one meter parameter that has the pro tools GR meter flag set

\return true if we have at least one GR meter
*/
bool PluginBase::hasProToolsGRMeters()
{
	for (unsigned int i = 0; i < numOutboundPluginParameters; i++)
	{
		if (outboundPluginParameters[i] && outboundPluginParameters[i]->isProtoolsGRMeter())
		{
			return true;
		}
	}
	return false;
}

/**
\brief  pro tools GR meter is only ever observed to be single meter; this merges all meter values for meters designated as Pro Tools meter

\return the GR meter value (for delivery to Pro Tools or AAX host)
*/
double PluginBase::getProToolsGRValue()
{
	// --- rip through and accumulate into one value
	double accumMeter = 0;
	double count = 0;
	for (unsigned int i = 0; i < numOutboundPluginParameters; i++)
	{
		if (outboundPluginParameters[i] && outboundPluginParameters[i]->isProtoolsGRMeter())
		{
			accumMeter += outboundPluginParameters[i]->getControlValue();
			count++;
		}
	}
	if (count > 0)
		accumMeter /= count;

	return accumMeter;
}

/**
\brief add a new I/O configuration pair

\param ioConfig structure containing one input/output channel pair

\return the new count of I/O combinations
*/
uint32_t PluginBase::addSupportedIOCombination(ChannelIOConfig ioConfig)
{
	// --- inc count
	pluginDescriptor.numSupportedIOCombinations++;
	ChannelIOConfig* configArray = new ChannelIOConfig[pluginDescriptor.numSupportedIOCombinations];

	// --- this is only done once at construction and needs to be old-fashioned array :(
	// --- do we have existing array?
	if (pluginDescriptor.supportedIOCombinations)
	{
		// --- fast block memory copy:
		memcpy(&configArray[0], &pluginDescriptor.supportedIOCombinations[0], sizeof(ChannelIOConfig) * (pluginDescriptor.numSupportedIOCombinations - 1));
		delete[] pluginDescriptor.supportedIOCombinations;
	}

	pluginDescriptor.supportedIOCombinations = configArray;
	pluginDescriptor.supportedIOCombinations[pluginDescriptor.numSupportedIOCombinations - 1] = ioConfig;

	return pluginDescriptor.numSupportedIOCombinations;
}

/**
\brief add a new I/O configuration pair

\param ioConfig structure containing one aux input/output channel pair

\return the new count of aux I/O combinations
*/
uint32_t PluginBase::addSupportedAuxIOCombination(ChannelIOConfig ioConfig)
{
	// --- inc count
	pluginDescriptor.numSupportedAuxIOCombinations++;
	ChannelIOConfig* configArray = new ChannelIOConfig[pluginDescriptor.numSupportedAuxIOCombinations];

	// --- this is only done once at construction and needs to be old-fashioned array :(
	// --- do we have existing array?
	if (pluginDescriptor.supportedAuxIOCombinations)
	{
		// --- fast block memory copy:
		memcpy(&configArray[0], &pluginDescriptor.supportedAuxIOCombinations[0], sizeof(ChannelIOConfig) * (pluginDescriptor.numSupportedAuxIOCombinations - 1));
		delete[] pluginDescriptor.supportedAuxIOCombinations;
	}

	pluginDescriptor.supportedAuxIOCombinations = configArray;
	pluginDescriptor.supportedAuxIOCombinations[pluginDescriptor.numSupportedAuxIOCombinations - 1] = ioConfig;

	return pluginDescriptor.numSupportedAuxIOCombinations;
}

/**
\brief check to see if plugin support a given input channel format

\param channelFormat the format to test (e.g. kCFStereo)

\return true if channel format is supported
*/
bool PluginBase::hasSupportedInputChannelFormat(uint32_t channelFormat)
{
	for (uint32_t i = 0; i< getNumSupportedIOCombinations(); i++)
	{
		if (getChannelInputFormat(i) == channelFormat)
			return true;
	}
	return false;
}

/**
\brief check to see if plugin support a given output channel format

\param channelFormat the format to test (e.g. kCFStereo)

\return true if channel format is supported
*/
bool PluginBase::hasSupportedOutputChannelFormat(uint32_t channelFormat)
{
	for (uint32_t i = 0; i< getNumSupportedIOCombinations(); i++)
	{
		if (getChannelOutputFormat(i) == channelFormat)
			return true;
	}
	return false;
}

/**
\brief get the number of input channels for a configuration at a given index

\param ioConfigIndex of the configuration in our list

\return number of channels
*/
uint32_t PluginBase::getInputChannelCount(uint32_t ioConfigIndex)
{
	if (ioConfigIndex < pluginDescriptor.numSupportedIOCombinations)
		return pluginDescriptor.getChannelCountForChannelIOConfig(pluginDescriptor.supportedIOCombinations[ioConfigIndex].inputChannelFormat);

	return 0;
}

/**
\brief get the number of output channels for a configuration at a given index

\param ioConfigIndex of the configuration in our list

\return number of channels
*/
uint32_t PluginBase::getOutputChannelCount(uint32_t ioConfigIndex)
{
	if (ioConfigIndex < pluginDescriptor.numSupportedIOCombinations)
		return pluginDescriptor.getChannelCountForChannelIOConfig(pluginDescriptor.supportedIOCombinations[ioConfigIndex].outputChannelFormat);

	return 0;
}


/**
\brief set a parameter's value with the actual value (as double)

\param _controlID control ID of the parameter
\param _controlValue the new value to set on the parameter
*/
void PluginBase::setPIParamValue(uint32_t _controlID, double _controlValue)
{
	PluginParameter* piParam = getPluginParameterByControlID(_controlID);
	if (!piParam) return; /// not handled

	// --- set value
	piParam->setControlValue(_controlValue);
}

/**
\brief set a parameter's value with the normalied value (as double)

\param _controlID control ID of the parameter
\param _normalizedValue the new normalized value to set on the parameter
\param applyTaper add the tapering during application (not used in all situations)
*/
double PluginBase::setPIParamValueNormalized(uint32_t _controlID, double _normalizedValue, bool applyTaper)
{
	PluginParameter* piParam = getPluginParameterByControlID(_controlID);
	if (!piParam) return 0.0; /// not handled

							  // --- set value
	return piParam->setControlValueNormalized(_normalizedValue, applyTaper);
}

/**
\brief update a bound-variable with a parameter's control value; this is the essence of the variable binding operation

\param _controlID control ID of the parameter

\return true if variable was updated, false otherwise
*/
bool PluginBase::updatePIParamBoundValue(uint32_t _controlID)
{
	PluginParameter* piParam = getPluginParameterByControlID(_controlID);
	if (!piParam) return false; /// not handled

	// --- update
	return piParam->updateInBoundVariable();
}


/**
\brief delete GUI update structures in a list

\param guiParameters the list to clear
*/
void PluginBase::clearUpdateGUIParameters(std::vector<GUIParameter*>& guiParameters)
{
	// --- delete them
	for (std::vector<GUIParameter*>::iterator it = guiParameters.begin(); it != guiParameters.end(); ++it)
		delete *it;
}


/**
\brief copies parameters into a new list; used to initialize the GUI - note that this makes true, disconnected copies

\param disableSmoothing turn off smoothing in copied parameters

\return a naked pointer to a newly created vector list
*/
std::vector<PluginParameter*>* PluginBase::makePluginParameterVectorCopy(bool disableSmoothing)
{
	std::vector<PluginParameter*>* PluginParameterPtr = new std::vector<PluginParameter*>;

	// --- copy params, do not use copy constructor; it copies pointers
	for (std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it)
	{
		PluginParameter* piParam = new PluginParameter(**it);

		if (disableSmoothing)
			piParam->setParameterSmoothing(false);

		PluginParameterPtr->push_back(piParam);
	}

	return PluginParameterPtr;
}

/**
\brief creates the preset parameter ID/Value pair PresetParameter objects and adds them to supplied list

\param presetParameters list to add presets to
\param disableSmoothing turn off smoothing (reserved for future use)

\return true if at least one preset was added to the list
*/
bool PluginBase::initPresetParameters(std::vector<PresetParameter>& presetParameters, bool disableSmoothing)
{
	bool retVal = false;
	for (std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it)
	{
		PresetParameter preParam((*it)->getControlID(), (*it)->getControlValue());
		presetParameters.push_back(preParam);
		retVal = true;
	}

	return retVal;
}

/**
\brief set a new value for a preset

\param presetParameters list of presets
\param _controlID control ID of parameter to set
\param _controlValue the new preset control value

\return true if preset was found and updated
*/
bool PluginBase::setPresetParameter(std::vector<PresetParameter>& presetParameters, uint32_t _controlID, double _controlValue)
{
	bool foundIt = false;
	for (std::vector<PresetParameter>::iterator it = presetParameters.begin(); it != presetParameters.end(); ++it)
	{
		if ((*it).controlID == _controlID)
		{
			(*it).actualValue = _controlValue;
			foundIt = true;
		}
	}

	return foundIt;
}

/**
\brief gets name as a const char* for connecting with all APIs at some level

\param index index of preset

\return name as const char*
*/
const char* PluginBase::getPresetName(uint32_t index)
{
	if (index < presets.size())
	{
		return presets[index]->presetName.c_str();
	}
	return "";
}

/**
\brief add a new preset

\param preset a PresetInfo strucutre that describes the preset

\return the new numbner of presets in the list
*/
size_t PluginBase::addPreset(PresetInfo* preset)
{
	presets.push_back(preset);
	return presets.size();
}

/**
\brief remove a  preset - NOTE: does not delete object

\param index index of preset to remove
*/
void PluginBase::removePreset(uint32_t index)
{
	if (index < presets.size())
	{
		PresetInfo* preset = presets[index];
		// --- clean up
		for (uint32_t i = 0; i < preset->presetParameters.size(); i++)
			preset->presetParameters.pop_back();

		delete preset;
		presets.erase(presets.begin() + index);
	}
}

/**
\brief remove all presets - NOTE: does not delete objects
*/
void PluginBase::removeAllPresets()
{
	for (std::vector<PresetInfo*>::iterator it = presets.begin(); it != presets.end(); ++it)
	{
		// --- clean up
		for (uint32_t i = 0; i < (*it)->presetParameters.size(); i++)
			(*it)->presetParameters.pop_back();

		delete *it;
	}
	presets.clear();
}

/**
\brief get a preset

\return a naked pointer to the preset object
*/
PresetInfo* PluginBase::getPreset(uint32_t index)
{
	if (index < presets.size())
		return presets[index];

	return nullptr;
}

/**
\brief called at the end of the initialization phase, this function creates the various non map-versions of the parameter lists\n
       and initializes the parameters; this is the final step of construction
*/
void PluginBase::initPluginParameterArray()
{
	if (pluginParameterArray)
		delete[] pluginParameterArray;

	numPluginParameters = pluginParameters.size();
	numSmoothablePluginParameters = 0;
	numOutboundPluginParameters = 0;

	pluginParameterArray = new PluginParameter*[numPluginParameters];
	for (unsigned int i = 0; i < numPluginParameters; i++)
	{
		pluginParameterArray[i] = pluginParameters[i];

		// --- how many are potentially smoothable?
		if (pluginParameters[i]->getControlVariableType() == controlVariableType::kDouble ||
			pluginParameters[i]->getControlVariableType() == controlVariableType::kFloat)
			numSmoothablePluginParameters++;

		// --- how many are outbound?
		if (pluginParameters[i]->getControlVariableType() == controlVariableType::kMeter)
			numOutboundPluginParameters++;
	}

	// --- smoothable parameters; this is called during audio processing so we want this array to be as small as possible
	if (smoothablePluginParameters)
		delete[] smoothablePluginParameters;

	int m = 0;
	if (numSmoothablePluginParameters > 0)
	{
		smoothablePluginParameters = new PluginParameter*[numSmoothablePluginParameters];
		for (unsigned int i = 0; i < numPluginParameters; i++)
		{
			if (pluginParameters[i]->getControlVariableType() == controlVariableType::kDouble ||
				pluginParameters[i]->getControlVariableType() == controlVariableType::kFloat)
				smoothablePluginParameters[m++] = pluginParameters[i];
		}
	}

	if (outboundPluginParameters)
		delete[] outboundPluginParameters;

	m = 0;
	if (numOutboundPluginParameters > 0)
	{
		outboundPluginParameters = new PluginParameter*[numOutboundPluginParameters];
		for (unsigned int i = 0; i < numPluginParameters; i++)
		{
			if (pluginParameters[i]->getControlVariableType() == controlVariableType::kMeter)
				outboundPluginParameters[m++] = pluginParameters[i];
		}
	}

}

/**
\brief helper function to compare a PluginParameter's value with a string version of it

\param controlID the control ID of the parameter to test
\param compareString the string to test against the parameter's value (as string)

\return true if strings compare, false otherwise
*/
bool PluginBase::compareSelectedString(int32_t controlID, const char* compareString)
{
    PluginParameter* piParam = getPluginParameterByControlID(controlID);
    if(!piParam) return false;

    std::string str = piParam->getControlValueAsString();
    if(str.compare(compareString) == 0)
        return true;

    return false;
}


