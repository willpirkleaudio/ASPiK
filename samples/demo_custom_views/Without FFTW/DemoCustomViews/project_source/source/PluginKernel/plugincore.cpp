// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugincore.cpp
//
/**
    \file   plugincore.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  Implementation file for PluginCore object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "plugincore.h"
#include "plugindescription.h"
#include "customviews.h" // for custom knob message only!!

/**
\brief PluginCore constructor is launching pad for object initialization

Operations:
- initialize the plugin description (strings, codes, numbers, see initPluginDescriptors())
- setup the plugin's audio I/O channel support
- create the PluginParameter objects that represent the plugin parameters (see FX book if needed)
- create the presets
*/
PluginCore::PluginCore()
{
    // --- describe the plugin; call the helper to init the static parts you setup in plugindescription.h
    initPluginDescriptors();

    // --- default I/O combinations
	// --- for FX plugins
	if (getPluginType() == kFXPlugin)
	{
		addSupportedIOCombination({ kCFMono, kCFMono });
		addSupportedIOCombination({ kCFMono, kCFStereo });
		addSupportedIOCombination({ kCFStereo, kCFStereo });
	}
	else // --- synth plugins have no input, only output
	{
		addSupportedIOCombination({ kCFNone, kCFMono });
		addSupportedIOCombination({ kCFNone, kCFStereo });
	}

	// --- for sidechaining, we support mono and stereo inputs; auxOutputs reserved for future use
	addSupportedAuxIOCombination({ kCFMono, kCFNone });
	addSupportedAuxIOCombination({ kCFStereo, kCFNone });

	// --- create the parameters
    initPluginParameters();

    // --- create the presets
    initPluginPresets();

	// --- disable queue (NO GUI YET!)
	enableCustomViewDataQueue(false);
}

