// -----------------------------------------------------------------------------
//    ASPiK AU Shell File:  aufxplugin.cpp
//
/**
    \file   aufxplugin.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implementation file for the AUFXPlugin which is the ASPiK AU Shell object

			- included in ASPiK(TM) AU Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AU Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "ausynthplugin.h"

#define	SMTG_MAKE_STRING_PRIVATE_DONT_USE(x)	# x
#define	SMTG_MAKE_STRING(x)		SMTG_MAKE_STRING_PRIVATE_DONT_USE(x)

#pragma mark ____AUFXPlugin


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//--- Library Entry Point, see .exp file
AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, AUSynthPlugin)

// --- Factory preset: in case the plugin core does not specifically declare one
static const int kNumberPresets = 1;
static AUPreset kPresets[kNumberPresets] = {
    { 0, CFSTR("Factory Preset") }
};

#pragma mark ____Construction_Initialization
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::AUSynthPlugin
//
/**
\brief constructor for plugin object

 NOTES:
 - creates core
 - creates ALL ASPiK interfaces
 - sets up AU parameters
 - intitializes AU objecgt

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUSynthPlugin::AUSynthPlugin(AudioUnit component) : AUInstrumentBase(component, 0, 1) // No inputs, One output
{
    CreateElements(); // --- create input, output ports, groups and parts

    // Create the PluginCore
    pluginCore = new PluginCore;
    if(!pluginCore) return;

    // --- initialize the plugin core
    //     Currently, this only passes the folder location of the DLL to the core for storage and use
    //     for example to load samples or other non-internal plugin stuff
    const char* bundleIDStr = pluginCore->getAUBundleID();
    if(bundleIDStr)
    {
        CFStringRef bundleID = CFStringCreateWithCString(NULL, bundleIDStr, kCFStringEncodingASCII);
        PluginInfo initInfo;
        initInfo.pathToDLL = getMyComponentDirectory(bundleID);
        if(initInfo.pathToDLL)
        {
            pluginCore->initialize(initInfo);
            delete[] initInfo.pathToDLL;
        }
    }

    // --- setup IGUIPluginConnector - must be done early as possible
    guiPluginConnector = new GUIPluginConnector(this, pluginCore);
    midiEventQueue = new AUMIDIEventQueue(pluginCore);
    pluginHostConnector = new PluginHostConnector(this);

    pluginCore->setPluginHostConnector(pluginHostConnector);
    presetsArrayData = nullptr;

    if(pluginCore->getPresetCount() > 0)
    {
        // --- create space for the presets array
        presetsArrayData = malloc(pluginCore->getPresetCount()*sizeof(AUPreset));
        memset(presetsArrayData, 0, pluginCore->getPresetCount()*sizeof(AUPreset));
    }

    sidechainBufferList = nullptr;
    hasSidechain = false;
    sidechainChannelCount = 0;
    auChannelInfo = nullptr;
    pluginGUI = nullptr;

    inputBuffers = new float*[MAX_CHANNEL_COUNT];
    outputBuffers  = new float*[MAX_CHANNEL_COUNT];
    sidechainInputBuffers  = new float*[MAX_CHANNEL_COUNT];

    memset(&inputBuffers[0], 0, sizeof(float*)*MAX_CHANNEL_COUNT);
    memset(&outputBuffers[0], 0, sizeof(float*)*MAX_CHANNEL_COUNT);

    // --- setup sidechain
    memset(&sidechainInputBuffers[0], 0, sizeof(float*)*MAX_CHANNEL_COUNT);
    hasSidechain = pluginCore->hasSidechain();

    // --- NOTE: there is a known bug (feature?) in Logic 9 and Pro X: if the sidechain is set to No Input
    //           then the sidechain buffers will point to the same buffers as the audio input!
    if(hasSidechain)
    {
        SetBusCount(kAudioUnitScope_Input, 2);
        SafeGetElement(kAudioUnitScope_Input, 0)->SetName(CFSTR("Main Input"));
        SafeGetElement(kAudioUnitScope_Input, 1)->SetName(CFSTR("Sidechain"));
     }

    // --- final init
    initAUParametersWithPluginCore();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::~AUSynthPlugin
//
/**
 \brief destructor for plugin object

 NOTES:
 - destory core
 - destory all buffers

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUSynthPlugin::~AUSynthPlugin()
{
    // --- fixes reaper bug that destroys AU before closing GUI for final time
    if(pluginGUI)
    {
        pluginGUI->clearGUIPluginConnector();
        pluginGUI->close();
    }

    if(presetsArrayData)
    {
        free(presetsArrayData);
        presetsArrayData = nullptr;
    }

    // -- cio
    if(auChannelInfo)
    {
        delete [] auChannelInfo;
        auChannelInfo = nullptr;
    }

    if(pluginCore)
    {
        delete pluginCore;
        pluginCore = nullptr;
    }

    if(guiPluginConnector)
    {
        delete guiPluginConnector;
        guiPluginConnector = nullptr;
    }

    if(inputBuffers)
    {
        delete [] inputBuffers;
        inputBuffers = nullptr;
    }

    if(outputBuffers)
    {
        delete [] outputBuffers;
        inputBuffers = nullptr;
    }

    if(sidechainInputBuffers)
    {
        delete [] sidechainInputBuffers;
        sidechainInputBuffers = nullptr;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::Reset -> pluginCore->reset
//
/**
 \brief reset function for AU and core

 NOTES:
 - call base class
 - reset core
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::Reset(AudioUnitScope 	 inScope,
                                  AudioUnitElement   inElement)
{
    // --- reset the base class
    AUBase::Reset(inScope, inElement);

    if(pluginCore)
    {
        ResetInfo info;
        info.sampleRate = GetOutput(0)->GetStreamFormat().mSampleRate;
        info.bitDepth = GetOutput(0)->GetStreamFormat().SampleWordSize() * 8;

        pluginCore->reset(info);

        // --- AU Specific
        latencyInSeconds = pluginCore->getLatencyInSamples() / GetOutput(0)->GetStreamFormat().mSampleRate;
    }

    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::SupportedNumChannels
//
/**
 \brief return an array of AUChannelInfo structures with input and output channel combinations

 NOTES:
 - query the PluginCore for its channel setup information - it does all work
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit

 \return the number of AUChannelInfo structures in the array
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UInt32 AUSynthPlugin::SupportedNumChannels (const AUChannelInfo** outInfo)
{
    // --- set an array of arrays of different combinations of supported numbers
    //     of ins and outs
    if(!pluginCore) return 0;

    if(auChannelInfo)
        delete [] auChannelInfo;

    auChannelInfo = new AUChannelInfo[pluginCore->getNumSupportedIOCombinations()];
    for(int i=0; i<pluginCore->getNumSupportedIOCombinations(); i++)
    {
        auChannelInfo[i].inChannels = pluginCore->getInputChannelCount(i);
        auChannelInfo[i].outChannels = pluginCore->getOutputChannelCount(i);
    }

    if(outInfo)
        *outInfo = (const AUChannelInfo*)auChannelInfo;

    return pluginCore->getNumSupportedIOCombinations();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::Initialize
//
/**
 \brief the AU init function

 NOTES:
 - set the plugin sample rate and bit depth
 - set and store the plugin latency in seconds for AU host queries
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit

 \return the number of AUChannelInfo structures in the array
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::Initialize()
{
    if(pluginCore)
    {
        ResetInfo info;
        info.sampleRate = GetOutput(0)->GetStreamFormat().mSampleRate;
        info.bitDepth = GetOutput(0)->GetStreamFormat().SampleWordSize() * 8;

        pluginCore->reset(info);

        // --- AU Specific
        latencyInSeconds = pluginCore->getLatencyInSamples() / GetOutput(0)->GetStreamFormat().mSampleRate;
    }

    return AUInstrumentBase::Initialize();
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::RestoreState
//
/**
 \brief called when a user preset is updated; may also be called during init; note the
 call sequence depends on the host and AULab calls early init functions in a different order
 that Logic.

 NOTES:
 - just calls base class; additions reserved for future
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::RestoreState(CFPropertyListRef inData)
{
    ComponentResult result = AUBase::RestoreState(inData);
    return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::setAUParameterChangeEvent
//
/**
 \brief safely set an AU parameter change event

 NOTES:
 - thread-safe parameter change evengt
 - https://developer.apple.com/documentation/audiounit

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUSynthPlugin::setAUParameterChangeEvent(unsigned int controlID, double actualValue)
{
    AudioUnitParameter param = {this->GetComponentInstance(), static_cast<AudioUnitParameterID>(controlID), kAudioUnitScope_Global, 0};
    AUParameterSet(NULL, this, &param, actualValue, 0);
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::getAUParameter
//
/**
 \brief safely get an AU parameter's value

 \param controlID the AU parameter ID value

 \return the actual value of the parameter
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double AUSynthPlugin::getAUParameter(unsigned int controlID)
{
    return Globals()->GetParameter(controlID);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::initAUParametersWithPluginCore
//
/**
 \brief set up all AU parameters at once using plugin core's parameters; performed only once

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUSynthPlugin::initAUParametersWithPluginCore()
{
    if(!pluginCore) return;

    for(int32_t i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);

        if(piParam)
        {
            // --- AudioUnitParameterID = piParam->getControlID()
            //     AudioUnitParameterValue piParam->getControlValue()
            Globals()->SetParameter(piParam->getControlID(), piParam->getControlValue());
        }
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::updatePluginCoreParameters
//
/**
 \brief safely transfer AU parameter changes into the Core

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUSynthPlugin::updatePluginCoreParameters()
{
    if(!pluginCore) return;

    ParameterUpdateInfo paramInfo;
    paramInfo.bufferProcUpdate = true;

    for(int32_t i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);

        if(piParam)
        {
            // --- get the threadsafe global parameter
            AudioUnitParameterValue inValue = Globals()->GetParameter(piParam->getControlID());

            // --- set in core's parameter
            pluginCore->updatePluginParameter(piParam->getControlID(), inValue, paramInfo);
        }
    }
}



///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::updateAUParametersWithPluginCore
//
/**
 \brief outoing transfer AU parameter changes (meter data) from the Core

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUSynthPlugin::updateAUParametersWithPluginCore() // outbound
{
    if(!pluginCore) return;

    int32_t startIndex = 0;
    while(startIndex >= 0)
    {
        PluginParameter* piParam = pluginCore->getNextParameterOfType(startIndex, controlVariableType::kMeter);

        if(piParam)
            Globals()->SetParameter(piParam->getControlID(), piParam->getControlValue());
    }
}

#pragma mark ____AU Functions: Process Audio

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::Render -> this is the main render funciton for Synth plugins
//
/**
 \brief first function to be called during buffer process cycle

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::Render(AudioUnitRenderActionFlags &		ioActionFlags,
                                  const AudioTimeStamp &            inTimeStamp,
                                  UInt32                            inNumberFrames)
{
    if(!pluginCore) return noErr;

    // --- threadsafe sync to globals
    updatePluginCoreParameters();

    // --- switch double-queue
    if(midiEventQueue)
        midiEventQueue->toggleQueue();

    // --- audio processing
    ProcessBufferInfo info;

    // --- get the number of channels
    AudioBufferList& bufferList = GetOutput(0)->GetBufferList();
    size_t numInputChannels = 0;//(size_t) GetInput(0)->GetStreamFormat().mChannelsPerFrame;
    size_t numOutputChannels = bufferList.mNumberBuffers;
    size_t numAuxInputChannels = 0;

    for(int i=0; i<numOutputChannels; i++)
    {
        outputBuffers[i] = (float*)bufferList.mBuffers[i].mData;//
    }

    info.inputs = inputBuffers;
    info.outputs = outputBuffers;
    info.auxInputs = sidechainInputBuffers;
    info.auxOutputs = nullptr; // --- for future use

    // --- may not even need this...
    info.numAudioInChannels = numInputChannels;
    info.numAudioOutChannels = numOutputChannels;
    info.numAuxAudioInChannels = 0;
    info.numAuxAudioOutChannels = 0; // --- for future use

    info.numFramesToProcess = inNumberFrames;

    // --- setup the channel configs
    info.channelIOConfig.inputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(info.numAudioInChannels);
    info.channelIOConfig.outputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(info.numAudioOutChannels);

    // --- sidechain channel config
    info.auxChannelIOConfig.inputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(sidechainChannelCount);
    info.auxChannelIOConfig.outputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(0);

    info.midiEventQueue = midiEventQueue;

    // --- NEED HOST INFO STUFF
    HostInfo hostInfo;// = {0};
    updateHostInfo(&hostInfo);
    info.hostInfo = &hostInfo;

    // --- process the buffers
    pluginCore->processAudioBuffers(info);

    // --- thread safe update to meters
    updateAUParametersWithPluginCore();

    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::updateHostInfo
//
/**
 \brief update this buffer run's incoming host data (BPM, etc...)

 \param hostInfo a HostInfo structure to fill with incoming data
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AUSynthPlugin::updateHostInfo(HostInfo* hostInfo)
{
    if(!hostInfo) return;
    memset(hostInfo, 0, sizeof(HostInfo));

    Float64 outCurrentBeat = 0.0;
    Float64 outCurrentTempo = 0.0;
    UInt32  outDeltaSampleOffsetToNextBeat = 0.0;
    Float32 outTimeSig_Numerator = 0.0;
    UInt32  outTimeSig_Denominator = 0.0;
    Float64 outCurrentMeasureDownBeat = 0.0;
    Boolean outIsPlaying = false;
    Boolean outTransportStateChanged = false;
    Float64 outCurrentSampleInTimeLine = 0.0;
    Boolean outIsCycling = false;
    Float64 outCycleStartBeat = 0.0;
    Float64 outCycleEndBeat = 0.0;

    OSStatus status = CallHostBeatAndTempo(&outCurrentBeat, &outCurrentTempo);
    if(status == noErr)
    {
        hostInfo->dBPM = outCurrentTempo;
        hostInfo->dCurrentBeat = outCurrentBeat;
    }

    status = CallHostMusicalTimeLocation(&outDeltaSampleOffsetToNextBeat, &outTimeSig_Numerator, &outTimeSig_Denominator, &outCurrentMeasureDownBeat);
    if(status == noErr)
    {
        hostInfo->dCurrentMeasureDownBeat = outCurrentMeasureDownBeat;
        hostInfo->nDeltaSampleOffsetToNextBeat = outDeltaSampleOffsetToNextBeat;
        hostInfo->fTimeSigNumerator = outTimeSig_Numerator;
        hostInfo->uTimeSigDenomintor = outTimeSig_Denominator;
    }

    status = CallHostTransportState (&outIsPlaying, &outTransportStateChanged, &outCurrentSampleInTimeLine, &outIsCycling, &outCycleStartBeat, &outCycleEndBeat);
    {
        hostInfo->dCycleEndBeat = outCycleEndBeat;
        hostInfo->dCycleStartBeat = outCycleStartBeat;
        hostInfo->bIsCycling = outIsCycling;
        hostInfo->bTransportStateChanged = outTransportStateChanged;
        hostInfo->bIsPlayingAU = outIsPlaying;
        hostInfo->uAbsoluteFrameBufferIndex = outCurrentSampleInTimeLine;
        hostInfo->dAbsoluteFrameBufferTime = outCurrentSampleInTimeLine/GetOutput(0)->GetStreamFormat().mSampleRate;
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::SetParameter
//
/**
 \brief this just calls base class

 NOTES:
 - do not be tempted into using this for plugin core sync; this function should not be
 modified
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult AUSynthPlugin::SetParameter(AudioUnitParameterID	   inID,
                                               AudioUnitScope 		   inScope,
                                               AudioUnitElement 	   inElement,
                                               AudioUnitParameterValue inValue,
                                               UInt32				   inBufferOffsetInFrames)
{
    return AUBase::SetParameter(inID, inScope, inElement, inValue, inBufferOffsetInFrames);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::GetParameterInfo
//
/**
 \brief get information about each AU parameter that was initialized

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::GetParameterInfo(AudioUnitScope		   inScope,
                                                   AudioUnitParameterID    inParameterID,
                                                   AudioUnitParameterInfo& outParameterInfo )
{
    // --- here, the client is querying us for each of our controls. It wants a description
    //     (name) and I have set it up for custom units since that's the most general so
    //     we also give it units.
    //

    ComponentResult result = noErr;
    if(!pluginCore) return kAudioUnitErr_InvalidParameter;

    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable + kAudioUnitParameterFlag_IsReadable;

    if(inScope == kAudioUnitScope_Global)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByControlID(inParameterID);
        if(!piParam) return kAudioUnitErr_InvalidParameter;

        // --- make the name objects
        CFStringRef name = CFStringCreateWithCString(NULL, piParam->getControlName(), kCFStringEncodingASCII);
        CFStringRef units = CFStringCreateWithCString(NULL, piParam->getControlUnits(), kCFStringEncodingASCII);

        // --- fill in the name; you have to call a function to do this
        AUBase::FillInParameterName (outParameterInfo, name, false);

        // --- if unsigned int data, tell it we are Indexed; this will make it query us for
        //     strings to fill a dropdown control; those strings are chunks of your
        //     enum string for that control
        if(piParam->getControlVariableType() == controlVariableType::kTypedEnumStringList)
            outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
        else
        {
            // --- custom, set units
            outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit;
            outParameterInfo.unitName = units;
        }

        // --- set min and max
        outParameterInfo.minValue = piParam->getMinValue();
        outParameterInfo.maxValue = piParam->getMaxValue();
        outParameterInfo.defaultValue = piParam->getDefaultValue(); // or just getControlValue() ??

        // --- rest of flags
        outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
    }

    return result;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::GetParameterValueStrings
//
/**
 \brief get parameter string-lists (for string-list params only)

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
  - https://developer.apple.com/documentation/audiounit
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::GetParameterValueStrings(AudioUnitScope       inScope,
                                                           AudioUnitParameterID	inParameterID,
                                                           CFArrayRef *			outStrings)
{
    if(!pluginCore) return kAudioUnitErr_InvalidParameter;

    if(inScope == kAudioUnitScope_Global)
    {
        if (outStrings == NULL)
            return noErr;

        PluginParameter* piParam = pluginCore->getPluginParameterByControlID(inParameterID);
        if(!piParam) return kAudioUnitErr_InvalidParameter;

        // --- convert the list into an array
        CFStringRef enumList = CFStringCreateWithCString(NULL, piParam->getCommaSeparatedStringList(), kCFStringEncodingASCII);
        CFStringRef comma CFSTR(",");
        CFArrayRef strings = CFStringCreateArrayBySeparatingStrings(NULL, enumList, comma);

        // --- create the array COPY (important: these are local variables above!)
        *outStrings = CFArrayCreateCopy(NULL, strings);

        return noErr;
    }


    return kAudioUnitErr_InvalidParameter;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::GetPropertyInfo
//
/**
 \brief queries from host about plugin properties

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::GetPropertyInfo(AudioUnitPropertyID inID,
                                                  AudioUnitScope      inScope,
                                                  AudioUnitElement    inElement,
                                                  UInt32&             outDataSize,
                                                  Boolean&            outWritable)
{
    if(!pluginCore) return kAudioUnitErr_InvalidParameter;

    if (inScope == kAudioUnitScope_Global)
    {
        switch(inID)
        {
            // --- we have a Cocoa GUI
            case kAudioUnitProperty_CocoaUI:
            {
                if(pluginCore->hasCustomGUI())
                {
                    outWritable = false;
                    outDataSize = sizeof(AudioUnitCocoaViewInfo);
                    return noErr;
                }
            }
            case kOpenGUI:
            {
                #if __LP64__
                outDataSize = sizeof(uint64_t);
                #else
                outDataSize = sizeof(uint32_t);
                #endif

                outWritable = false;
                return noErr;
            }
        }
    }
    return AUInstrumentBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::GetProperty
//
/**
 \brief queries from host to get property information

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::GetProperty(AudioUnitPropertyID       inID,
                                              AudioUnitScope      inScope,
                                              AudioUnitElement    inElement,
                                              void*               outData)
{
    if (inScope == kAudioUnitScope_Global)
    {
        switch(inID)
         {
             // --- This property allows the host application to find the UI associated with this
             case kAudioUnitProperty_CocoaUI:
             {
                 // --- Look for a resource in the main bundle by name and type.
                  CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFStringCreateWithCString(nullptr, pluginCore->getAUBundleID(), kCFStringEncodingASCII));

                 if(bundle == NULL) return fnfErr;

                 CFURLRef bundleURL = CFBundleCopyResourceURL(bundle,
                                                               CFStringCreateWithCString(nullptr, pluginCore->getAUBundleName(), kCFStringEncodingASCII),
                                                               CFSTR("bundle"),
                                                               NULL);

                 if(bundleURL == NULL) return fnfErr;

                 CFStringRef className = CFStringCreateWithCString(nullptr, pluginCore->getAUCocoaViewFactoryName(), kCFStringEncodingASCII);

                 AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, {className} };
                 *((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;

                 return noErr;
             }

             return kAudioUnitErr_InvalidProperty;
         }
    }
    return AUInstrumentBase::GetProperty(inID, inScope, inElement, outData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::SetProperty
//
/**
 \brief open and close the GUI object

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus	AUSynthPlugin::SetProperty(AudioUnitPropertyID inID,
                                          AudioUnitScope 	  inScope,
                                          AudioUnitElement 	  inElement,
                                          const void*		  inData,
                                          UInt32 			  inDataSize)
{
   	if (inScope == kAudioUnitScope_Global)
    {
        if(!pluginCore) return kAudioUnitErr_InvalidParameter;

        switch (inID)
        {
            case kOpenGUI:
            {
                VIEW_STRUCT* pVS = (VIEW_STRUCT*)inData;
                void* createdCustomGUI = NULL;
                if(!createdCustomGUI)
                {
                    // --- Look for a resource in the main bundle by name and type.
                    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFStringCreateWithCString(nullptr, pluginCore->getAUBundleID(), kCFStringEncodingASCII));
                    if (bundle == nullptr) return fnfErr;

                    // --- get .uidesc file
                    CFURLRef bundleURL = CFBundleCopyResourceURL(bundle,CFSTR("PluginGUI"),CFSTR("uidesc"),NULL);
                    CFStringRef xmlPath = CFURLCopyPath(bundleURL);
                    int nSize = CFStringGetLength(xmlPath);
                    char* path = new char[nSize+1];
                    memset(path, 0, (nSize+1)*sizeof(char));
                    CFStringGetCString(xmlPath, path, nSize+1, kCFStringEncodingASCII);
                    CFRelease(xmlPath);

                    std::vector<PluginParameter*>* PluginParameterPtr = pluginCore->makePluginParameterVectorCopy();

                    // --- create GUI
                    pluginGUI = new VSTGUI::PluginGUI(path);
                    pluginGUI->setGUIWindowFrame(pVS->pGUIFrame);

                    if(pluginGUI)
                    {
                        bool openedGUI = pluginGUI->open("Editor", pVS->pWindow, PluginParameterPtr, VSTGUI::kNSView, guiPluginConnector, pVS->au);

                        // --- delete the PluginParameterPtr guts, and pointer too...
                        for(std::vector<PluginParameter*>::iterator it = PluginParameterPtr->begin(); it !=  PluginParameterPtr->end(); ++it)
                        {
                            delete *it;
                        }
                        delete PluginParameterPtr;

                        if(openedGUI)
                        {
                            pluginGUI->getSize(pVS->width, pVS->height);
                            pluginGUI->setAU(pVS->au);

                            // --- frame -> view notifications
                            pVS->pGUIView = pluginGUI;

                            // --- let plugin core know
                            if(guiPluginConnector)
                                guiPluginConnector->guiDidOpen();
                        }

                        delete [] path;
                        return noErr;
                    }
                }

                break;
            }
            case kCloseGUI:
            {
                if(guiPluginConnector)
                    guiPluginConnector->guiWillClose();

                // --- destroy GUI
                if(pluginGUI)
                {
                    // --- close it
                    pluginGUI->close();

                    // --- ref count should be exactly 1
                    if(pluginGUI->getNbReference() != 1)
                        Assert(true, "ASSERT FAILED: pluginGUI->getNbReference() != 1");

                    // --- self-destruct
                    VSTGUI::PluginGUI* oldGUI = pluginGUI;
                    pluginGUI = 0;
                    oldGUI->forget();

                }
                return noErr;
                break;
            }
        }
    }

    return kAudioUnitErr_InvalidProperty;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::GetPresets
//
/**
 \brief return a static array of preset information structures

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult	AUSynthPlugin::GetPresets(CFArrayRef *outData) const
{
    // --- this is used to determine if presets are supported
    //     which in this unit they are so we implement this method!
    if(outData == NULL) return noErr;
    if(!pluginCore) return noErr;

    if(pluginCore->getPresetCount() <= 0)
    {
        // make the array
        CFMutableArrayRef theArray = CFArrayCreateMutable (NULL, kNumberPresets, NULL);

        // copy our preset names
        for (int i = 0; i < kNumberPresets; ++i)
        {
            CFArrayAppendValue (theArray, &kPresets[i]);
        }

        *outData = (CFArrayRef)theArray;
        return noErr;
    }

    // make the array
    CFMutableArrayRef theArray = CFArrayCreateMutable (NULL, pluginCore->getPresetCount(), NULL);

    // copy our preset names
    for (int i = 0; i < pluginCore->getPresetCount(); ++i)
    {
        // --- presetsArrayData is a void*-ed block of raw bytes; will be freed in destructor
        ((AUPreset*)presetsArrayData)[i].presetNumber = i;
        ((AUPreset*)presetsArrayData)[i].presetName = CFStringCreateWithCString(NULL, pluginCore->getPresetName(i), kCFStringEncodingASCII);

        CFArrayAppendValue(theArray, (AUPreset*)presetsArrayData+i); // note pointer addition to move to next chunk
    }

    *outData = (CFArrayRef)theArray;
    return noErr;

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::NewFactoryPresetSet
//
/**
 \brief user has selected a new preset

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::NewFactoryPresetSet(const AUPreset & inNewFactoryPreset)
{
    if(!pluginCore) return noErr;

    SInt32 chosenPreset = inNewFactoryPreset.presetNumber;

    currentPreset = chosenPreset;

    if (chosenPreset < 0 || chosenPreset >= pluginCore->getPresetCount())
        return kAudioUnitErr_InvalidPropertyValue;

    if(pluginCore->getPresetCount() <= 0) return kAudioUnitErr_InvalidPropertyValue;

    // --- Sync Preset Name
    AUPreset auPreset = { 0, CFSTR("Factory Preset") };

    if(pluginCore->getPresetCount() > 0)
    {
        auPreset.presetNumber = currentPreset;
        auPreset.presetName = CFStringCreateWithCString(NULL, pluginCore->getPresetName(currentPreset), kCFStringEncodingASCII);
    }

    // --- set name
    SetAFactoryPresetAsCurrent(auPreset);

    PresetInfo* preset = pluginCore->getPreset(currentPreset);
    if(preset)
    {
        for(int j=0; j<preset->presetParameters.size(); j++)
        {
            PresetParameter preParam = preset->presetParameters[j];
            Globals()->SetParameter(preParam.controlID, preParam.actualValue);
        }
    }

    return noErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::HandleNoteOn
//
/**
 \brief specialized MIDI handler for only this message; CURRENTLY NOT USED, see HandleMidiEvent

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::HandleNoteOn(UInt8 	inChannel,
                                        UInt8 	inNoteNumber,
                                        UInt8 	inVelocity,
                                        UInt32  inStartFrame)
{
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::HandleNoteOff
//
/**
 \brief specialized MIDI handler for only this message; CURRENTLY NOT USED, see HandleMidiEvent

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::HandleNoteOff(UInt8 	inChannel,
                                         UInt8 	inNoteNumber,
                                         UInt8 	inVelocity,
                                         UInt32 inStartFrame)
{
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::HandleNoteOff
//
/**
 \brief specialized MIDI handler for only this message; CURRENTLY NOT USED, see HandleMidiEvent

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::HandlePitchWheel(UInt8  inChannel,
                                            UInt8  inPitch1,
                                            UInt8  inPitch2,
                                            UInt32 inStartFrame)
{
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::HandleControlChange
//
/**
 \brief specialized MIDI handler for only this message; CURRENTLY NOT USED, see HandleMidiEvent

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
 */
