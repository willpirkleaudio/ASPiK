// -----------------------------------------------------------------------------
//    ASPiK AU Shell File:  ausynthplugin.h
//
/**
    \file   ausynthplugin.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for the AUSynthPlugin which is the ASPiK AU Shell object

			- included in ASPiK(TM) AU Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AU Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include <AudioToolbox/AudioUnitUtilities.h>
#include "AUInstrumentBase.h"   // for Synths
#include "plugingui.h"
#include "plugincore.h"
#include <math.h>
#include <queue>
#include <string>

/**
\enum guiMessage
\ingroup AU-Shell
\brief
Use this enum to send custom messages from the GUI to the AU plugin. This is the VSTGUI-approved mechanism.

enum guiMessage { kOpenGUI = 64000, kCloseGUI };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/

// custom properties id's must be 64000 or greater
// see <AudioUnit/AudioUnitProperties.h> for a list of Apple-defined standard properties
//
// --- These are for VSTGUI4 messaging - WP
enum guiMessage
{
	kOpenGUI = 64000,
    kCloseGUI
};

class GUIPluginConnector;
class PluginHostConnector;
class AUMIDIEventQueue;

/**
\class AUSynthPlugin
\ingroup AU-Shell
\brief
The AUSynthPlugin is the ASPiK plugin shell for Audio Units synth plugins. It contains the plugin kernel and all necessary intefaces, implemented as separate C++ obejcts.

NOTES:
- derived from AUMIDIEffectBase to allow MIDI input/output if desired; it is not necessary to use MIDI but it is bSCAvailable
- however, AUMIDIEffectBase does not allow exposing a sidechain so we get around this issue by setting the AU effet type to "aufx" even though it is really "aumu" -- this will only generate a warning during validation but is otherwise secure.
- the required interfaces are implemented on member objects that handle the thread-safe transfer of parameter information

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class AUSynthPlugin : public AUInstrumentBase
{
public:
    AUSynthPlugin(AudioUnit component);
    ~AUSynthPlugin();

    /** AU override method */
    virtual ComponentResult	Version() {return 1000;}

    /** AU override method */
    virtual ComponentResult	Initialize();

    /** AU override method */
    virtual OSStatus GetPropertyInfo(AudioUnitPropertyID	inID,
                                     AudioUnitScope         nScope,
                                     AudioUnitElement       inElement,
                                     UInt32&                outDataSize,
                                     Boolean&               outWritable );

    /** AU override method */
    virtual OSStatus GetProperty(AudioUnitPropertyID    inID,
                                 AudioUnitScope 		inScope,
                                 AudioUnitElement       inElement,
                                 void*                  outData );

    /** AU override method */
    virtual OSStatus SetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope 	 inScope,
                                 AudioUnitElement 	 inElement,
                                 const void*		 inData,
                                 UInt32 			 inDataSize);

    /** AU override method */
    virtual ComponentResult	GetParameterInfo(AudioUnitScope			inScope,
                                             AudioUnitParameterID	inParameterID,
                                             AudioUnitParameterInfo	&outParameterInfo );

    /** AU override method */
    virtual ComponentResult	GetPresets(CFArrayRef* outData)	const;

    /** AU override method */
    virtual OSStatus NewFactoryPresetSet (const AUPreset& inNewFactoryPreset);

    /** AU override method reply to host query

    \return TRUE if plugin core wants a tail
    */
    virtual	bool SupportsTail()
    {
        if(pluginCore)
            return pluginCore->getTailTimeInMSec() > 0 ? true : false;

        return false;
    }

    virtual Float64	GetTailTime()
    {
        if(pluginCore)
            return pluginCore->getTailTimeInMSec() / 1000.0;

        return 0.0;
    }

    /** AU override method */
    virtual Float64	GetLatency() {return latencyInSeconds;}


    /** AU override method */
    virtual ComponentResult SetParameter(AudioUnitParameterID	 inID,
                                         AudioUnitScope 		 inScope,
                                         AudioUnitElement 		 inElement,
                                         AudioUnitParameterValue inValue,
                                         UInt32					 inBufferOffsetInFrames);

    /** AU override method */
    virtual OSStatus Render(AudioUnitRenderActionFlags &		ioActionFlags,
                            const AudioTimeStamp &              inTimeStamp,
                            UInt32                              inNumberFrames);


    /** AU override method */
    virtual ComponentResult	Reset(AudioUnitScope   inScope,
                                  AudioUnitElement inElement);

    /** AU override method */
    virtual ComponentResult	GetParameterValueStrings(AudioUnitScope		  inScope,
                                                     AudioUnitParameterID inParameterID,
                                                     CFArrayRef*		  outStrings);

    // --- need this for when user selects a NON factory-preset (ie they created the preset in the Client)
    /** AU override method */
    virtual ComponentResult	RestoreState(CFPropertyListRef inData);
    /** AU override method */
    virtual UInt32 SupportedNumChannels(const AUChannelInfo** outInfo);

    // --- MIDI Functions
    /** AU override method */
    virtual OSStatus HandleNoteOn(UInt8 	inChannel,
                                  UInt8 	inNoteNumber,
                                  UInt8 	inVelocity,
                                  UInt32    inStartFrame);

    /** AU override method */
    virtual OSStatus HandleNoteOff(UInt8 	inChannel,
                                   UInt8 	inNoteNumber,
                                   UInt8 	inVelocity,
                                   UInt32   inStartFrame);

    // --- MIDI Pitchbend (slightly different from all other CCs)
    /** AU override method */
    virtual OSStatus HandlePitchWheel(UInt8  inChannel,
                                      UInt8  inPitch1,
                                      UInt8  inPitch2,
                                      UInt32 inStartFrame);

    // --- all other MIDI CC messages
    /** AU override method */
    virtual OSStatus HandleControlChange(UInt8  inChannel,
                                         UInt8  inController,
                                         UInt8  inValue,
                                         UInt32	inStartFrame);

    // --- for ALL other MIDI messages you can get them here
    /** AU override method */
    virtual OSStatus HandleMidiEvent(UInt8  status,
                             UInt8  channel,
                             UInt8  data1,
                             UInt8  data2,
                             UInt32 inStartFrame);

    /**
     \brief helper function to get a path to the location where THIS library is loaded

     \param bundleID the bundle ID as a string

     \return the path as a simple, naked char*
     */
    char* getMyComponentDirectory(CFStringRef bundleID)
    {
        if (bundleID != NULL)
        {
            CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
            if(helixBundle != NULL)
            {
                CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
                if(bundleURL != NULL)
                {
                    CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);

                    CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                    CFRelease(componentFolderPathURL);

                    if(myComponentPath != NULL)
                    {
                        int nSize = CFStringGetLength(myComponentPath);
                        char* path = new char[nSize+1];
                        memset(path, 0, (nSize+1)*sizeof(char));

                        bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                        CFRelease(myComponentPath);

                        if(success) return path;
                        else return NULL;
                    }
                    CFRelease(bundleURL);
                }
            }
            CFRelease(bundleID);
        }
        return NULL;
    }

     /**
     \brief safely issue a parameter change event

     \param controlID the AU parameter identifier (same as PluginParameter's controlID)
     \param actualValue the value to set
     */
    void setAUParameterChangeEvent(unsigned int controlID, double actualValue);

    /**
     \brief safely get a parameter value

     \param controlID the AU parameter identifier (same as PluginParameter's controlID)

     \return the parameter's actualValue
     */
    double getAUParameter(unsigned int controlID);

    GUIPluginConnector* guiPluginConnector = nullptr;       ///< GUI -> Plugin interface
    PluginHostConnector* pluginHostConnector = nullptr;     ///< Plugin -> Host interface
    AUMIDIEventQueue* midiEventQueue = nullptr;             ///< double-buffered-queue for MIDI messaging

