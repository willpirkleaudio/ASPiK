// -----------------------------------------------------------------------------
//    ASPiK-Core File:  aaxpluginparameters.cpp
//
//    http://www.aspikplugins.com
//    http://www.willpirkle.com
//
/**
    \file   aaxpluginparameters.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implementation file for the AAXPluginParameters object; this code is
    		based heavily off of the monolithic plugin example in the AAX SDK
    		but custom fit to include the ASPiK kernel objects in a tight
    		synchronization.

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "AAXPluginParameters.h"

// --- built-in linear taper
#include "AAX_CLinearTaperDelegate.h"

// --- custom tapers
#include "LogTaperDelegate.h"
#include "AntiLogTaperDelegate.h"
#include "VoltOctaveTaperDelegate.h"
#include "channelformats.h"

#include "AAX_CNumberDisplayDelegate.h"
#include "AAX_CDecibelDisplayDelegateDecorator.h"
#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CBinaryDisplayDelegate.h"
#include "AAX_CUnitDisplayDelegateDecorator.h"
#include "AAX_CStringDisplayDelegate.h"
#include "AAX_MIDIUtilities.h"
#include "AAX_Assert.h"
#include "AAX_CMutex.h" // --- not currently used

// --- custom VSTGUI4 derived classes
#include "customcontrols.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::Create()
//
/**
 \brief creation mechanism for this object

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_CEffectParameters * AAX_CALLBACK AAXPluginParameters::Create()
{
	return new AAXPluginParameters();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::AAXPluginParameters()
//
/**
 \brief object constructor; this creates the ASPiK core and all of its required interfaces.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAXPluginParameters::AAXPluginParameters() : AAX_CEffectParameters()
{
    // --- the one and only core object
    pluginCore = new PluginCore;

    // --- initialize the plugin core
    //     Currently, this only passes the folder location of the DLL to the core for storage and use
    //     for example to load samples or other non-internal plugin stuff
    PluginInfo initInfo;
#ifdef MAC
    const char* bundleIDStr = pluginCore->getAAXBundleID();
    if (bundleIDStr)
    {
        CFStringRef bundleID = CFStringCreateWithCString(NULL, bundleIDStr, kCFStringEncodingASCII);
        initInfo.pathToDLL = getMyComponentDirectory(bundleID);
        if (initInfo.pathToDLL)
        {
            pluginCore->initialize(initInfo);
        }
    }
#else // Windows
    std::string strBundleName(pluginCore->getPluginBundleName());
    strBundleName.append(".aaxplugin");
	char* path = getMyDLLDirectory(strBundleName.c_str());
	initInfo.pathToDLL = path;
    // --- send to core
    if (initInfo.pathToDLL)
    {
        pluginCore->initialize(initInfo);
		delete[] path;
    }
#endif

    // --- check parameter count vs. our static declaration
    if(pluginCore->getPluginParameterCount()> kSynchronizedParameterQueueSize)
    {
        fprintf(stderr, "Parameter count exceeds kSynchronizedParameterQueueSize; please adjust queus size in AAXPluginParameters.h");
    }

    // --- setup IGUIPluginConnector - must be done early as possible
    guiPluginConnector = new GUIPluginConnector(this, pluginCore);
    pluginHostConnector = new PluginHostConnector(this);
    pluginCore->setPluginHostConnector(pluginHostConnector);

    // --- MIDI queue setup
    midiEventQueue = new AAXMIDIEventQueue(pluginCore);

    // --- other inits
    mCurrentStateNum = -1;
    softBypass = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::AAXPluginParameters()
//
/**
 \brief object destructor; this destroys the ASPiK core and all of its required interfaces.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAXPluginParameters::~AAXPluginParameters()
{
    if(pluginCore) delete pluginCore;
    if(guiPluginConnector) delete guiPluginConnector;
    if(midiEventQueue) delete midiEventQueue;
    if(pluginHostConnector) delete pluginHostConnector;

    DeleteUsedParameterChanges();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::GetCustomData()
//
/**
 \brief note the data that is transferred to GUI; the core is ONLY used for initialization and then it is unused.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::GetCustomData(AAX_CTypeID iDataBlockID, uint32_t iDataSize, void* oData, uint32_t* oDataWritten) const
{
    if(iDataBlockID == PLUGIN_CUSTOMDATA_ID)
    {
        pluginCustomData* pData = (pluginCustomData*)oData;
        pData->guiPlugin_Connector = guiPluginConnector;
        pData->plugin_Core = pluginCore;

        *oDataWritten = sizeof(pluginCustomData*);
        return AAX_SUCCESS;
    }
    return AAX_ERROR_UNIMPLEMENTED;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::EffectInit()
//
/**
 \brief note the data that is transferred to GUI; the core is ONLY used for initialization and then it is unused.

 NOTES:
 - create all AAX parameter objects and register; this also sets up slots in the monolithic parameters arrays
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::EffectInit()
{
    // --- bypass, must be first
    AAX_CString bypassID = cDefaultMasterBypassID;
    AAX_IParameter* masterBypass = new AAX_CParameter<bool>(bypassID.CString(), AAX_CString("Master Bypass"), false,
        AAX_CBinaryTaperDelegate<bool>(),
        AAX_CBinaryDisplayDelegate<bool>("bypass", "on"), true);
    masterBypass->SetNumberOfSteps( 2 );
    masterBypass->SetType( AAX_eParameterType_Discrete );
    mParameterManager.AddParameter(masterBypass);
    AddSynchronizedParameter(*masterBypass);

    if(!pluginCore)
         return AAX_SUCCESS;

    // --- init core
    AAX_CSampleRate sampleRate = 44100;

    // --- get the sample rate
    Controller()->GetSampleRate(&sampleRate);

    // --- report our latency
	Controller()->SetSignalLatency((int32_t)pluginCore->getLatencyInSamples());

    ResetInfo info;
    info.sampleRate = sampleRate;

    // --- reset with sample rate
    pluginCore->reset(info);

    for(uint32_t i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);

        if(piParam)
        {
            std::string sUnit = " ";
            sUnit.append(piParam->getControlUnits());

            std::stringstream strAAX_ID;                // use control index as "Unique ID"
            strAAX_ID << piParam->getControlID() + 1;   // offset by 1 b/c of Master Bypass

            // --- with custom GUI, the PluginGUI object will handle details
            if(pluginCore->hasCustomGUI())
            {
                // --- if meter, set non-automatable
                if(piParam->isMeterParam())
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                                   AAX_CString(piParam->getControlName()),
                                                                                   piParam->getDefaultValue(),
                                                                                   AAX_CLinearTaperDelegate<double>(0.0, 1.0),
                                                                                   AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(),
                                                                                   sUnit.c_str()), false); // false = no automation for meters!
                    // --- 128 is default, not used for meter
                    param->SetNumberOfSteps(128);
                    param->SetType(AAX_eParameterType_Continuous);//AAX_eParameterType_Continuous);
                   
                    // mParameterManager.AddParameter(param); REMOVED to prevent save "bug"
                    mMeterParameterManager.AddParameter(param);
					// --- NOTE: not adding to synchro list because not automatable
                }
                else // --- all other controls
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                               AAX_CString(piParam->getControlName()),
                                                                               piParam->getDefaultValue(),
                                                                               AAX_CLinearTaperDelegate<double>(piParam->getMinValue(), piParam->getMaxValue()),
                                                                               AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(), piParam->getControlUnits()), true);
                    // --- GUI handles everything
                    param->SetNumberOfSteps(128);
                    param->SetType(AAX_eParameterType_Continuous);
                    mParameterManager.AddParameter(param);

					// --- do not synchronize non-variable bound stuff
					if(!piParam->isNonVariableBoundParam())
						AddSynchronizedParameter(*param);
                }
            }
            else // setup for client GUI
            {
                // --- do not register non-bound controls as parameters
                if(piParam->isNonVariableBoundParam() || piParam->isMeterParam())
                {
                    continue;
                }
                else if(piParam->isStringListParam())
                {
                    int numStrings = (int)piParam->getStringCount();
                    std::map<int32_t, AAX_CString> enumStrings;

                    for (int j=0; j<numStrings; j++)
                    {
                        std::string stringParam = piParam->getStringByIndex(j);
                        const char* text = stringParam.c_str();
                        enumStrings.insert(std::pair<int32_t, AAX_CString>(j, AAX_CString(text)) );
                    }

                    // NOTE: this linear taper uses the hidden RealPrecision of 1 to force integer rounding: AAX_CLinearTaperDelegate<int32_t,1> the ",1" see the AAX_CLinearTaperDelegate declaration
                    AAX_CParameter<int32_t>* param = new AAX_CParameter<int32_t>(strAAX_ID.str().c_str(),
                                                                                 AAX_CString(piParam->getControlName()),
																				 (int32_t)piParam->getDefaultValue(),
																				 AAX_CLinearTaperDelegate<int32_t, 1>((int32_t)piParam->getMinValue(), (int32_t)piParam->getMaxValue()),
                                                                                 AAX_CStringDisplayDelegate<int32_t>(enumStrings),
                                                                                 true);
                    param->SetNumberOfSteps(128);//nEnumStrings);
                    param->SetType(AAX_eParameterType_Continuous);
                    mParameterManager.AddParameter(param);
                    AddSynchronizedParameter(*param);
                }
                else if(piParam->isLogTaper())
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                               AAX_CString(piParam->getControlName()),
                                                                               piParam->getDefaultValue(),
                                                                               LogTaperDelegate<double>(piParam->getMinValue(), piParam->getMaxValue()),
                                                                               AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(), piParam->getControlUnits()), true);
                    // --- for default GUI only
                    if(piParam->isIntParam())
                    {
                        param->SetNumberOfSteps((uint32_t)(piParam->getMaxValue() - piParam->getMaxValue() + 1));
                        param->SetType(AAX_eParameterType_Discrete);
                    }
                    else
                    {
                        param->SetNumberOfSteps(128);
                        param->SetType(AAX_eParameterType_Continuous);
                    }

                    mParameterManager.AddParameter(param);
                    AddSynchronizedParameter(*param);

                }
                else if(piParam->isAntiLogTaper())
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                               AAX_CString(piParam->getControlName()),
                                                                               piParam->getDefaultValue(),
                                                                               AntiLogTaperDelegate<double>(piParam->getMinValue(), piParam->getMaxValue()),
                                                                               AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(), piParam->getControlUnits()), true);
                    // --- for default GUI only
                    if(piParam->isIntParam())
                    {
						param->SetNumberOfSteps((uint32_t)(piParam->getMaxValue() - piParam->getMaxValue() + 1));
                        param->SetType(AAX_eParameterType_Discrete);
                    }
                    else
                    {
                        param->SetNumberOfSteps(128);
                        param->SetType(AAX_eParameterType_Continuous);
                    }
                    mParameterManager.AddParameter(param);
                    AddSynchronizedParameter(*param);

                }
                else if(piParam->isVoltOctaveTaper())
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                               AAX_CString(piParam->getControlName()),
                                                                               piParam->getDefaultValue(),
                                                                               VoltOctaveTaperDelegate<double>(piParam->getMinValue(), piParam->getMaxValue()),
                                                                               AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(), piParam->getControlUnits()), true);
                    // --- for default GUI only
                    if(piParam->isIntParam())
                    {
						param->SetNumberOfSteps((uint32_t)(piParam->getMaxValue() - piParam->getMaxValue() + 1));
                        param->SetType(AAX_eParameterType_Discrete);
                    }
                    else
                    {
                        param->SetNumberOfSteps(128);
                        param->SetType(AAX_eParameterType_Continuous);
                    }
                    mParameterManager.AddParameter(param);
                    AddSynchronizedParameter(*param);

                }
                else if(!piParam->isMeterParam())
                {
                    AAX_CParameter<double>* param = new AAX_CParameter<double>(strAAX_ID.str().c_str(),
                                                                             AAX_CString(piParam->getControlName()),
                                                                             piParam->getDefaultValue(),
                                                                             AAX_CLinearTaperDelegate<double>(piParam->getMinValue(), piParam->getMaxValue()),
                                                                             AAX_CUnitDisplayDelegateDecorator<double>(AAX_CNumberDisplayDelegate<double>(), piParam->getControlUnits()), true);
                    // --- for default GUI only
                    if(piParam->isIntParam())
                    {
						param->SetNumberOfSteps((uint32_t)(piParam->getMaxValue() - piParam->getMaxValue() + 1));
                        param->SetType(AAX_eParameterType_Discrete);
                    }
                    else
                    {
                        param->SetNumberOfSteps(128);
                        param->SetType(AAX_eParameterType_Continuous);
                    }
                    mParameterManager.AddParameter(param);
                    AddSynchronizedParameter(*param);
                }
            }
        }
    }

    return AAX_SUCCESS;
 }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::UpdatePluginParameters()
//
/**
 \brief called once per buffer process operation to update any parameters that changed during the buffer fill

 NOTES:
  - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::UpdatePluginParameters(const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues)
{
    if(!pluginCore) return;

    for(int32_t i = 0; i < inNumSynchronizedParamValues; ++i)
    {
        AAX_CParamID const iParameterID = inSynchronizedParamValues[i]->first;
        const AAX_IParameter* const param = mParameterManager.GetParameterByID(iParameterID);
        unsigned controlID = atoi(iParameterID) - 1; // --- first param is ALWAYS bypass

        double controlValue = 0.0;
        bool bsuccess = param->GetValueAsDouble(&controlValue);

        ParameterUpdateInfo paramInfo;
        
        if(bsuccess)
            pluginCore->updatePluginParameter(controlID, controlValue, paramInfo);
    }

    return;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::updateHostInfo()
//
/**
 \brief called once per buffer process operation to set the host information structure for the core

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::updateHostInfo(AAXAlgorithm* ioRenderInfo, HostInfo* hostInfo)
{
    if(!hostInfo) return;
    memset(hostInfo, 0, sizeof(HostInfo));

    AAX_IMIDINode* const transportNode = ioRenderInfo->midiTransportNode;
    AAX_ITransport* const midiTransport = transportNode->GetTransport();

    // --- get info
    int32_t nTSNumerator = 0;
    int32_t nTSDenominator = 0;

    midiTransport->IsTransportPlaying(&hostInfo->bIsPlayingAAX);
    midiTransport->GetCurrentTempo(&hostInfo->dBPM);
    midiTransport->GetCurrentNativeSampleLocation((int64_t*)&hostInfo->uAbsoluteFrameBufferIndex); // notoriously incorrect
    midiTransport->GetCurrentMeter(&nTSNumerator, &nTSDenominator);
    midiTransport->GetCurrentTickPosition(&hostInfo->nTickPosition);
    midiTransport->GetCurrentLoopPosition (&hostInfo->bLooping, &hostInfo->nLoopStartTick, &hostInfo->nLoopEndTick);

    hostInfo->fTimeSigNumerator = (float)nTSNumerator;
    hostInfo->uTimeSigDenomintor = (unsigned int)nTSDenominator;

    // --- see the documentation in AAX_ITransport.h about these - if you decide to use them, they need to be relocated to a non-realtime thread!
#ifdef ENABLE_EXTRA_HOST_INFO // by default this is NOT defined because of the performance hit you take with these extra functions; use at your own risk and/or relocate to a non-realtime thread

    // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
    midiTransport->GetBarBeatPosition(&hostInfo->nBars, &hostInfo->nBeats, &hostInfo->nDisplayTicks, hostInfo->uAbsoluteFrameBufferIndex);

    // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
    midiTransport->GetCustomTickPosition(&hostInfo->nCustomTickPosition, hostInfo->uAbsoluteFrameBufferIndex);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::ProcessAudio()
//
/**
 \brief audio processing function; note that ALL algorithms point to this function so we have to decode channel information.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::ProcessAudio(AAXAlgorithm* ioRenderInfo, const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues)
{
    // --- global param update for dirty params
    UpdatePluginParameters(inSynchronizedParamValues, inNumSynchronizedParamValues);

    // --- MIDI input (all plugins can have MIDI in)
    AAX_IMIDINode* const midiInNode = ioRenderInfo->midiInputNode;
    AAX_CMidiStream* const midiBuffer = midiInNode->GetNodeBuffer();
    const AAX_CMidiPacket* currentMidiPacket = midiBuffer->mBuffer;
    uint32_t midiBufferSize = midiBuffer->mBufferSize;

    // --- audio input
    const int32_t buffersize = *(ioRenderInfo->bufferLength);
    AAX_EStemFormat inputStemFormat = AAX_eStemFormat_None;
    AAX_EStemFormat outputStemFormat = AAX_eStemFormat_None;
    Controller()->GetInputStemFormat(&inputStemFormat);
    Controller()->GetOutputStemFormat(&outputStemFormat);
    int32_t nLatencySamples = 0;

    Controller()->GetHybridSignalLatency(&nLatencySamples);

    const int32_t numChannelsIn = AAX_STEM_FORMAT_CHANNEL_COUNT(inputStemFormat);
    const int32_t numChannelsOut = AAX_STEM_FORMAT_CHANNEL_COUNT(outputStemFormat);

#ifdef WANT_SIDECHAIN
    const int32_t sidechainChannelIndex = *ioRenderInfo->sidechainChannel;
#else
    const int32_t sidechainChannelIndex = 0;
#endif

#ifdef PT_GR_METER
    // -- grmeter
    float* const AAX_RESTRICT grMeters = *ioRenderInfo->grMeterPtrs;
#endif
    
    // --- implement soft bypass
    if(softBypass)
    {
        for (int ch = 0; ch < numChannelsIn; ch++)
        {
            const float* const AAX_RESTRICT pInputBuffer = ioRenderInfo->inputBufferPtrs[ch];
            float* const AAX_RESTRICT pOutputBuffer = ioRenderInfo->outputBufferPtrs[ch];

            for (int t = 0; t < buffersize; t ++)
            {
                pOutputBuffer[t] = pInputBuffer[t];
            }
        }
        return;
    }

    // --- audio processing
    ProcessBufferInfo info;
    info.inputs = &ioRenderInfo->inputBufferPtrs[0];
    info.outputs = &ioRenderInfo->outputBufferPtrs[0];
    if(sidechainChannelIndex)
    {
        info.auxInputs = &ioRenderInfo->inputBufferPtrs[sidechainChannelIndex];
    }
    info.auxOutputs = nullptr; // --- for future use

    // --- set channel counts
    info.numAudioInChannels = numChannelsIn;
    info.numAudioOutChannels = numChannelsOut;
    info.numAuxAudioInChannels = sidechainChannelIndex ? 1 : 0; // AAX is mono SC only
    info.numAuxAudioOutChannels= 0; // --- for future use

    info.numFramesToProcess = buffersize;

    // --- setup the channel configs
    info.channelIOConfig.inputChannelFormat = getChannelFormatForAAXStemFormat(inputStemFormat);
    info.channelIOConfig.outputChannelFormat = getChannelFormatForAAXStemFormat(outputStemFormat);

    // --- sidechain channel config
    info.auxChannelIOConfig.inputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(info.numAuxAudioInChannels);
    info.auxChannelIOConfig.outputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(0);

    // --- MIDI
    midiEventQueue->setMIDIpackets(currentMidiPacket, midiBufferSize);
    info.midiEventQueue = midiEventQueue;

    // --- get host info
    HostInfo hostInfo;
    updateHostInfo(ioRenderInfo, &hostInfo);
    info.hostInfo = &hostInfo;

    // --- process the buffers
    pluginCore->processAudioBuffers(info);

#ifdef PT_GR_METER
    // --- optional ProTools GR Meters
    if(grMeters && pluginCore->hasProToolsGRMeters())
    {
        // --- get the meter values for those defined with the PT Meter flag = true
        //     since there is only one GR meter (at least for now) we will merge the meters into mono
        //     See the getProToolsGRValue() function; you can copy it here and modify as you like
        //     to get each GR meter values independently when the time comes that we see multiple GR meters
        //
        //     getProToolsGRValue() returns the average value of all meters defined as PT GR meters
        *grMeters = (float)pluginCore->getProToolsGRValue();
    }
#endif

    // --- update outbound parameters
    updateOutboundAAXParameters();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::StaticRenderAudio()
//
/**
 \brief static callback function that exists as an object member and is part of the
        monolithic programming paradigm described in great detail in the source below.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAX_CALLBACK AAXPluginParameters::StaticRenderAudio(AAXAlgorithm* const	inInstancesBegin [], const void* inInstancesEnd)
{
	for (AAXAlgorithm * const * instanceRenderInfoPtr = inInstancesBegin; instanceRenderInfoPtr != inInstancesEnd; ++instanceRenderInfoPtr)
	{
		pluginPrivateData* privateData = (*instanceRenderInfoPtr)->privateData;
		if(privateData != 0)
		{
			// --- Grab the object pointer from the Context
			AAXPluginParameters*	parameters = privateData->aaxPluginParameters;
			if (parameters != 0)
			{
				// --- Update synchronized parameters to the target state num and get a queue of parameter value changes
				SParamValList paramValList = parameters->GetUpdatesForState(*(*instanceRenderInfoPtr)->currentStateNum);
				parameters->ProcessAudio(*instanceRenderInfoPtr, (const TParamValPair**)paramValList.mElem, paramValList.mSize);

				// --- Queue the parameter value pairs for later deletion
				for (int32_t i = 0; i < paramValList.mSize; ++i)
				{
					const AAX_IContainer::EStatus pushResult = parameters->mFinishedParameterValues.Push(paramValList.mElem[i]);
					AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
				}
			}
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::StaticDescribe()
//
/**
 \brief static describe function that exists as an object member and is part of the
        monolithic programming paradigm described in great detail in the source below.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::StaticDescribe(AAX_IComponentDescriptor& outDesc)
{
	AAX_Result err = AAX_SUCCESS;

    // --- plugin core; only for info, will be destroyed
    PluginCore* plugin = new PluginCore;
    std::string midiInStr = plugin->getPluginName();
    midiInStr.append(" MIDI In"); // identify midi in

	// --- AUDIO
	err |= outDesc.AddAudioIn(AAX_FIELD_INDEX (AAXAlgorithm, inputBufferPtrs));
	err |= outDesc.AddAudioOut(AAX_FIELD_INDEX (AAXAlgorithm, outputBufferPtrs));
	err |= outDesc.AddAudioBufferLength( AAX_FIELD_INDEX (AAXAlgorithm, bufferLength) );

	// --- MIDI
	err |= outDesc.AddMIDINode(AAX_FIELD_INDEX(AAXAlgorithm, midiInputNode), AAX_eMIDINodeType_LocalInput, midiInStr.c_str(), 0xffff); // 0xffff = mask
    err |= outDesc.AddMIDINode(AAX_FIELD_INDEX(AAXAlgorithm, midiTransportNode), AAX_eMIDINodeType_Transport, "Transport", 0xffff); // 0xffff = mask

	// --- private data
	err |= outDesc.AddPrivateData(AAX_FIELD_INDEX(AAXAlgorithm, privateData), sizeof(pluginPrivateData), AAX_ePrivateDataOptions_DefaultOptions);
	err |= outDesc.AddDataInPort(AAX_FIELD_INDEX(AAXAlgorithm, currentStateNum), sizeof(uint64_t));

	// --- setup properties
	AAX_IPropertyMap* properties = outDesc.NewPropertyMap();
	AAX_ASSERT(properties);
	if(!properties) return err;

    if(!plugin->hasCustomGUI())
        properties->AddProperty(AAX_eProperty_UsesClientGUI, true); 	// Register for auto-GUI

    delete plugin;

	// --- if sidechaining...
	#ifdef WANT_SIDECHAIN
		err |= outDesc.AddSideChainIn(AAX_FIELD_INDEX(AAXAlgorithm, sidechainChannel));
		err |= properties->AddProperty(AAX_eProperty_SupportsSideChainInput, true );
	#endif

#ifdef PT_GR_METER
    // --- define one GR meter (have never seen more than this, even on AVID S6) but you can add more here
    err |= outDesc.AddMeters (AAX_FIELD_INDEX(AAXAlgorithm, grMeterPtrs), &GR_MeterID, meterTapCount);
 #endif

    // --- transport node
    err |= properties->AddProperty(AAX_eProperty_UsesTransport, true);

	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::GenerateCoefficients()
//
/**
 \brief called at the top of the buffer processing cycle to queue up dirty parameters; part
        of the AAX monolithic parameters paradigm

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::GenerateCoefficients()
{
	AAX_Result result = AAX_CEffectParameters::GenerateCoefficients();
	if (AAX_SUCCESS != result) { return result; }

	const int64_t stateNum = mStateCounter++;

	// --- Check for dirty parameters
	TNumberedParamStateList::second_type paramStateList;
	while (false == mDirtyParameters.empty())
	{
		TParamSet::iterator paramIter = mDirtyParameters.begin();
		const AAX_IParameter* const param = *paramIter;
		mDirtyParameters.erase(param);
		if (NULL != param)
		{
			paramStateList.push_back(new TParamValPair(param->Identifier(), param->CloneValue()));
		}
	}

	if (false == paramStateList.empty())
	{
		TNumberedParamStateList* const numberedParamState = new TNumberedParamStateList(std::make_pair(stateNum, paramStateList));
		{
			const AAX_IContainer::EStatus pushResult = mQueuedParameterChanges.Push(numberedParamState);
			AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
		}
	}

	result = Controller()->PostPacket(AAX_FIELD_INDEX(AAXAlgorithm, currentStateNum), &stateNum, sizeof(int64_t));

	return result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::AddSynchronizedParameter()
//
/**
 \brief called to add parameters to the synchronized list; this is only called once during EffectInit( )
        as part of the AAX monolithic parameters paradigm

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::AddSynchronizedParameter(const AAX_IParameter& inParameter)
{
	mSynchronizedParameters.insert(inParameter.Identifier());
	AAX_ASSERT(inParameter.Automatable());

    // check for "Parameter count exceeds kSynchronizedParameterQueueSize; please adjust queus size in AAXPluginParameters.h"
	AAX_ASSERT(kSynchronizedParameterQueueSize >= mSynchronizedParameters.size());
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::UpdateParameterNormalizedValue()
//
/**
 \brief called when a parameter needs to be updated (aka it is "dirty")
        as part of the AAX monolithic parameters paradigm

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource )
{
	AAX_Result result = AAX_CEffectParameters::UpdateParameterNormalizedValue(iParameterID, iValue, iSource);
	if (AAX_SUCCESS != result) { return result; }

	if(AAX::IsParameterIDEqual("MasterBypassID", iParameterID))
	{
		softBypass = iValue == 0 ? false : true;
		return AAX_SUCCESS;
	}

	const AAX_IParameter* const param = mParameterManager.GetParameterByID(iParameterID);

	// --- add to dirty param list
	if ((param) && (0 < mSynchronizedParameters.count(iParameterID)))
	{
		mDirtyParameters.insert(param);
	}

	return result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::ResetFieldData()
//
/**
 \brief this is where we set the pointer back to this objecgt for our private date; part of
 monolithic paradigm and described in detail in book source below.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t iDataSize) const
{
	if (inFieldIndex == AAX_FIELD_INDEX(AAXAlgorithm, privateData) )
	{
		AAX_ASSERT( iDataSize == sizeof(pluginPrivateData) );
		memset(oData, 0, iDataSize);

		pluginPrivateData* privateData = static_cast <pluginPrivateData*> (oData);
		privateData->aaxPluginParameters = (AAXPluginParameters*) this;
		return AAX_SUCCESS;
	}

	return AAX_CEffectParameters::ResetFieldData(inFieldIndex, oData, iDataSize);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::GetUpdatesForState()
//
/**
 \brief part of monolithic paradigm and described in detail in book source below.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAXPluginParameters::SParamValList AAXPluginParameters::GetUpdatesForState(int64_t inTargetStateNum)
{
    if(mCurrentStateNum != inTargetStateNum)
    {
        mCurrentStateNum = inTargetStateNum;
    }
	SParamValList paramValList;
	TNumberedStateListQueue stateLists;

	for(TNumberedParamStateList* numberedStateList = mQueuedParameterChanges.Peek();
		 // Condition
		 (NULL != numberedStateList) && // there is an element in the queue
		 (	(numberedStateList->first <= inTargetStateNum) || // next queued state is before or equal to target state
		  ((-0xFFFF > inTargetStateNum) && (0xFFFF < numberedStateList->first)) // target state counter has wrapped around
		  );

		 // Increment
		 numberedStateList = mQueuedParameterChanges.Peek()
		 )
	{
		// We'll use this state, so pop it from the queue
		const TNumberedParamStateList* const poppedPair = mQueuedParameterChanges.Pop();
		AAX_ASSERT(poppedPair == numberedStateList); // We can trust that this will match because there is only one thread calling Pop() on mQueuedParameterChanges
		const AAX_IContainer::EStatus pushResult = stateLists.Push(numberedStateList);
		AAX_ASSERT(AAX_IContainer::eStatus_Success == pushResult);
		numberedStateList = mQueuedParameterChanges.Peek();
	}

	// Transfer all parameter states into the single SParamValList
	for (TNumberedParamStateList* numberedStateList = stateLists.Pop(); NULL != numberedStateList; numberedStateList = stateLists.Pop())
	{
		paramValList.Append(numberedStateList->second);
		numberedStateList->second.clear(); // Ownership of all elements has been transferred to paramValList

		// Queue the now-empty container for later deletion
		mFinishedParameterChanges.Push(numberedStateList);
	}

	return paramValList;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::DeleteUsedParameterChanges()
//
/**
 \brief part of monolithic paradigm and described in detail in book source below, this is the clean-up function.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::DeleteUsedParameterChanges()
{
	// Deep-delete all elements from the used parameter change queue
	for (TNumberedParamStateList* numberedStateList = mFinishedParameterChanges.Pop(); NULL != numberedStateList; numberedStateList = mFinishedParameterChanges.Pop())
	{
		TNumberedParamStateList::second_type& curStateList = numberedStateList->second;
		for (std::list<TParamValPair*>::const_iterator iter = curStateList.begin(); iter != curStateList.end(); ++iter)
		{
			if (*iter) { delete *iter; }
		}
		delete numberedStateList;
	}

	// Delete all used parameter values
	for (const TParamValPair* paramVal = mFinishedParameterValues.Pop(); NULL != paramVal; paramVal = mFinishedParameterValues.Pop())
	{
		if (paramVal) { delete paramVal; }
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::TimerWakeup()
//
/**
 \brief cleans out unneeded parameters

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::TimerWakeup()
{
	DeleteUsedParameterChanges();
	return AAX_CEffectParameters::TimerWakeup();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    AAXPluginGUAAXPluginParametersI::GetParameterNormalizedValue()
//
/**
 \brief allows threadsafe getting of parameters for GUI; modified to differentiate beteen meters to prevent
        a "bug" where the non-saved state marker pops up when saving a meter-based GUI

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginParameters::GetParameterNormalizedValue (AAX_CParamID iParameterID, double * oValuePtr ) const
{
    if(!pluginCore) return AAX_ERROR_NULL_OBJECT;
     
    int nTag = atoi(iParameterID) - 1;
    PluginParameter* piParam = pluginCore->getPluginParameterByControlID(nTag);
    if(piParam)
    {
        if(piParam->getControlVariableType() == controlVariableType::kMeter)
        {
            const AAX_IParameter* parameter = mMeterParameterManager.GetParameterByID( iParameterID );
            if (parameter == 0)
                return AAX_ERROR_INVALID_PARAMETER_ID;

            *oValuePtr = parameter->GetNormalizedValue();
            return AAX_SUCCESS;
        }
        else
            return AAX_CEffectParameters::GetParameterNormalizedValue(iParameterID, oValuePtr);
    }
    
    return AAX_ERROR_NULL_OBJECT;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUAAXPluginParametersI::updateOutboundAAXParameters()
//
/**
 \brief threadsafe update of outbound parameters (meter variables) for GUI.

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginParameters::updateOutboundAAXParameters()
{
    if(!pluginCore) return;

    int32_t startIndex = 0;
    while(startIndex >= 0)
    {
        PluginParameter* piParam = pluginCore->getNextParameterOfType(startIndex, controlVariableType::kMeter);
        if(piParam)
        {
            std::stringstream str;
            str << piParam->getControlID() + 1;
            SetMeterParameterNormalizedValue(str.str().c_str(), piParam->getControlValue());
        }
    }
}


