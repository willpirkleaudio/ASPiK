// --- header


#include "vst3plugin.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "base/source/fstreamer.h"

// --- plugin specific
#include "customparameters.h"
#include "channelformats.h"

enum { kPresetParam = 'prst' };

namespace Steinberg {
namespace Vst {
namespace ASPiK {

// --- for versioning in serialization
static uint64 VSTPluginVersion = 0;		///< VST versioning for serialization
static FUID* VST3PluginCID = nullptr;	///< the FUID
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::VST3Plugin
//
/**
\brief object constructor: because of class factory, do NOT use this for init; use initialize() instead

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VST3Plugin::VST3Plugin()
{
    // --- VST3.7 support for IProcessContextRequirements
    //
    // --- these are variables that are transferred into the
    //     PluginCore's HostInfo structure for sync to BPM and
    //     other uses
    // --- See the processcontextrequirements.h file in the VST3 SDK
    //     for more options (e.g. transport state)∫.
    processContextRequirements.needTempo();
    processContextRequirements.wantsTimeSignature();
    processContextRequirements.wantsContinousTimeSamples();
    processContextRequirements.wantsSystemTime();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::~VST3Plugin
//
/**
\brief object destructor: because of class factory, do NOT use this for destruction; 
use terminate() instead

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VST3Plugin::~VST3Plugin()
{
}
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::initialize
//
/**
\brief object initializer

NOTES:
- Call the base class
- Add a Stereo Audio Output
- Add a MIDI event inputs (16: one for each channel)
- Add GUI parameters (EditController part)
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::initialize(FUnknown* context)
{
    // --- inits
    m_pParamUpdateQueueArray = nullptr;
    hasSidechain = false;
    enableSAAVST3 = false;
    sampleAccuracy = 1;
	plugInSideBypass = false;
    
    // --- create the core
    pluginCore = new PluginCore;
    if(!pluginCore)
        return kResultFalse; // fail
    
    // --- initialize the plugin core
    //     Currently, this only passes the folder location of the DLL to the core for storage and use
    //     for example to load samples or other non-internal plugin stuff
    PluginInfo initInfo;
#ifdef MAC
    const char* bundleIDStr = pluginCore->getVST3BundleID();
    if (bundleIDStr)
    {
        CFStringRef bundleID = CFStringCreateWithCString(NULL, bundleIDStr, kCFStringEncodingASCII);
        initInfo.pathToDLL = getMyComponentDirectory(bundleID);
    }
#else // Windows
#ifdef ENABLE_WINDOWS_H
    std::string strBundleName(pluginCore->getPluginBundleName());
    strBundleName.append(".vst3");

    initInfo.pathToDLL = getMyDLLDirectory(USTRING(strBundleName.c_str()));
#endif
#endif
    // --- send to core
    if (initInfo.pathToDLL)
        pluginCore->initialize(initInfo);
    

    // --- setup IGUIPluginConnector - must be done early as possible
    guiPluginConnector = new GUIPluginConnector(pluginCore, this);
    pluginHostConnector = new PluginHostConnector(this);
    pluginCore->setPluginHostConnector(pluginHostConnector);
    
    // --- MIDI queue setup
    midiEventQueue = new VSTMIDIEventQueue(pluginCore);
    
    // --- setup needs
    m_uLatencyInSamples = pluginCore->getLatencyInSamples();
    hasSidechain = pluginCore->hasSidechain();
    enableSAAVST3 = pluginCore->wantsVST3SampleAccurateAutomation();
    sampleAccuracy = pluginCore->getVST3SampleAccuracyGranularity();

    tresult result = SingleComponentEffect::initialize(context);

	if(result == kResultTrue)
	{
        // --- can get name of host with this:
        FUnknownPtr<Vst::IHostApplication> hostApplication(context);
        if(hostApplication)
        {
            String128 name;
            hostApplication->getName(name);
        }
        
        // --- BUSS SETUP
        // --- our plugin wants one input bus and one output bus
        //     so we default to stereo here, however we will refine the
        //     buss formats later during querying of VST3Plugin::setBusArrangements()
        //     below. This is where we ask the core what it supports
        removeAudioBusses();
        if (pluginCore->getPluginType() == kSynthPlugin)
        {
            addAudioOutput(STR16("Stereo Output"), SpeakerArr::kStereo);
        }
        else // FX
        {
            addAudioInput(STR16("Stereo Input"), SpeakerArr::kStereo);
            addAudioOutput(STR16("Stereo Output"), SpeakerArr::kStereo);
        }
        
        // --- sidechain bus is stereo
        if(hasSidechain)
            addAudioInput(STR16("AuxInput"), SpeakerArr::kStereo, kAux);

		// --- MIDI event input bus, 16 channels (note we support MIDI for all plugin types)
		addEventInput(STR16("Event Input"), 16);
       
        // --- create the queue
        if (enableSAAVST3)
        {
            m_pParamUpdateQueueArray = new VSTParamUpdateQueue *[pluginCore->getPluginParameterCount()];
            memset(m_pParamUpdateQueueArray, 0, sizeof(VSTParamUpdateQueue *) * pluginCore->getPluginParameterCount());
        }

        // --- with custom GUI, theP luginGUI object will handle details
		for (unsigned int i = 0; i < pluginCore->getPluginParameterCount(); i++)
        {
            PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);

            if(piParam)
            {
                // --- sample accurate automation
                if (enableSAAVST3)
                {
                    m_pParamUpdateQueueArray[i] = new VSTParamUpdateQueue();
                    m_pParamUpdateQueueArray[i]->initialize(piParam->getDefaultValue(), piParam->getMinValue(), piParam->getMaxValue(), &sampleAccuracy);
                }
                
                // --- you can choose to register non-bound controls as parameters
                if(piParam->isNonVariableBoundParam())
                {
                    PeakParameter* peakParam = new PeakParameter(ParameterInfo::kIsReadOnly, piParam->getControlID(), USTRING(piParam->getControlName()));
                    peakParam->setNormalized(0.0);
                    parameters.addParameter(peakParam);
                }
                else if(piParam->isMeterParam())
                {
                    PeakParameter* peakParam = new PeakParameter(ParameterInfo::kIsReadOnly, piParam->getControlID(), USTRING(piParam->getControlName()));
                    peakParam->setNormalized(0.0);
                    parameters.addParameter(peakParam);
                }
                else if(piParam->isStringListParam())
                {
                    StringListParameter* enumStringParam = new StringListParameter(USTRING(piParam->getControlName()), piParam->getControlID());
					size_t numStrings = piParam->getStringCount();
                    for (unsigned int j=0; j<numStrings; j++)
                    {
                        std::string stringParam = piParam->getStringByIndex(j);
                        enumStringParam->appendString(USTRING(stringParam.c_str()));
                    }
                    parameters.addParameter(enumStringParam);

                }
                else if(piParam->isLogTaper())
                {
                    Parameter* param = new LogParameter(USTRING(piParam->getControlName()),
                                                        piParam->getControlID(),
                                                        USTRING(piParam->getControlUnits()),
                                                        piParam->getMinValue(),
                                                        piParam->getMaxValue(),
                                                        piParam->getDefaultValue());

                    param->setPrecision(piParam->isIntParam() ? 0 : piParam->getDisplayPrecision()); // fractional sig digits
                    parameters.addParameter(param);

                }
                else if(piParam->isAntiLogTaper())
                {
                     Parameter* param = new AntiLogParameter(USTRING(piParam->getControlName()),
                                                            piParam->getControlID(),
                                                            USTRING(piParam->getControlUnits()),
                                                            piParam->getMinValue(),
                                                            piParam->getMaxValue(),
                                                            piParam->getDefaultValue());

                    param->setPrecision(piParam->isIntParam() ? 0 : piParam->getDisplayPrecision()); // fractional sig digits
                    parameters.addParameter(param);
                }
                else if(piParam->isVoltOctaveTaper())
                {
   
                    Parameter* param = new VoltOctaveParameter(USTRING(piParam->getControlName()),
                                                               piParam->getControlID(),
                                                               USTRING(piParam->getControlUnits()),
                                                               piParam->getMinValue(),
                                                               piParam->getMaxValue(),
                                                               piParam->getDefaultValue());
   
                    param->setPrecision(piParam->isIntParam() ? 0 : piParam->getDisplayPrecision()); // fractional sig digits
                    parameters.addParameter(param);
                }
                else //  linear
                {
                   Parameter* param = new RangeParameter(USTRING(piParam->getControlName()),
                                                         piParam->getControlID(),
                                                         USTRING(piParam->getControlUnits()),
                                                         piParam->getMinValue(),
                                                         piParam->getMaxValue(),
                                                         piParam->getDefaultValue());
  
                    param->setPrecision(piParam->isIntParam() ? 0 : piParam->getDisplayPrecision()); // fractional sig digits
                    parameters.addParameter(param);
                }
            }
        }
        
        // --- one and only bypass parameter
        Parameter* param = new RangeParameter(USTRING("Bypass"), PLUGIN_SIDE_BYPASS, USTRING(""),
                                   0, 1, 0, 0, ParameterInfo::kCanAutomate|ParameterInfo::kIsBypass);
        parameters.addParameter(param);
        
        // --- root
        UnitInfo uinfoRoot;
        uinfoRoot.id = 1;
        uinfoRoot.parentUnitId = kRootUnitId;
        uinfoRoot.programListId = kNoProgramListId;
        Steinberg::UString (uinfoRoot.name, USTRINGSIZE (uinfoRoot.name)).assign (USTRING ("RootUnit"));
        addUnit(new Unit (uinfoRoot));

        // --- presets
        if(pluginCore->getPresetCount() > 0)
        {
            // --- add presets
            UnitInfo uinfoPreset;
            uinfoPreset.id = kRootUnitId;
            uinfoPreset.parentUnitId = 1;
            uinfoPreset.programListId = kPresetParam;
            UString name(uinfoPreset.name, 128);
            name.fromAscii("PresetsUnit");
            addUnit(new Unit (uinfoPreset));

//         //   if(false)
//         //   {
                ProgramList* prgList = new ProgramList (String ("Bank"), PRESET_NAME, kRootUnitId);
                addProgramList (prgList);

           //     prgList->addProgram (title);
                for (unsigned int i = 0; i<pluginCore->getPresetCount(); i++)
                {
                    prgList->addProgram (USTRING(pluginCore->getPresetName(i)));

                    //presetParam->appendString(USTRING(pluginCore->getPresetName(i)));
                }

                //---Program Change parameter---
                Parameter* prgParam = prgList->getParameter ();

                // by default this program change parameter if automatable we can overwrite this:
                prgParam->getInfo ().flags &= ~ParameterInfo::kCanAutomate;

                parameters.addParameter (prgParam);
//       //     }
        //    else
        //    {
                // --- the PRESET parameter
                StringListParameter* presetParam = new StringListParameter(USTRING("Factory Presets"),
                                                                           kPresetParam, USTRING(""),
                                                                           ParameterInfo::kIsProgramChange | ParameterInfo::kIsList,
                                                                           kRootUnitId);
                // --- enumerate names
                int count = pluginCore->getPresetCount();

                for (unsigned int i = 0; i<pluginCore->getPresetCount(); i++)
                {
                    presetParam->appendString(USTRING(pluginCore->getPresetName(i)));
                }

                //  --- add preset
                parameters.addParameter(presetParam);
          //  }
        }
    }

    return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::terminate
//
/**
\brief object destroyer

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::terminate()
{
    if(VST3PluginCID)
    {
        delete VST3PluginCID;
        VST3PluginCID = nullptr;
    }
    
    if(pluginCore)
    {
        // --- sample accurate automation
		for (unsigned int i = 0; i < pluginCore->getPluginParameterCount(); i++)
        {
            if (m_pParamUpdateQueueArray)
            {
                if (m_pParamUpdateQueueArray[i])
                    delete m_pParamUpdateQueueArray[i];
            }
        }
    }
    
    if(m_pParamUpdateQueueArray)
        delete[] m_pParamUpdateQueueArray;
    
    if(pluginCore) delete pluginCore;
    if(guiPluginConnector) delete guiPluginConnector;
    if(midiEventQueue) delete midiEventQueue;
    if(pluginHostConnector) delete pluginHostConnector;
    
    return SingleComponentEffect::terminate();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setBusArrangements
//
/**
\brief Client queries us for our supported Busses; this is where you can modify to support mono, surround, etc...

NOTES:
- The host will query upwards until it finds the target, for example if the audio file is 5.1, the
host queries mono, stereo, LCR, LCR + LFE, BFormat, etc . . . then 5.1, giving us the chance to
process as many inputs as possible. If we support 5.1, it will stop there (target). Note
that some DAWs might reject the target if the audio adapter does not support it.
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
\param numIns = number of input busses
\param numOuts = number of output busses
\param inputs[x] -->  requested input channel format for that bus
\param outputs[x] --> requested output channel format for that bus
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setBusArrangements(SpeakerArrangement* inputs, int32 numIns,
                                                  SpeakerArrangement* outputs, int32 numOuts)
{
    // --- FX: we support one input and one output bus
    if(numIns == 1 && numOuts == 1)
    {
        CString inStr = SpeakerArr::getSpeakerArrangementString(inputs[0], false);
        CString outStr = SpeakerArr::getSpeakerArrangementString(outputs[0], false);
        
        uint32_t inFormat = getChannelFormatForSpkrArrangement(inputs[0]);
        uint32_t outFormat = getChannelFormatForSpkrArrangement(outputs[0]);
        
        // --- does plugin support this format?
        if(pluginCore->hasSupportedInputChannelFormat(inFormat) &&
           pluginCore->hasSupportedOutputChannelFormat(outFormat))
        {
            removeAudioBusses();
            std::string strInput(inStr);
            strInput.append(" Input");
            addAudioInput(USTRING(strInput.c_str()), inputs[0]);

            std::string strOutput(outStr);
            strOutput.append(" Output");
            addAudioOutput(USTRING(strOutput.c_str()), outputs[0]);
            
            return kResultTrue;
        }
    }
    else if(numIns == 0 && numOuts == 1)    // --- SYNTH: we support one output bus, NO input
     {
        CString outStr = SpeakerArr::getSpeakerArrangementString(outputs[0], false);
        uint32_t outFormat = getChannelFormatForSpkrArrangement(outputs[0]);
         
         // --- does plugin support this format?
         if(pluginCore->hasSupportedOutputChannelFormat(outFormat))
         {
             removeAudioBusses();
             std::string strOutput(outStr);
             strOutput.append(" Output");
             addAudioOutput(USTRING(strOutput.c_str()), outputs[0]);
             
             return kResultTrue;
         }
     }
    else // not supported, we'll default to stereo I/O
    {
        removeAudioBusses ();
        addAudioInput  (STR16 ("Stereo In"),  SpeakerArr::kStereo);
        addAudioOutput (STR16 ("Stereo Out"), SpeakerArr::kStereo);
        
        return kResultFalse;
    }
    
    // else not supported bus count (should never happen, according to SDK
    return kResultFalse;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::canProcessSampleSize
//
/**
\brief Client queries us for our supported sample lengths

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::canProcessSampleSize(int32 symbolicSampleSize)
{
	// --- we support 32 bit audio
	if (symbolicSampleSize == kSample32)
	{
		return kResultTrue;
	}
	return kResultFalse;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setupProcessing
//
/**
\brief we get information about sample rate, bit-depth, etc...

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setupProcessing(ProcessSetup& newSetup)
{
    // --- reset
    if(pluginCore)
    {
        ResetInfo info;
        info.sampleRate = processSetup.sampleRate;
        info.bitDepth = processSetup.symbolicSampleSize;
        pluginCore->reset(info);
    }

	// --- base class
	return SingleComponentEffect::setupProcessing(newSetup);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getTailSamples
//
/**
\brief Returns the tail-time in samples.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
uint32 PLUGIN_API VST3Plugin::getTailSamples()
{
	if (pluginCore)
	{
		// --- check for infinte tail
		if (pluginCore->wantsInfiniteTailVST3())
			return kInfiniteTail;

		// --- else return our setting
		return (uint32)(processSetup.sampleRate*(pluginCore->getTailTimeInMSec() / 1000.0));
	}
    else
        return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setActive
//
/**
\brief VST3 plugins may be turned on or off; you are supposed to dynamically delare stuff when activated
	then delete when de-activated.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setActive(TBool state)
{

    // --- do ON stuff
	if(state)
	{
        // --- reset
        if(pluginCore)
        {
            ResetInfo info;
            info.sampleRate = processSetup.sampleRate;
            info.bitDepth = processSetup.symbolicSampleSize;
            pluginCore->reset(info);
        }
	}
	else
	{
 		// --- do OFF stuff;
	}

	// --- base class method call is last
	return SingleComponentEffect::setActive (state);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setState
//
/**
\brief This is the READ part of the serialization process. We get the stream interface and use it
	   to read from the filestream.

NOTES:
- The datatypes/read order must EXACTLY match the getState() version or crashes may happen or variables not initialized properly.
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setState(IBStream* fileStream)
{
	IBStreamer s(fileStream, kLittleEndian);
	uint64 version = 0;
	double data = 0;

	// --- read the version
	if(!s.readInt64u(version)) return kResultFalse;

    // --- serialize
	for (unsigned int i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);
        
        if(piParam)
        {
            if(!s.readDouble(data))
                return kResultFalse;
            else
            {
                // --- temp disable smoothing
                bool smooth = piParam->getParameterSmoothing();
                piParam->setParameterSmoothing(false);

                // --- init actual, no smooth
                piParam->setControlValue(data);
                
                // --- reset
                piParam->setParameterSmoothing(smooth);
            }
        }
    }
    
    // --- add plugin side bypassing
    if(!s.readBool(plugInSideBypass)) return kResultFalse;
    
    // --- set the bypass state
    setParamNormalized (PLUGIN_SIDE_BYPASS, plugInSideBypass);

    // --- do next version...
    if(version >= 1)
    {
        // --- for future versioning
    }

    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getState
//
/**
\brief This is the WRITE part of the serialization process. We get the stream interface and use it
	   to write to the filestream. This is important because it is how the Factory Default is set
	   at startup, as well as when writing presets.

NOTES:
- The datatypes/read order must EXACTLY match the getState() version or crashes may happen or variables not initialized properly.
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getState(IBStream* fileStream)
{
	// --- get a stream I/F
	IBStreamer s(fileStream, kLittleEndian);

	// --- VSTPluginVersion - see top of this file; versioning can be used during the READ operation for updating
    //     your plugin without breaking older version saved states
	if(!s.writeInt64u(VSTPluginVersion)) return kResultFalse;

	// --- write out all of the params
    // --- serialize
	for (unsigned int i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);
        
        if(piParam)
        {
           if(!s.writeDouble(piParam->getControlValue()))
               return kResultFalse;
        }
    }

	// --- add plugin side bypassing
	if(!s.writeBool(plugInSideBypass)) return kResultFalse;

	// --- v1: for future use
	//

    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::doControlUpdate
//
/**
\brief Find and issue Control Changes

\return true if a control was changed
	
NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool VST3Plugin::doControlUpdate(ProcessData& data)
{
	bool paramChange = false;

	// --- check
    IParameterChanges* paramChanges = data.inputParameterChanges;
	if(!paramChanges)
		return paramChange;
    
	// --- get the param count and setup a loop for processing queue data
    int32 count = paramChanges->getParameterCount();

	// --- make sure there is something there
	if(count <= 0)
		return paramChange;

	// --- loop
	for(int32 i=0; i<count; i++)
	{
		// get the message queue for ith parameter
        IParamValueQueue* queue = paramChanges->getParameterData(i);

		if(queue)
		{
			// --- check for control points
			if(queue->getPointCount() <= 0) return false;

			int32 sampleOffset = 0.0;
			ParamValue value = 0.0;
			ParamID pid = queue->getParameterId();

			// these only are updated if a change has occurred (a control got moved)
			//
            // --- get the last point in queue, if user is not using the sample accurate queue
			if(queue->getPoint(queue->getPointCount() - 1, /* last update point */
				sampleOffset,			/* sample offset */
				value) == kResultTrue)	/* value = [0..1] */
			{
				// --- at least one param changed
				paramChange = true;
 
                PluginParameter* piParam = pluginCore->getPluginParameterByControlID(pid);
                
                if(piParam)
                {
                    // --- add the sample accurate queue
                    if (enableSAAVST3)
                    {
                        m_pParamUpdateQueueArray[i]->setParamValueQueue(queue, data.numSamples);
                        piParam->setParameterUpdateQueue(m_pParamUpdateQueueArray[i]);
                    }
                    else
                        piParam->setControlValueNormalized(value, true); // false = do not apply taper
                }
				else if(pid == PLUGIN_SIDE_BYPASS) // want 0 to 1
				{
                    plugInSideBypass = value > 0.5f ? true : false;
                    break;
				}
			}
		}
	}

	return paramChange;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::updateHostInfo