protected:
    // --- Plugin Core
    PluginCore* pluginCore = nullptr;           ///< GUI the plugin core: alive for FULL lifecycle of shell
    void initAUParametersWithPluginCore();      ///< setup the AU parameter list with the plugin core's parameter list
    void updateAUParametersWithPluginCore();    ///< send parameter update info (metering, output)
    void updatePluginCoreParameters();          ///< set the plugin core parameters from the AU parameters (called during each buffer process cycle)
    void updateHostInfo(HostInfo* hostInfo);    ///< set the HostInfo for the core (varies by API)

    // --- sidechaining
    bool hasSidechain = false;                          ///< sidechain flag
    AudioBufferList* sidechainBufferList = nullptr;     ///< sidechain buffers (if active)
    int sidechainChannelCount = 0;                      ///< num sidechain channels
    AUChannelInfo* auChannelInfo = nullptr;             ///< the current channel information
    float** inputBuffers = nullptr;                     ///< de-interleaved incoming audio input buffers
    float** outputBuffers = nullptr;                    ///< de-interleaved outgoing audio output buffers
    float** sidechainInputBuffers = nullptr;            ///< de-interleaved incoming audio sidechain buffers

    // --- raw bytes for "static" preset data
    void* presetsArrayData = nullptr;                   ///< contiguous memory block for persistent preset data
    int currentPreset = 0;                              ///< current preset's index value

    // --- NOTE: AU takes latency in seconds, not samples; this is recalculated
    //           during init() and reset() operations
    Float64 latencyInSeconds = 0 ;                      ///< au latency (seconds!)


    ///< VSTGUI4 Editor NOTE: this is only used to open and close the GUI
    //                          It should NEVER be used to try to communicate
    //                          directly with the GUI - not thread safe
    VSTGUI::PluginGUI* pluginGUI = nullptr;
};