/*
 // --- NOTE: Logic hooks the Volume and Pan controls
 // --- But since MIDI CC 7 and 10 (volume and pan respectively) are reserved by the main channel strip controls,
 //     it's best to use MIDI CC 11 (expression) to automate volume effects
 //     http://www.soundonsound.com/sos/apr08/articles/logictech_0408.htm
 //
 On some plugins and instruments, CC#11 does nothing but control volume. On other plugins/instruments, CC#11 is programmed to control volume and timbre (brightness) simultaneously. This is a feature of the programming of the plugin or instrument and not an inherent quality of CC#11 data. In such a case, higher CC#11 values make a sound both louder and brighter, and vice versa. If in fact your instruments respond to CC#11 only with a change in volume then you might as well not try and fight city hall: use CC#11 as your volume control.
 */
// --- CC handler
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AUSynthPlugin::HandleControlChange(UInt8  inChannel,
                                               UInt8  inController,
                                               UInt8  inValue,
                                               UInt32 inStartFrame)
{

    return noErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynthPlugin::HandleMidiEvent
//
/**
 \brief specialized MIDI handler to add events to the plugin's queue

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AU Programming Guide
 - see AU SDK for more information on this function and its parameters
 - https://developer.apple.com/documentation/audiounit
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUSynthPlugin::HandleMidiEvent(UInt8  status,
                                           UInt8  channel,
                                           UInt8  data1,
                                           UInt8  data2,
                                           UInt32 inStartFrame)
{
    if(midiEventQueue)
    {
        midiEvent event;
        event.midiMessage = (unsigned int)status;
        event.midiChannel = (unsigned int)channel;
        event.midiData1 = (unsigned int)data1;
        event.midiData2 = (unsigned int)data2;;
        event.midiSampleOffset = inStartFrame;
        midiEventQueue->addEvent(event);
    }


    // --- call base class to do its thing
    return AUInstrumentBase::HandleMidiEvent(status, channel, data1, data2, inStartFrame);
}