/**
\brief create all of your plugin parameters here

\return true if parameters were created, false if they already existed
*/
bool PluginCore::initPluginParameters()
{
	if (pluginParameterMap.size() > 0)
		return false;

    // --- Add your plugin parameter instantiation code bewtween these hex codes
	// **--0xDEA7--**
    // --- Declaration of Plugin Parameter Objects
    PluginParameter* piParam = nullptr;
    
    // --- continuous control: Cust Knob
    piParam = new PluginParameter(controlID::customKnob, "Cust Knob", "", controlVariableType::kDouble, 0.000000, 100.000000, 50.000000, taper::kLinearTaper);
    piParam->setParameterSmoothing(true);
    piParam->setSmoothingTimeMsec(20.00);
    piParam->setBoundVariable(&customKnob, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // --- non-GUI-bound variable: Switcher
    piParam = new PluginParameter(controlID::Switcher, "Switcher");
    addPluginParameter(piParam);
    
    // --- continuous control: Left Vol
    piParam = new PluginParameter(controlID::leftVolume_dB, "Left Vol", "dB", controlVariableType::kDouble, -60.000000, 12.000000, 0.000000, taper::kLinearTaper);
    piParam->setParameterSmoothing(true);
    piParam->setSmoothingTimeMsec(20.00);
    piParam->setBoundVariable(&leftVolume_dB, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // --- continuous control: Right Vol
    piParam = new PluginParameter(controlID::rightVolume_dB, "Right Vol", "dB", controlVariableType::kDouble, -60.000000, 12.000000, 0.000000, taper::kLinearTaper);
    piParam->setParameterSmoothing(true);
    piParam->setSmoothingTimeMsec(20.00);
    piParam->setBoundVariable(&rightVolume_dB, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // --- discrete control: LINK
    piParam = new PluginParameter(controlID::enableLink, "LINK", "SWITCH OFF,SWITCH ON", "SWITCH OFF");
    piParam->setBoundVariable(&enableLink, boundVariableType::kInt);
    piParam->setIsDiscreteSwitch(true);
    addPluginParameter(piParam);
    
    // --- non-GUI-bound variable: WaveView
    piParam = new PluginParameter(controlID::WaveView, "WaveView");
    addPluginParameter(piParam);
    
    
	// **--0xEDA5--**
   
    // --- BONUS Parameter
    // --- SCALE_GUI_SIZE
    PluginParameter* piParamBonus = new PluginParameter(SCALE_GUI_SIZE, "Scale GUI", "tiny,small,medium,normal,large,giant", "normal");
    addPluginParameter(piParamBonus);

	// --- create the super fast access array
	initPluginParameterArray();

    return true;
}

/**
\brief initialize object for a new run of audio; called just before audio streams

Operation:
- store sample rate and bit depth on audioProcDescriptor - this information is globally available to all core functions
- reset your member objects here

\param resetInfo structure of information about current audio format

\return true if operation succeeds, false otherwise
*/
bool PluginCore::reset(ResetInfo& resetInfo)
{
    // --- save for audio processing
    audioProcDescriptor.sampleRate = resetInfo.sampleRate;
    audioProcDescriptor.bitDepth = resetInfo.bitDepth;

    // --- other reset inits
    return PluginBase::reset(resetInfo);
}

/**
\brief one-time initialize function called after object creation and before the first reset( ) call

Operation:
- saves structure for the plugin to use; you can also load WAV files or state information here
*/
bool PluginCore::initialize(PluginInfo& pluginInfo)
{
	// --- add one-time init stuff here

	return true;
}

/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- syncInBoundVariables when preProcessAudioBuffers is called, it is *guaranteed* that all GUI control change information
  has been applied to plugin parameters; this binds parameter changes to your underlying variables
- NOTE: postUpdatePluginParameter( ) will be called for all bound variables that are acutally updated; if you need to process
  them individually, do so in that function
- use this function to bulk-transfer the bound variable data into your plugin's member object variables

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::preProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
    // --- sync internal variables to GUI parameters; you can also do this manually if you don't
    //     want to use the auto-variable-binding
    syncInBoundVariables();

    return true;
}

/**
\brief frame-processing method

Operation:
- decode the plugin type - for synth plugins, fill in the rendering code; for FX plugins, delete the if(synth) portion and add your processing code
- note that MIDI events are fired for each sample interval so that MIDI is tightly sunk with audio
- doSampleAccurateParameterUpdates will perform per-sample interval smoothing

\param processFrameInfo structure of information about *frame* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processAudioFrame(ProcessFrameInfo& processFrameInfo)
{
    // --- fire any MIDI events for this sample interval
    processFrameInfo.midiEventQueue->fireMidiEvents(processFrameInfo.currentFrame);

	// --- do per-frame updates; VST automation and parameter smoothing
	doSampleAccurateParameterUpdates();

    // --- decode the channelIOConfiguration and process accordingly
    //
	// --- Synth Plugin:
	// --- Synth Plugin --- remove for FX plugins
	if (getPluginType() == kSynthPlugin)
	{
		// --- output silence: change this with your signal render code
		processFrameInfo.audioOutputFrame[0] = 0.0;
		if (processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
			processFrameInfo.audioOutputFrame[1] = 0.0;

		return true;	/// processed
	}

    // --- decode the channelIOConfiguration and process accordingly
    //
    double leftGain = pow(10.0, leftVolume_dB / 20.0);
    double rightGain = pow(10.0, rightVolume_dB / 20.0);
    
    // --- FX Plugin:
    if (processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
        processFrameInfo.channelIOConfig.outputChannelFormat == kCFMono)
    {
        // --- pass through code: change this with your signal processing
        processFrameInfo.audioOutputFrame[0] = leftGain*processFrameInfo.audioInputFrame[0];
        
        // --- set waveform samples
		if (isCustomViewDataQueueEnabled())
			customViewDataQueue.enqueue(processFrameInfo.audioOutputFrame[0]);

        return true; /// processed
    }
    
    // --- Mono-In/Stereo-Out
    else if (processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
             processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
        // --- pass through code: change this with your signal processing
        processFrameInfo.audioOutputFrame[0] = leftGain*processFrameInfo.audioInputFrame[0];
        processFrameInfo.audioOutputFrame[1] = rightGain*processFrameInfo.audioInputFrame[0];
         
        // --- set waveform samples
		if (isCustomViewDataQueueEnabled())
			customViewDataQueue.enqueue(processFrameInfo.audioOutputFrame[0]);

        return true; /// processed
    }
    
    // --- Stereo-In/Stereo-Out
    else if (processFrameInfo.channelIOConfig.inputChannelFormat == kCFStereo &&
             processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
        // --- pass through code: change this with your signal processing
        processFrameInfo.audioOutputFrame[0] = leftGain*processFrameInfo.audioInputFrame[0];
        processFrameInfo.audioOutputFrame[1] = rightGain*processFrameInfo.audioInputFrame[0];
        
        // --- set waveform samples
		if (isCustomViewDataQueueEnabled())
			customViewDataQueue.enqueue(processFrameInfo.audioOutputFrame[0]);

        return true; /// processed
    }
    
    return false; /// NOT processed
}


/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- updateOutBoundVariables sends metering data to the GUI meters

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
	// --- update outbound variables; currently this is meter data only, but could be extended
	//     in the future
	updateOutBoundVariables();

    return true;
}

/**
\brief update the PluginParameter's value based on GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- use base class helper
    setPIParamValue(controlID, controlValue);

    // --- do any post-processing
    postUpdatePluginParameter(controlID, controlValue, paramInfo);

    return true; /// handled
}

/**
\brief update the PluginParameter's value based on *normlaized* GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param normalizedValue the new control value in normalized form
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo)
{
	// --- use base class helper, returns actual value
	double controlValue = setPIParamValueNormalized(controlID, normalizedValue, paramInfo.applyTaper);

	// --- do any post-processing
	postUpdatePluginParameter(controlID, controlValue, paramInfo);

	return true; /// handled
}

/**
\brief perform any operations after the plugin parameter has been updated; this is one paradigm for
	   transferring control information into vital plugin variables or member objects. If you use this
	   method you can decode the control ID and then do any cooking that is needed. NOTE: do not
	   overwrite bound variables here - this is ONLY for any extra cooking that is required to convert
	   the GUI data to meaninful coefficients or other specific modifiers.

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- now do any post update cooking; be careful with VST Sample Accurate automation
    //     If enabled, then make sure the cooking functions are short and efficient otherwise disable it
    //     for the Parameter involved
    /*switch(controlID)
    {
        case 0:
        {
            return true;    /// handled
        }

        default:
            return false;   /// not handled
    }*/

    return false;
}

/**
\brief has nothing to do with actual variable or updated variable (binding)

CAUTION:
- DO NOT update underlying variables here - this is only for sending GUI updates or letting you
  know that a parameter was changed; it should not change the state of your plugin.

WARNING:
- THIS IS NOT THE PREFERRED WAY TO LINK OR COMBINE CONTROLS TOGETHER. THE PROPER METHOD IS
  TO USE A CUSTOM SUB-CONTROLLER THAT IS PART OF THE GUI OBJECT AND CODE.
  SEE http://www.willpirkle.com for more information

\param controlID the control ID value of the parameter being updated
\param actualValue the new control value

\return true if operation succeeds, false otherwise
*/
bool PluginCore::guiParameterChanged(int32_t controlID, double actualValue)
{
	/*
	switch (controlID)
	{
		case controlID::<your control here>
		{

			return true; // handled
		}

		default:
			break;
	}*/

	return false; /// not handled
}

/**
\brief For Custom View and Custom Sub-Controller Operations

NOTES:
- this is for advanced users only to implement custom view and custom sub-controllers
- see the SDK for examples of use

\param messageInfo a structure containing information about the incoming message

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMessage(MessageInfo& messageInfo)
{
	// --- decode message
	switch (messageInfo.message)
	{
		// --- add customization appearance here
	case PLUGINGUI_DIDOPEN:
	{
		enableCustomViewDataQueue(true);
		return false;
	}

	// --- NULL pointers so that we don't accidentally use them
	case PLUGINGUI_WILLCLOSE:
	{
		enableCustomViewDataQueue(false);
		return false;
	}

	case PLUGINGUI_TIMERPING:
    {
        if (waveView || spectrumView)
        {
            float audioSample = 0.0;
            
            // --- try to get a value from queue
            bool success = customViewDataQueue.try_dequeue(audioSample);
            
            // --- if succeeds:
            if (success)
            {
                // --- empty queue into views; the each handle this differently
                while (success)
                {
                    if (waveView)
                        waveView->pushDataValue(audioSample);
                    
                    if (spectrumView)
                        spectrumView->pushDataValue(audioSample);
                    
                    // -- try to get next value until queue is empty
                    success = customViewDataQueue.try_dequeue(audioSample);
                }
            }
            
            // --- update and mark view as dirty
            if (waveView)
                waveView->updateView();
            
            if (spectrumView)
                spectrumView->updateView();
            
            return true;
        }
        
        return false;
    }
    // --- register the custom view, grab the ICustomView interface
    case PLUGINGUI_REGISTER_CUSTOMVIEW:
    {
        // --- decode name string
        if (messageInfo.inMessageString.compare("CustomWaveView") == 0)
        {
            // --- (1) get the custom view interface via incoming message data*
            if (waveView != static_cast<ICustomView*>(messageInfo.inMessageData))
                waveView = static_cast<ICustomView*>(messageInfo.inMessageData);
            
            if (!waveView) return false;
            
            // --- registered!
            return true;
        }
        
        if (messageInfo.inMessageString.compare("CustomSpectrumView") == 0)
        {
            // --- (1) get the custom view interface via incoming message data*
            if (spectrumView != static_cast<ICustomView*>(messageInfo.inMessageData))
                spectrumView = static_cast<ICustomView*>(messageInfo.inMessageData);
            
            if (!spectrumView) return false;
            
            // --- registered!
            return true;
        }
        
        // --- example of querying plugin for information and getting a pointer to the control
        //     which is VERY risky - you should use the custom view data structure and messaging
        //     to call functions on the control at the proper time
        if (messageInfo.inMessageString.compare("CustomKnobView") == 0)
        {
            // --- (1) get the custom view interface via incoming message data*
            if (knobView != static_cast<ICustomView*>(messageInfo.inMessageData))
                knobView = static_cast<ICustomView*>(messageInfo.inMessageData);
            
            if (!knobView) return false;
            
            // --- send the view a message
            VSTGUI::CustomViewMessage knobMessage;
            knobMessage.message = VSTGUI::MESSAGE_QUERY_CONTROL;
            knobMessage.queryString.assign("Hello There!");
            
            // --- send the message
            knobView->sendMessage(&knobMessage);
                        
            // --- check the reply string; the messgageData variable contains a pointer to the object (DANGEROUS)
            const char* reply = knobMessage.replyString.c_str();
            printf("%s", reply);
            
            // --- DO NOT DO THIS!!! (but it is possible)
            //CAnimKnob* customKnob = static_cast<CAnimKnob*>(knobMessage.messageData);
            
            // --- registered!
            return true;
        }
    }
	case PLUGINGUI_REGISTER_SUBCONTROLLER:
	case PLUGINGUI_QUERY_HASUSERCUSTOM:
	case PLUGINGUI_USER_CUSTOMOPEN:
	case PLUGINGUI_USER_CUSTOMCLOSE:
	case PLUGINGUI_EXTERNAL_SET_NORMVALUE:
	case PLUGINGUI_EXTERNAL_SET_ACTUALVALUE:
	{

		return false;
	}

	default:
		break;
	}

	return false; /// not handled
}


/**
\brief process a MIDI event

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param event a structure containing the MIDI event data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMIDIEvent(midiEvent& event)
{
	return true;
}

/**
\brief (for future use)

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param vectorJoysickData a structure containing joystick data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData)
{
	return true;
}

/**
\brief use this method to add new presets to the list

NOTES:
- see the SDK for examples of use
- for non RackAFX users that have large paramter counts, there is a secret GUI control you
  can enable to write C++ code into text files, one per preset. See the SDK or http://www.willpirkle.com for details

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginPresets()
{
	// **--0xFF7A--**

	// **--0xA7FF--**

    return true;
}

/**
\brief setup the plugin description strings, flags and codes; this is ordinarily done through the ASPiKreator or CMake

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginDescriptors()
{
    pluginDescriptor.pluginName = PluginCore::getPluginName();
    pluginDescriptor.shortPluginName = PluginCore::getShortPluginName();
    pluginDescriptor.vendorName = PluginCore::getVendorName();
    pluginDescriptor.pluginTypeCode = PluginCore::getPluginType();

	// --- describe the plugin attributes; set according to your needs
	pluginDescriptor.hasSidechain = kWantSidechain;
	pluginDescriptor.latencyInSamples = kLatencyInSamples;
	pluginDescriptor.tailTimeInMSec = kTailTimeMsec;
	pluginDescriptor.infiniteTailVST3 = kVSTInfiniteTail;

    // --- AAX
    apiSpecificInfo.aaxManufacturerID = kManufacturerID;
    apiSpecificInfo.aaxProductID = kAAXProductID;
    apiSpecificInfo.aaxBundleID = kAAXBundleID;  /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.aaxEffectID = "aaxDeveloper.";
    apiSpecificInfo.aaxEffectID.append(PluginCore::getPluginName());
    apiSpecificInfo.aaxPluginCategoryCode = kAAXCategory;

    // --- AU
    apiSpecificInfo.auBundleID = kAUBundleID;   /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.auBundleName = kAUBundleName;

    // --- VST3
    apiSpecificInfo.vst3FUID = PluginCore::getVSTFUID(); // OLE string format
    apiSpecificInfo.vst3BundleID = kVST3BundleID;/* MacOS only: this MUST match the bundle identifier in your info.plist file */
	apiSpecificInfo.enableVST3SampleAccurateAutomation = kVSTSAA;
	apiSpecificInfo.vst3SampleAccurateGranularity = kVST3SAAGranularity;

    // --- AU and AAX
    apiSpecificInfo.fourCharCode = PluginCore::getFourCharCode();

    return true;
}

// --- static functions required for VST3/AU only --------------------------------------------- //
const char* PluginCore::getPluginBundleName() { return kAUBundleName; }
const char* PluginCore::getPluginName(){ return kPluginName; }
const char* PluginCore::getShortPluginName(){ return kShortPluginName; }
const char* PluginCore::getVendorName(){ return kVendorName; }
const char* PluginCore::getVendorURL(){ return kVendorURL; }
const char* PluginCore::getVendorEmail(){ return kVendorEmail; }
const char* PluginCore::getAUCocoaViewFactoryName(){ return AU_COCOA_VIEWFACTORY_STRING; }
pluginType PluginCore::getPluginType(){ return kPluginType; }
const char* PluginCore::getVSTFUID(){ return kVSTFUID; }
int32_t PluginCore::getFourCharCode(){ return kFourCharCode; }