/**
\class PluginHostConnector
\ingroup AU-Shell
\brief
The PluginHostConnector implements the IPluginHostConnector interface for the plugin shell object. For AU, this requires implementing only one method, sendHostMessage( ). Only one message is processed for sendGUIUpdate that provides a mechanism to update the GUI controls from the plugin core. Note that this is not an ideal solution for most problems (e.g. linking controls together intelligently) -- you should always consider using a custom sub-controller and/or custom view to do this properly. On occasion, a more difficult scenario may arise (e.g. MIDI learn button, that must wait for user input from a MIDI instrument to toggle states) where the sendGUIUpdate method may be appropriate. See the example in the ASPiK SDK for more information.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginHostConnector : public IPluginHostConnector
{
public:
    PluginHostConnector(AUSynthPlugin* _auInstance){auInstance = _auInstance;}
    virtual ~PluginHostConnector(){}

    /**
    \brief process a message; by default it processes sendGUIUpdate to safely send a parameter change event but you can add your own custom message processing here (be careful)

    \param hostMessageInfo custom structure with message information; for sendGUIUpdate the message data arrives in the hostMessageInfo.guiUpdateData list; the function iterates over the list and sends AU parameter change events.
    */
    virtual void sendHostMessage(const HostMessageInfo& hostMessageInfo)
    {
        switch(hostMessageInfo.hostMessage)
        {
            case sendGUIUpdate:
            {
                GUIUpdateData guiUpdateData = hostMessageInfo.guiUpdateData;

                for(int i = 0; i < guiUpdateData.guiParameters.size(); i++)
                {
                    GUIParameter guiParam = guiUpdateData.guiParameters[i];

                    // --- threadsafe atomic write to global param & GUI update dispatch
                    auInstance->setAUParameterChangeEvent(guiParam.controlID, guiParam.actualValue);
                }

                // --- clean up
                for(int i = 0; i < guiUpdateData.guiParameters.size(); i++)
                    guiUpdateData.guiParameters.pop_back();

                break;
            }
            default:
                break;
        }
    }

protected:
    AUSynthPlugin* auInstance = nullptr; ///< our plugin object for setAUParameterChangeEvent()
};


/**
 \class CustomViewController
 \ingroup AU-Shell
 \brief
 The CustomViewController is part of the safe ICustomView feature in ASPiK. The CustomViewController maintains an ICustomView pointer. When the GUI registers and de-registers custom views during creation or destuction, the plugin shell is responsible for making sure that the original ICustomView pointer registered with the plugin core object *never goes out of scope* and this object is part of that system.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */// --- GUI -> Plugin interface