//
/**
\brief update the incoming host data for the plugin core

\param data process information from the host
\param hostInfo HostInfo struct to fill out as the return

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void VST3Plugin::updateHostInfo(ProcessData& data, HostInfo* hostInfo)
{
    if(!pluginCore) return;
    
    double dSampleInterval = 1.0 / pluginCore->getSampleRate();
    
    if (data.processContext) // NOTE: data.processContext is NULL during the VSTValidator process Test part I
    {
        hostInfo->dBPM = data.processContext->tempo;
        hostInfo->fTimeSigNumerator = data.processContext->timeSigNumerator;
        hostInfo->uTimeSigDenomintor = (unsigned int)data.processContext->timeSigDenominator;
        hostInfo->uAbsoluteFrameBufferIndex = (unsigned int)data.processContext->projectTimeSamples;
        hostInfo->dAbsoluteFrameBufferTime = (double)hostInfo->uAbsoluteFrameBufferIndex*dSampleInterval;
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::process
//
/**
\brief the VST3 audio processing function

\param data process information from the host

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::process(ProcessData& data)
{
    // --- check for control chages and update if needed
    //     Changed for 3.6.14: this is moved to top of function for bypass persistence
    //     during testing
    doControlUpdate(data);

	if (!pluginCore) return kResultFalse;
    
    // --- handle synth plugins differently - no input buss
    bool isSynth = pluginCore->getPluginType() == kSynthPlugin;
    
    // --- check for validator tests with bad input or output buffer
    if (isSynth && !data.outputs)
        return kResultTrue;
    else if (pluginCore->getPluginType() == kFXPlugin && (!data.inputs || !data.outputs))
        return kResultTrue;

    // --- setup buffer processing
    ProcessBufferInfo info;
 
    info.inputs = isSynth ? nullptr : &data.inputs[0].channelBuffers32[0];
    info.outputs = &data.outputs[0].channelBuffers32[0];
    
    // --- setup channel formats
    SpeakerArrangement inputArr;
    SpeakerArrangement outputArr;
    if(isSynth)
        info.numAudioInChannels = 0;
    else
    {
        getBusArrangement(kInput, 0, inputArr);
        info.numAudioInChannels = SpeakerArr::getChannelCount(inputArr);
    }
    getBusArrangement(kOutput, 0, outputArr);
    info.numAudioOutChannels = SpeakerArr::getChannelCount(outputArr);
    
    // --- setup the channel configs 
    if(isSynth) info.channelIOConfig.inputChannelFormat = kCFNone;
    else info.channelIOConfig.inputChannelFormat = getChannelFormatForSpkrArrangement(inputArr);
    
    // --- output (always)
    info.channelIOConfig.outputChannelFormat = getChannelFormatForSpkrArrangement(outputArr);

    // --- soft bypass for FX plugins
    if (plugInSideBypass && !isSynth)
    {
        for (int32 sample = 0; sample < data.numSamples; sample++)
        {
            // --- output = input
			for (unsigned int i = 0; i<info.numAuxAudioOutChannels; i++)
            {
                (data.outputs[0].channelBuffers32[i])[sample] = (data.inputs[0].channelBuffers32[i])[sample];
            }
        }

        // --- update the meters, force OFF
        updateMeters(data, true);
        
        return kResultTrue;
    }
    
    // --- set sidechain info
    if(hasSidechain)
    {
        BusList* busList = getBusList(kAudio, kInput);
        Bus* bus = busList ? (Bus*)busList->at(1) : 0;
        if (bus && bus->isActive())
        {
            info.numAuxAudioInChannels = data.inputs[1].numChannels;
            info.auxInputs = &data.inputs[1].channelBuffers32[0]; //** to sidechain
        }
    }
    
    info.auxOutputs = nullptr; // --- for future use
    info.numAuxAudioOutChannels = 0; // --- for future use

    info.numFramesToProcess = data.numSamples;
    
    // --- sidechain channel config
    info.auxChannelIOConfig.inputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(info.numAuxAudioInChannels);
    info.auxChannelIOConfig.outputChannelFormat = pluginCore->getDefaultChannelIOConfigForChannelCount(0);
    
    // --- MIDI
    IEventList* inputEvents = data.inputEvents;
    midiEventQueue->setEventList(inputEvents);
    info.midiEventQueue = midiEventQueue;
    
    // --- get host info
    HostInfo hostInfo;
    updateHostInfo(data, &hostInfo);
    // --- sample accurate automation is only available for VST3 hosts, and supercedes normal smoothing
    hostInfo.enableVSTSampleAccurateAutomation = pluginCore->wantsVST3SampleAccurateAutomation();
    info.hostInfo = &hostInfo;
    
    // --- process the buffers
    pluginCore->processAudioBuffers(info);
   
    // --- update the meters
    updateMeters(data);
 
    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::updateMeters
//
/**
\brief update the outbound VST3 parameters that correspond to plugin meter variables

\param data process information from the host
\param forceOff turn off meters regawrdless of audio input values

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void VST3Plugin::updateMeters(ProcessData& data, bool forceOff)
{
    if(!pluginCore) return;
    
    if(data.outputParameterChanges)
    {
        int32_t startIndex = 0;
        int32_t indexer = 0;
        while(startIndex >= 0)
        {
            PluginParameter* piParam = pluginCore->getNextParameterOfType(startIndex, controlVariableType::kMeter);
            if(piParam)
            {
                int32 queueIndex = 0;
                IParamValueQueue* queue = data.outputParameterChanges->addParameterData(piParam->getControlID(), queueIndex);
                
                if(queue)
                {
                    double meterValue = forceOff ? 0.0 : piParam->getControlValue();
                    queue->addPoint(indexer++, meterValue, queueIndex);
                }
            }
        }
    }
}
    

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getMidiControllerAssignment
//
/**
\brief The client queries this 129 times for 130 possible control messages, see ivstsmidicontrollers.h for
	   the VST defines for kPitchBend, kCtrlModWheel, etc... for each MIDI Channel in our Event Bus

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getMidiControllerAssignment(int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id/*out*/)
{
    // --- add for synth plugin book
	if(!pluginCore) return kResultFalse;

	// NOTE: we only have one EventBus(0)
	//       but it has 16 channels on it
    id = -1;
	if(busIndex == 0)
	{
        // --- decode the channel and controller
		switch(midiControllerNumber)
		{
			case kPitchBend:
   			case kCtrlModWheel:
			case kCtrlVolume:
			case kCtrlPan:
			case kCtrlExpression:
			case kAfterTouch:
			case kCtrlSustainOnOff:
			case kCtrlAllNotesOff:
                break;
		}

		if(id == -1)
		{
            id = 0;
			return kResultFalse;
		}
		else
			return kResultTrue;
	}

	return kResultFalse;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::createView
//
/**
\brief creates the custom GUI view

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IPlugView* PLUGIN_API VST3Plugin::createView(const char* _name)
{
    if(!pluginCore) return nullptr;

	ConstString name(_name);
	if(name == ViewType::kEditor)
	{
        PluginEditor* pluginEditor = nullptr;

#if MAC
		 // --- Look for a resource in the main bundle by name and type.
		 //     NOTE: this is set in info.plist
		CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFStringCreateWithCString(nullptr, pluginCore->getVST3BundleID(), kCFStringEncodingASCII));
		if (bundle == nullptr) return nullptr;

		// --- get .uidesc file
		CFURLRef bundleURL = CFBundleCopyResourceURL(bundle, CFSTR("PluginGUI"), CFSTR("uidesc"), nullptr);
		CFStringRef xmlPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
		int nSize = CFStringGetLength(xmlPath);
		char* path = new char[nSize + 1];
		memset(path, 0, (nSize + 1) * sizeof(char));
		CFStringGetFileSystemRepresentation(xmlPath, path, nSize + 1);
		CFRelease(xmlPath);

		// --- create GUI
		pluginEditor = new PluginEditor(path, pluginCore, guiPluginConnector, pluginHostConnector, this);
		delete[] path;
#else
		 // --- create GUI
		pluginEditor = new PluginEditor("PluginGUI.uidesc", pluginCore, guiPluginConnector, pluginHostConnector, this);
#endif

        return pluginEditor;
    }
    return nullptr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::receiveText
//
/**
\brief VST3 messaging system - not used in ASPiK but here if you want to play with messaging.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult VST3Plugin::receiveText(const char8* text)
{
	if(strcmp(text, "VSTGUITimerPing") == 0)
	{
        ; // timer notify
    }
	if(strcmp(text, "RecreateView") == 0)
	{
        ; // recreation notify
	}

	return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setComponentState
//
/**
\brief This is the serialization-read function so the GUI can be updated from a preset or startup 

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setComponentState(IBStream* fileStream)
{
    IBStreamer s(fileStream, kLittleEndian);
    uint64 version = 0;
    double data = 0;
    
    // --- read the version
    if(!s.readInt64u(version)) return kResultFalse;
    
    // --- serialize
	for (unsigned int i = 0; i < pluginCore->getPluginParameterCount(); i++)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByIndex(i);
        
        if(piParam)
        {
            if(!s.readDouble(data))
                return kResultFalse;
            else
                setParamNormalizedFromFile(piParam->getControlID(), data);
        }
    }
    
    // --- add plugin side bypassing
    if(!s.readBool(plugInSideBypass)) return kResultFalse;
    
    // --- do next version...
    if(version >= 1)
    {
        // --- for future versioning
    }

	return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setParamNormalizedFromFile
//
/**
\brief helper function for setComponentState()

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setParamNormalizedFromFile(ParamID tag, ParamValue value)
{
    // --- get the parameter
    Parameter* pParam = SingleComponentEffect::getParameterObject(tag);
    
    // --- verify pointer
    if(!pParam)
        return kResultFalse;
    
    // --- sync parameter
    //     this will call the update handler to modify the GUI controls (safe)
    tresult res = SingleComponentEffect::setParamNormalized(tag, pParam->toNormalized(value));
 
    return res;
}
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setParamNormalized
//
/**
\brief This is overridden for selecting a preset, this is also called when automating parameters

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::setParamNormalized(ParamID tag, ParamValue value)
{
    //tresult res = SingleComponentEffect::setParamNormalized(tag, value); //kResultFalse;
    tresult res = kResultFalse;

	// --- handle preset changes
	if(tag == kPresetParam)
	{
		int32 program = parameters.getParameter(tag)->toPlain(value);

        PresetInfo* preset = pluginCore->getPreset(program);
        if(preset)
        {
			for (unsigned int j = 0; j<preset->presetParameters.size(); j++)
            {
                PresetParameter preParam = preset->presetParameters[j];
                
                if(pluginCore)
                    pluginCore->setPIParamValue(preParam.controlID, preParam.actualValue);

                ParamValue normalizedValue = plainParamToNormalized(preParam.controlID, preParam.actualValue);
               
                // --- sync parameter
                //     this will call the update handler to modify the GUI controls (safe)
                res = SingleComponentEffect::setParamNormalized(preParam.controlID, normalizedValue);
            }
        }
        return res;
	}
    
    // --- sync parameter
    //     this will call the update handler to modify the GUI controls (safe)
    res = SingleComponentEffect::setParamNormalized(tag, value);
    
    // --- sync to pluginCore
    if(pluginCore)
    {
        ParamValue actualValue = normalizedParamToPlain(tag, value);
        pluginCore->setPIParamValue(tag, actualValue);
    }
    
 	return res;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::addProgramList
//
/**
\brief part of the IUnitInfo support for presets; generally no user editable code here.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool VST3Plugin::addProgramList(ProgramList* list)
{
    programIndexMap[list->getID ()] = programLists.size ();
    programLists.emplace_back (list, false);
    list->addDependent (this);
    return true;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramList
//
/**
\brief part of the IUnitInfo support for presets; generally no user editable code here.

NOTES:
- selects a program list (aka preset list)
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ProgramList* VST3Plugin::getProgramList(ProgramListID listId) const
{
    auto it = programIndexMap.find (listId);
    return it == programIndexMap.end () ? nullptr : programLists[it->second];
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::notifyPogramListChange
//
/**
\brief If list changes; should not be called as we only have one program list

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult VST3Plugin::notifyPogramListChange (ProgramListID listId, int32 programIndex)
{
  	tresult result = kResultFalse;
	FUnknownPtr<IUnitHandler> unitHandler(componentHandler);
	if (unitHandler)
		result = unitHandler->notifyProgramListChange (listId, programIndex);
	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramListCount
//
/**
\brief We have one list for our one set of presets

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int32 PLUGIN_API VST3Plugin::getProgramListCount ()
{
	if (parameters.getParameter(kPresetParam))
		return 1;
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramListInfo
//
/**
\brief Get information about our preset list.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getProgramListInfo(int32 listIndex, ProgramListInfo& info /*out*/)
{
	Parameter* param = parameters.getParameter(kPresetParam);
	if(param && listIndex == 0)
	{
		info.id = kPresetParam;
		info.programCount = (int32)param->toPlain (1) + 1;
		UString name (info.name, 128);
		name.fromAscii("Presets");
		return kResultTrue;
	}
	return kResultFalse;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramName
//
/**
\brief Get preset name

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getProgramName(ProgramListID listId, int32 programIndex, String128 name /*out*/)
{
	if(listId == kPresetParam)
	{
		Parameter* param = parameters.getParameter(kPresetParam);
		if (param)
		{
			ParamValue normalized = param->toNormalized (programIndex);
			param->toString (normalized, name);
			return kResultTrue;
		}
	}
	return kResultFalse;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setProgramName
//
/**
\brief Set preset name

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult VST3Plugin::setProgramName(ProgramListID listId, int32 programIndex, const String128 name /*in*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->setProgramName(programIndex, name);
	}
	return kResultFalse;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramInfo
//
/**
\brief Only used for presets.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getProgramInfo(ProgramListID listId, int32 programIndex, CString attributeId /*in*/, String128 attributeValue /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->getProgramInfo(programIndex, attributeId, attributeValue);
	}
	return kResultFalse;

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::hasProgramPitchNames
//
/**
\brief Not Used.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::hasProgramPitchNames(ProgramListID listId, int32 programIndex)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->hasPitchNames(programIndex);
	}
	return kResultFalse;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getProgramPitchName
//
/**
\brief Not Used.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API VST3Plugin::getProgramPitchName (ProgramListID listId, int32 programIndex, int16 midiPitch, String128 name /*out*/)
{
	ProgramIndexMap::const_iterator it = programIndexMap.find(listId);
	if (it != programIndexMap.end())
	{
		return programLists[it->second]->getPitchName(programIndex, midiPitch, name);
	}
	return kResultFalse;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::update
//
/**
\brief Toggle preset.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PLUGIN_API VST3Plugin::update(FUnknown* changedUnknown, int32 message)
{
	ProgramList* programList = FCast<ProgramList> (changedUnknown);
	if(programList)
	{
		FUnknownPtr<IUnitHandler> unitHandler(componentHandler);
		if (unitHandler)
			unitHandler->notifyProgramListChange(programList->getID (), kAllProgramInvalid);
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::addUnit
//
/**
\brief IUnitInfo handler.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//bool VST3Plugin::addUnit(Unit* unit)
//{
//	units.emplace_back(IPtr<Unit>(unit, false));
//	return true;
//}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getFUID
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FUID* VST3Plugin::getFUID()
{
	if (VST3PluginCID) delete VST3PluginCID;

    VST3PluginCID = new FUID;
    VST3PluginCID->fromRegistryString(PluginCore::getVSTFUID());
    return VST3PluginCID;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getPluginName
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char* VST3Plugin::getPluginName()
{
    return PluginCore::getPluginName();
}
  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getVendorName
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char* VST3Plugin::getVendorName()
{
    return PluginCore::getVendorName();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getVendorURL
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char* VST3Plugin::getVendorURL()
{
    return PluginCore::getVendorURL();
}
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getVendorEmail
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char* VST3Plugin::getVendorEmail()
{
    return PluginCore::getVendorEmail();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getPluginType
//
/**
\brief static function for class facgtory access to plugin core's corresponding static function

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CString VST3Plugin::getPluginType()
{
    if(PluginCore::getPluginType() == pluginType::kFXPlugin)
        return Vst::PlugType::kFx;
    else if(PluginCore::getPluginType() == pluginType::kSynthPlugin)
        return Vst::PlugType::kInstrumentSynth;
    
    return Vst::PlugType::kFx;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::VSTParamUpdateQueue
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VSTParamUpdateQueue::VSTParamUpdateQueue(void)
{
	queueIndex = 0;
	queueSize = 0;
	bufferSize = 0;
	parameterQueue = nullptr;
	sampleAccuracy = nullptr;
	x1 = 0.0;
	y1 = 0.0;
	x2 = 0.0;
	y2 = 0.0;
	slope = 1.0;
	dirtyBit = false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::initialize
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void VSTParamUpdateQueue::initialize(ParamValue _initialValue, ParamValue _minValue, ParamValue _maxValue, unsigned int* _sampleAccuracy)
{
	bufferSize = 0;
	initialValue = 0.0;
	previousValue = 0.0;
	minValue = _minValue;
	maxValue = _maxValue;
	sampleAccuracy = _sampleAccuracy;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setParamValueQueue
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void VSTParamUpdateQueue::setParamValueQueue(IParamValueQueue* _paramValueQueue, unsigned int _bufferSize)
{
	bufferSize = _bufferSize;
	parameterQueue = _paramValueQueue;
	queueSize = parameterQueue->getPointCount();
	queueIndex = 0;
	sampleOffset = 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::setSlope
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void VSTParamUpdateQueue::setSlope()
{
	int32 sampleOffset = 0;
	ParamValue value = 0.0;

	if (parameterQueue->getPoint(queueIndex, sampleOffset, value) == Steinberg::kResultTrue)
	{
		if (queueIndex == 0 && sampleOffset == 0)
		{
			x1 = sampleOffset;
			y1 = value;

			if (parameterQueue->getPointCount()<2)
			{
				x2 = bufferSize;
				y2 = value;
			}
			else
			{
				queueIndex++;
				int32 sampleOffset2 = 0.0;
				ParamValue value2 = 0.0;
				if (parameterQueue->getPoint(queueIndex, sampleOffset2, value2) == Steinberg::kResultTrue)
				{
					x2 = sampleOffset2;
					y2 = value2;
				}
			}
		}
		else
		{
			if (dirtyBit == true)
			{
				x1 = 0;

				dirtyBit = false;
			}
			else
			{
				x1 = x2;

			}
			y1 = y2;
			x2 = sampleOffset;
			y2 = value;
		}
		if (x2 == bufferSize - 1)
		{
			dirtyBit = true;
		}
		slope = (y2 - y1) / (float)(x2 - x1);
		yIntercept = y1 - (slope * x1);

		queueIndex++;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::interpolate
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ParamValue VSTParamUpdateQueue::interpolate(int x1, int x2, ParamValue y1, ParamValue y2, int x)
{
	if (x == x1)
		return y1;
	if (x == x2)
		return y2;

	return (slope * x) + yIntercept;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::needsUpdate
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int VSTParamUpdateQueue::needsUpdate(int x, ParamValue &value)
{
	int nSampleGranularity = *sampleAccuracy;

	ParamValue  newValue = interpolate(x1, x2, y1, y2, x);

	if (x == 0 || x == x2)
		setSlope();
	
    // return 0 if slope is 0
	if (slope == 0.0)
		return 0;

	if (nSampleGranularity == 0)
	{
		if (x != x1&&x != x2)
			return 0;
	}
	else if (nSampleGranularity != 0)
	{
		if (x%nSampleGranularity != 0 && x != x1&&x != x2)
			return 0;
	}

	if (newValue == previousValue)
		return 2;
	else
	{
		value = newValue;
		previousValue = newValue;
		return 1;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getParameterIndex
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
unsigned int VSTParamUpdateQueue::getParameterIndex()
{
	return parameterQueue->getParameterId();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getValueAtOffset
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool VSTParamUpdateQueue::getValueAtOffset(long int _sampleOffset, double _previousValue, double& _nextValue)
{
	sampleOffset = _sampleOffset;
	ParamValue value = 0;
	int nRC = needsUpdate(_sampleOffset, value);
	if (nRC == 1)
	{
		_nextValue = value;
		return true;
	}

	return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	VST3Plugin::getNextValue
//
/**
\brief ASPiK support for sample accurate auatomation

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool VSTParamUpdateQueue::getNextValue(double& _nextValue)
{
	ParamValue value = 0;
	int nRC = needsUpdate(sampleOffset++, value);
	if (nRC == 1)
	{
		_nextValue = value;
		return true;
	}

	return false;
}
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::PluginEditor
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PluginEditor::PluginEditor(VSTGUI::UTF8StringPtr _xmlFile, PluginCore* _pluginCore, GUIPluginConnector* _guiPluginConnector, PluginHostConnector* _pluginHostConnector, VST3Plugin* _editController)
: CPluginView()
, PluginGUI(_xmlFile)
, pluginCore(_pluginCore)
, guiPluginConnector(_guiPluginConnector)
, pluginHostConnector(_pluginHostConnector)
, editController(_editController)
{

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::~PluginEditor
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PluginEditor::~PluginEditor ()
{
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::setFrame
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::setFrame(IPlugFrame* frame)
{
    plugFrame = frame;
    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::attached
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::attached(void* parent, FIDString type)
{
#if MAC
    if (isPlatformTypeSupported(type) != kResultTrue)
        return kResultFalse;
    
#if MAC_COCOA && MAC_CARBON && !(VSTGUI_VERSION_MAJOR >= 4 && VSTGUI_VERSION_MINOR >= 1)
    CFrame::setCocoaMode(strcmp(type, kPlatformTypeNSView) == 0);
#endif
#endif
    
    VSTGUI::PlatformType platformType = VSTGUI::kDefaultNative;
    
#if MAC
#if TARGET_OS_IPHONE
    if (strcmp(type, kPlatformTypeUIView) == 0)
        platformType = kUIView;
#else
#if MAC_CARBON
    if (strcmp(type, kPlatformTypeHIView) == 0)
        platformType = kWindowRef;
#endif
    
#if MAC_COCOA
    if (strcmp(type, kPlatformTypeNSView) == 0)
        platformType = VSTGUI::kNSView;
#endif
#endif
#endif
    
    // --- open the gui here
	// --- set up the GUI parameter copy list
	std::vector<PluginParameter*>* PluginParameterPtr = pluginCore->makePluginParameterVectorCopy();

	// --- create GUI
	bool opened = open("Editor", parent, PluginParameterPtr, platformType, guiPluginConnector, nullptr);

    // --- setup our map of VST3UpdateHandler objects for (LEGAL) remote GUI sync
	if (opened)
	{
		if (editController)
        {
            int32 count = editController->getParameterCount();
            for(int i=0; i<count; i++)
            {
                ParameterInfo info;
                tresult res = editController->getParameterInfo (i, info);
                if(res == kResultTrue)
                {
                    VSTGUI::ControlUpdateReceiver* receiver = this->getControlUpdateReceiver(info.id);
                    if(receiver)
                    {
                        Parameter* param = editController->getParameterObject(info.id);
                        if(param)
                        {
                            // --- we have a parameter and control receiver pair!
                            VST3UpdateHandler* vst3Updater = new VST3UpdateHandler(receiver, editController);
                            updateHandlers.insert(std::make_pair(info.id, vst3Updater));
                            param->addRef();
                            param->addDependent(vst3Updater);
                        }
                    }
                }

            
            }
         }

		setGUIWindowFrame((IGUIWindowFrame*)this);

		if (true)
		{
			ViewRect vr(0, 0, (int32)frame->getWidth(), (int32)frame->getHeight());
			setRect(vr);
			if (plugFrame)
				plugFrame->resizeView(this, &vr);
		}
	}

	// --- delete the PluginParameterPtr guts, and pointer too...
	for (std::vector<PluginParameter*>::iterator it = PluginParameterPtr->begin(); it != PluginParameterPtr->end(); ++it)
	{
		delete *it;
	}
	delete PluginParameterPtr;

    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::removed
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::removed()
{
    // --- close the GUI here
    // --- remove dependents
    if (editController)
    {
        int32 count = editController->getParameterCount();
        for(int i=0; i<count; i++)
        {
            ParameterInfo info;
            tresult res = editController->getParameterInfo (i, info);
            if(res == kResultTrue)
            {
                VST3UpdateHandler* vst3Updater = updateHandlers[info.id];
                if(vst3Updater)
                {
                    Parameter* param = editController->getParameterObject(info.id);
                    if(param)
                    {
                        param->removeDependent(vst3Updater);
                        param->release();
                    }
                    
                    updateHandlers.erase(info.id);
                }
            }
        }
    }

    close();
    
    return kResultTrue;
}
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::isPlatformTypeSupported
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::isPlatformTypeSupported(FIDString type)
{
#if WINDOWS
    if (strcmp(type, kPlatformTypeHWND) == 0)
        return kResultTrue;
    
#elif MAC
#if TARGET_OS_IPHONE
    if (strcmp(type, kPlatformTypeUIView) == 0)
        return kResultTrue;
#else
#if MAC_CARBON
    if (strcmp(type, kPlatformTypeHIView) == 0)
        return kResultTrue;
#endif
    
#if MAC_COCOA
    if (strcmp(type, kPlatformTypeNSView) == 0)
        return kResultTrue;
#endif
#endif
#endif
    
    return kInvalidArgument;
}
    
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::onSize
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::onSize(ViewRect* newSize)
{
    if (frame)
        frame->setSize(newSize->right - newSize->left, newSize->bottom - newSize->top);
    
    if (newSize)
        rect = *newSize;
    
    return kResultTrue;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PluginEditor::getSize
//
/**
\brief ASPiK support VST3 GUI - this wraps the ASPiK GUI so that it conforms to the IPlugView interface.

NOTES:
- see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and a VST3 Programming Guide
- see VST3 SDK Documentation for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tresult PLUGIN_API PluginEditor::getSize(ViewRect* size)
{
    if (size)
    {
        *size = rect;
        return kResultTrue;
    }
    return kInvalidArgument;
}
    
    
}}} // namespaces