//
// --- container for a custom view pointer
class CustomViewController : public ICustomView
{
public:
    /** constructor: pass the newly registered ICustomView* */
    CustomViewController(ICustomView* _customViewIF) { customViewIF = _customViewIF; }
    virtual ~CustomViewController() {}

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void updateView()
    {
        if (customViewIF)
            customViewIF->updateView();
    }

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void pushDataValue(double data)
    {
        if (customViewIF)
            customViewIF->pushDataValue(data);
    }

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void sendMessage(void* data)
    {
        if (customViewIF)
            customViewIF->sendMessage(data);
    }

    /** set a NEW ICustomView* -- this is called when a custom view is registered */
    void setCustomViewPtr(ICustomView* _customViewIF) { customViewIF = _customViewIF; }

    /** get the current ICustomView* -- note this is read-only (const) and it also MAY be NULL */
    const ICustomView* getCustomViewPtr() { return customViewIF; }

    /** clear the stored ICustomView* -- this is called when a custom view is de-registered */
    void clearCustomViewPtr() { customViewIF = nullptr; }


private:
    ICustomView* customViewIF = nullptr; ///< the currently active ICustomView interface, registered from the GUI
};


/**
 \class GUIPluginConnector
 \ingroup AU-Shell
 \brief
 The GUIPluginConnector interface creates a safe message mechanism for the GUI to issue requests to the plugin shell. The following messages are processed via functions:

 - tell core that a parameter has changed (asynchronous)
 - get the AU parameter value (thread-safe via AU instance)
 - get the AU parameter value in normalized form (thread-safe via AU instance)
 - register and de-register sub-controllers (normally not used, but here for future expansion)
 - register and de-register custom-views
 - send plugin core messages about current GUI states (did open, will close, timer ping)

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */// --- GUI -> Plugin interface
class GUIPluginConnector : public IGUIPluginConnector
{
public:
    /** construct with two pointers */
    GUIPluginConnector(AUSynthPlugin* _auInstance, PluginCore* _pluginCore){auInstance = _auInstance; pluginCore = _pluginCore;}

    /** destroy subcontroller and custom view container objects */
    virtual ~GUIPluginConnector()
    {
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            delete it->second;
        }
        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            delete it->second;
        }
    }

    /** inform core that parameter has changed (NOT to be used to alter plugin states!) */
    virtual void parameterChanged(int32_t controlID, double actualValue, double /*normalizedValue*/)
    {
        if(pluginCore)
            pluginCore->guiParameterChanged(controlID, actualValue);
    }

    /** get AU parameter in actual form */
    virtual double getActualPluginParameter(int32_t controlID)
    {
        PluginParameter* piParam = pluginCore->getPluginParameterByControlID(controlID);
        if(piParam)
        {
            return auInstance->getAUParameter(controlID);
        }
        else
            return 0.0;
    }

    /** get AU parameter in normalized form */
    virtual double getNormalizedPluginParameter(int32_t controlID)
    {
        if(pluginCore && auInstance)
        {
            PluginParameter* piParam = pluginCore->getPluginParameterByControlID(controlID);
            if(piParam)
            {
                double actualValue = getActualPluginParameter(controlID);
                return piParam->getNormalizedControlValueWithActualValue(actualValue);
            }
        }
        return 0.0;
    }

    /** store sub-controller's ICustomView and send message to core */
    virtual bool registerSubcontroller(std::string subcontrollerName, ICustomView* customViewConnector)
    {
        // --- do we have this in our map already?
        CustomViewController* pCVC = nullptr;

        customViewControllerMap::const_iterator it = customSubControllerMap.find(subcontrollerName);
        if (it != customSubControllerMap.end())
        {
            pCVC = it->second;
            pCVC->setCustomViewPtr(customViewConnector);
        }
        else
        {
            pCVC = new CustomViewController(customViewConnector);
            customSubControllerMap.insert(std::make_pair(subcontrollerName, pCVC));
        }

        MessageInfo info(PLUGINGUI_REGISTER_SUBCONTROLLER);
        info.inMessageString = subcontrollerName;
        info.inMessageData = pCVC;

        if (pluginCore && pluginCore->processMessage(info))
            return true;

        return false;
    }

    /** remove sub-controller's ICustomView and send message to core */
    virtual bool deRregisterSubcontroller(ICustomView* customViewConnector)
    {
        CustomViewController* pCVC = getCustomSubController(customViewConnector);
        if (pCVC)
        {
            pCVC->clearCustomViewPtr();

            MessageInfo info(PLUGINGUI_DE_REGISTER_SUBCONTROLLER);
            info.inMessageString = "";
            info.inMessageData = pCVC;

            if (pluginCore && pluginCore->processMessage(info))
                return true;
        }

        return false;
    }


    /** store custom view's ICustomView and send message to core */
    virtual bool registerCustomView(std::string customViewName, ICustomView* customViewConnector)
    {
        // --- do we have this in our map already?
        CustomViewController* pCVC = nullptr;

        customViewControllerMap::const_iterator it = customViewMap.find(customViewName);
        if (it != customViewMap.end())
        {
            pCVC = it->second;
            pCVC->setCustomViewPtr(customViewConnector);
        }
        else
        {
            pCVC = new CustomViewController(customViewConnector);
            customViewMap.insert(std::make_pair(customViewName, pCVC));
        }

        MessageInfo info(PLUGINGUI_REGISTER_CUSTOMVIEW);
        info.inMessageString = customViewName;
        info.inMessageData = pCVC;

        if(pluginCore && pluginCore->processMessage(info))
            return true;

        return false;
    }

    /** remove custom view's ICustomView and send message to core */
    virtual bool deRegisterCustomView(ICustomView* customViewConnector)
    {
        CustomViewController* pCVC = getCustomViewController(customViewConnector);
        if (pCVC)
        {
            // --- clear it
            pCVC->clearCustomViewPtr();

            MessageInfo info(PLUGINGUI_DE_REGISTER_CUSTOMVIEW);
            info.inMessageString = "";
            info.inMessageData = pCVC;

            if (pluginCore && pluginCore->processMessage(info))
                return true;
        }

        return false;
    }

    /** send message to core: GUI has been opened and all VISIBLE controls are active */
    virtual bool guiDidOpen()
    {
        if(!pluginCore) return false;
        MessageInfo info(PLUGINGUI_DIDOPEN);
        return pluginCore->processMessage(info);
    }

    /** send message to core: GUI will close; all controls active but will vanish */
    virtual bool guiWillClose()
    {
        if(!pluginCore) return false;

        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            it->second->clearCustomViewPtr();
        }
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            it->second->clearCustomViewPtr();
        }

        MessageInfo info(PLUGINGUI_WILLCLOSE);
        return pluginCore->processMessage(info);
    }

    /** send message to core: repaint timer */
    virtual bool guiTimerPing()
    {
        if(!pluginCore) return false;
        MessageInfo info(PLUGINGUI_TIMERPING);
        return pluginCore->processMessage(info);
    }

    /** send message to core: a non-bound variable has changed (NOT implemented or needed, but may be added for your own customization) */
    virtual bool checkNonBoundValueChange(int tag, float normalizedValue)
    {
        if(!pluginCore) return false;

        // --- do any additional stuff here
        // --- dispatch non-bound value changes directly to receiver

        return false;
    }


protected:
    PluginCore* pluginCore = nullptr; ///< the core object
    AUSynthPlugin* auInstance = nullptr; ///< the AU plugin (NOTE this is not base-class)

    // --- this is for supporting the persistent interface pointer for the core object
    //     and is required by ASPiK Specifications
    typedef std::map<std::string, CustomViewController*> customViewControllerMap; ///< map of custom view controllers
    customViewControllerMap customViewMap;

    /** get a custom view controller object (internal use only) */
    CustomViewController* getCustomViewController(ICustomView* customViewConnector)
    {
        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            if (it->second->getCustomViewPtr() == customViewConnector)
                return it->second;
        }

        return nullptr;
    }

    customViewControllerMap customSubControllerMap;   ///< map of custom sub-controllers

    /** get a sub-controller controller object (internal use only) */
    CustomViewController* getCustomSubController(ICustomView* customViewConnector)
    {
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            if (it->second->getCustomViewPtr() == customViewConnector)
                return it->second;
        }

        return nullptr;
    }
};


/**
 \class AUMIDIEventQueue
 \ingroup AU-Shell
 \brief
 The AUMIDIEventQueue interface queues incoming MIDI messages and blasts them out during the buffer processing phase.

 NOTES:
 - the current version uses a pair of queues and an atomic boolean flag to implement a double-buffer scheme
 - this will be updated later to use a lock-free ring buffer but the operation will remain transparemt

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */// --- GUI -> Plugin interface
class AUMIDIEventQueue : public IMidiEventQueue
{
public:

    /** consruct with a core pointer */
    AUMIDIEventQueue(PluginCore* _pluginCore)
    {
        pluginCore = _pluginCore;
        writingQueueA.store(true);
    };

    /** just clear out the events that may remain */
    virtual ~AUMIDIEventQueue()
    {
        clearEvents();
    }

    /** clear both A and B queues */
    void clearEvents()
    {
        clearQueueAEvents();
        clearQueueBEvents();
    }

    /** clear queue A */
    void clearQueueAEvents()
    {
        if(midiEventQueueA.size() > 0)
            fprintf(stderr, "midiEventQueueA.size() > 0: %u", midiEventQueueA.size() > 0);

        while(midiEventQueueA.size() > 0)
            midiEventQueueA.pop();
    }

    /** clear queue B */
    void clearQueueBEvents()
    {
        if(midiEventQueueB.size() > 0)
            fprintf(stderr, "midiEventQueueB.size() > 0: %u", midiEventQueueB.size() > 0);

        while(midiEventQueueB.size() > 0)
            midiEventQueueB.pop();
    }

    /** atomic flag toggle */
    void toggleQueue()
    {
        // --- toggle the atomic bool
        writingQueueA = !writingQueueA;

        // --- clear out write-queue (should never be non-empty)
        if(writingQueueA)
            clearQueueAEvents();
        else
            clearQueueBEvents();
    }

    /** add a MIDI event to the currently active writing queue */
    inline void addEvent(midiEvent event)
    {
        if(writingQueueA)
        {
            midiEventQueueA.push(event);
           // fprintf(stderr, "QA addEvent: %u", event.midiData1);
           // fprintf(stderr, "  with offset: %u\n", event.midiSampleOffset);

        }
        else
        {
            midiEventQueueB.push(event);
            //fprintf(stderr, "QB addEvent: %u", event.midiData1);
           // fprintf(stderr, "  with offset: %u\n", event.midiSampleOffset);

        }
    }

    /** get a MIDI event from the currently active reading queue */
    virtual unsigned int getEventCount()
    {
        if(writingQueueA)
            return midiEventQueueB.size();
        else
            return midiEventQueueA.size();
    }

    /** send MIDI event with this sample offset to the core for processing */
    virtual bool fireMidiEvents(unsigned int sampleOffset)
    {
        std::queue<midiEvent>* readingQueue = writingQueueA ? &midiEventQueueB : &midiEventQueueA;

        if(readingQueue->size() <= 0 || !pluginCore) return false;

        while(readingQueue->size() > 0)
        {
            // --- check the current top
            midiEvent event = readingQueue->front();
            if(event.midiSampleOffset != sampleOffset) return false;

           // fprintf(stderr, "fired MIDI Event: %u", event.midiData1);
          //  fprintf(stderr, "  with offset: %u\n", event.midiSampleOffset);

            // --- send to core for processing
            if(pluginCore)
                pluginCore->processMIDIEvent(event);

            // --- pop to remove
            readingQueue->pop();
        }
        return true;
    }


protected:
    PluginCore* pluginCore = nullptr;                   ///< the core object to send MIDI messages to
    std::queue<midiEvent> midiEventQueueA;              ///< queue A
    std::queue<midiEvent> midiEventQueueB;              ///< queue B
    std::atomic<bool> writingQueueA;                    ///< atomic flag for toggling buffers
};
