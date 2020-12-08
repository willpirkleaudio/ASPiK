// --- header


#ifndef __VST3Plugin__
#define __VST3Plugin__

#// --- VST3
#include "public.sdk/source/vst/vstsinglecomponenteffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

// --- MIDI EVENTS
#include "pluginterfaces/vst/ivstevents.h"

// --- WString Support
#include "pluginterfaces/base/ustring.h"

// --- our plugin core object
#include "plugincore.h"
#include "plugingui.h"

// --- windows.h bug
// #define ENABLE_WINDOWS_H 1

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#else
#ifdef ENABLE_WINDOWS_H
	#include <windows.h>
#endif
extern void* hInstance; // VSTGUI hInstance
#endif

#include <vector>

namespace Steinberg {
namespace Vst {
namespace ASPiK {
/*
	The Processor object here ALSO contains the Edit Controller component since these
	are combined as SingleComponentEffect; see documentation
*/
class VSTParamUpdateQueue;
class GUIPluginConnector;
class PluginHostConnector;
class VSTMIDIEventQueue;

// static const ProgramListID kProgramListId = 1;    ///< no programs are used in the unit.



/**
\class VST3Plugin
\ingroup VST-Shell
\brief
The VST3Plugin object is the ASPiK plugin shell for the VST3 API

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class VST3Plugin : public SingleComponentEffect, public IMidiMapping 
{
public:
	// --- constructor
	VST3Plugin();

	// --- destructor
	~VST3Plugin();

    /*** IAudioProcessor Interface ***/
    /** One time init to define our I/O and vsteditcontroller parameters */
    tresult PLUGIN_API initialize(FUnknown* context) override;

	/** Define the audio I/O we support */
    tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) override;

    /**  Define our word-length capabilities (currently 32 bit only) */
    tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) override;

    /**  you can access info about the processing via ProcessSetup; see ivstaudioprocessor.h */
    tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup) override;

    /**  Turn on/off */
    tresult PLUGIN_API setActive(TBool state) override;

    /**  Serialization: Save and load presets from a file stream */
    //					  These get/set the plugin variables
    tresult PLUGIN_API setState(IBStream* fileStream) override;
    tresult PLUGIN_API getState(IBStream* fileStream) override;

    /**  functions to reduce size of process() */
    //     Update the GUI control variables
    bool doControlUpdate(ProcessData& data);

    /**  The all important process method where the audio is rendered/effected */
    tresult PLUGIN_API process(ProcessData& data) override;

	/** IMidiMapping */
    virtual tresult PLUGIN_API getMidiControllerAssignment(int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id/*out*/) override;

	/** IPlugView: create our custom GUI */
    IPlugView* PLUGIN_API createView(const char* _name) override;

    /** the end. this destroys the core */
    tresult PLUGIN_API terminate() override;

	/** for GUI_TIMER_PING and recreate view operations */
	virtual tresult receiveText(const char8* text) override;

	/** helper function for serialization */
	tresult PLUGIN_API setParamNormalizedFromFile(ParamID tag, ParamValue value);

	/** serialize-read from file to setup the GUI parameters */
	tresult PLUGIN_API setComponentState(IBStream* fileStream) override;

	/** for meters */
	void updateMeters(ProcessData& data, bool forceOff = false);

	/** our COM creation method */
	static FUnknown* createInstance(void* context) {return (IAudioProcessor*)new VST3Plugin(); }

	/** IUnitInfo */
	//bool addUnit (Unit* unit);

	/** for future compat; not curently supporting program lists; only have/need Factory Presets! */
	bool addProgramList (ProgramList* list);
	ProgramList* getProgramList(ProgramListID listId) const;
	tresult notifyPogramListChange(ProgramListID listId, int32 programIndex = kAllProgramInvalid);

	tresult PLUGIN_API setParamNormalized (ParamID tag, ParamValue value) override;
    virtual int32 PLUGIN_API getProgramListCount() override;
	virtual tresult PLUGIN_API getProgramListInfo(int32 listIndex, ProgramListInfo& info /*out*/) override;
	virtual tresult PLUGIN_API getProgramName(ProgramListID listId, int32 programIndex, String128 name /*out*/) override;
	virtual tresult PLUGIN_API getProgramInfo(ProgramListID listId, int32 programIndex, CString attributeId /*in*/, String128 attributeValue /*out*/) override;
	virtual tresult PLUGIN_API hasProgramPitchNames(ProgramListID listId, int32 programIndex) override;
	virtual tresult PLUGIN_API getProgramPitchName(ProgramListID listId, int32 programIndex, int16 midiPitch, String128 name /*out*/) override;
    virtual tresult setProgramName(ProgramListID listId, int32 programIndex, const String128 name /*in*/) override;

	/** from IDependent------------------ */
    virtual void PLUGIN_API update(FUnknown* changedUnknown, int32 message) override ;

	// --- latency support
	uint32 m_uLatencyInSamples = 0;		///< set in constructor with plugin
	
	/** base class override */
	virtual uint32 PLUGIN_API getLatencySamples() override {
		return m_uLatencyInSamples; }

	/** base class override for tailtime */
	virtual uint32 PLUGIN_API getTailSamples() override ;

    /** update host info for this process loop */
    void updateHostInfo(ProcessData& data, HostInfo* hostInfo);

    static FUID* getFUID(); ///< static function for VST3 clsss factory
    static const char* getPluginName(); ///< static function for VST3 clsss factory
    static const char* getVendorName(); ///< static function for VST3 clsss factory
    static const char* getVendorURL(); ///< static function for VST3 clsss factory
    static const char* getVendorEmail(); ///< static function for VST3 clsss factory
    static CString getPluginType(); ///< static function for VST3 clsss factory

	// --- define the IMidiMapping interface
	OBJ_METHODS(VST3Plugin, SingleComponentEffect)
	DEFINE_INTERFACES
		DEF_INTERFACE(IMidiMapping)
		DEF_INTERFACE(IUnitInfo)
	END_DEFINE_INTERFACES(SingleComponentEffect)
	REFCOUNT_METHODS(SingleComponentEffect)


private:
    // --- our plugin core and interfaces
    PluginCore* pluginCore = nullptr;						///< the core
    GUIPluginConnector* guiPluginConnector = nullptr;       ///< GUI Plugin interface
    PluginHostConnector* pluginHostConnector = nullptr;     ///< Plugin Host interface
    VSTMIDIEventQueue* midiEventQueue = nullptr;            ///< queue for sample accurate MIDI messaging
	bool plugInSideBypass = false; ///< bypass flag
	bool hasSidechain = false; ///< sidechain flag

protected:
	// --- sample accurate parameter automation
	VSTParamUpdateQueue ** m_pParamUpdateQueueArray = nullptr;	///<  sample accurate parameter automation
	unsigned int sampleAccuracy = 1;///<  sample accurate parameter automation
	bool enableSAAVST3 = false;///<  sample accurate parameter automation

	// --- IUnitInfo and factory Preset support
	typedef std::vector<IPtr<ProgramList> > ProgramListVector;
	typedef std::map<ProgramListID, ProgramListVector::size_type> ProgramIndexMap;
	typedef std::vector<IPtr<Unit> > UnitVector;
	UnitVector units;
	ProgramListVector programLists;
	ProgramIndexMap programIndexMap;
	UnitID selectedUnit;


#if defined _WINDOWS || defined _WINDLL
#ifdef ENABLE_WINDOWS_H
    // --- getMyDLLDirectory()
    //     returns the directory where the .component resides
	char* getMyDLLDirectory(UString cPluginName)
	{
		HMODULE hmodule = GetModuleHandle(cPluginName);

		TCHAR dir[MAX_PATH];
		memset(&dir[0], 0, MAX_PATH*sizeof(TCHAR));
		dir[MAX_PATH-1] = '\0';

		if(hmodule)
			GetModuleFileName(hmodule, &dir[0], MAX_PATH);
		else
			return nullptr;

		// convert to UString
		UString DLLPath(&dir[0], MAX_PATH);

		char* pFullPath = new char[MAX_PATH];
		char* pDLLRoot = new char[MAX_PATH];

		DLLPath.toAscii(pFullPath, MAX_PATH);

		size_t nLenDir = strlen(pFullPath);
		size_t nLenDLL = wcslen(cPluginName) + 1;	// +1 is for trailing backslash
		memcpy(pDLLRoot, pFullPath, nLenDir-nLenDLL);
		pDLLRoot[nLenDir-nLenDLL] = '\0';

		delete [] pFullPath;

		// caller must delete this after use
		return pDLLRoot;
	}
#endif
#endif
#if MAC
    // --- getMyComponentDirectory()
    //     returns the directory where the .component resides
    char* getMyComponentDirectory(CFStringRef bundleID)
    {
        if (bundleID != nullptr)
        {
            CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
            if(helixBundle != nullptr)
            {
                CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
                if(bundleURL != nullptr)
                {
                    CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(nullptr, bundleURL);

                    CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                    CFRelease(componentFolderPathURL);

                    if(myComponentPath != nullptr)
                    {
                        int nSize = CFStringGetLength(myComponentPath);
                        char* path = new char[nSize+1];
                        memset(path, 0, (nSize+1)*sizeof(char));

                        bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                        CFRelease(myComponentPath);

                        if(success) return path;
                        else return nullptr;
                    }
                    CFRelease(bundleURL);
                }
            }
            CFRelease(bundleID);
        }
        return nullptr;
    }
#endif

};

/**
\class VSTParamUpdateQueue
\ingroup VST-Shell

\brief
The VSTParamUpdateQueue object maintains a parameter update queue for one ASPiK PluginParameter object.
It is only used as part of the sample-accurate automation feature in ASPiK.

\author Will Pirkle http://www.willpirkle.com
\remark This object is part of the ASPiK plugin framework
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class VSTParamUpdateQueue : public IParameterUpdateQueue
{
protected:
	unsigned int bufferSize = 0;
	ParamValue initialValue = 0.0;
	ParamValue previousValue = 0.0;
	ParamValue maxValue = 0.0;
	ParamValue minValue = 0.0;

	// --- Store slope and b so that it needs to be calculated only once.
	ParamValue slope;
	ParamValue yIntercept;

    // --- Controls granularity
	unsigned int* sampleAccuracy = nullptr;
	int queueIndex = 0;
	int queueSize = 0;
	IParamValueQueue* parameterQueue = nullptr;
	int x1, x2 = 0;
	double y1, y2 = 0;
	bool dirtyBit = false;
	int sampleOffset = 0;

public:
    VSTParamUpdateQueue(void);
    virtual ~VSTParamUpdateQueue(void){}
	void initialize(ParamValue _initialValue, ParamValue _minValue, ParamValue _maxValue, unsigned int* _sampleAccuracy);
	void setParamValueQueue(IParamValueQueue* _paramValueQueue, unsigned int _bufferSize);
	void setSlope();
	ParamValue interpolate(int x1, int x2, ParamValue y1, ParamValue y2, int x);
	int needsUpdate(int x, ParamValue  &value);

	// --- IParameterUpdateQueue
	unsigned int getParameterIndex();
	bool getValueAtOffset(long int _sampleOffset, double _previousValue, double& _nextValue);
	bool getNextValue(double& _nextValue);
};



/**
\class PluginHostConnector
\ingroup VST-Shell

\brief
The PluginHostConnector implements the IPluginHostConnector interface for the plugin shell object. 
For VST, this requires implementing only one method, sendHostMessage( ). Only one message is processed for 
sendGUIUpdate that provides a mechanism to update the GUI controls from the plugin core. Note that this is not an
ideal solution for most problems (e.g. linking controls together intelligently) -- you should always consider 
using a custom sub-controller and/or custom view to do this properly. On occasion, a more difficult scenario may 
arise (e.g. MIDI learn button, that must wait for user input from a MIDI instrument to toggle states) where the
sendGUIUpdate method may be appropriate. See the example in the ASPiK SDK for more information.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginHostConnector : public IPluginHostConnector
{
public:
    PluginHostConnector(VST3Plugin* _editController) {editController = _editController;}
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

                for(unsigned int i = 0; i < guiUpdateData.guiParameters.size(); i++)
                {
                    GUIParameter guiParam = guiUpdateData.guiParameters[i];
                    if(editController)
                    {
                        ParamValue normalizedValue = editController->plainParamToNormalized(guiParam.controlID, guiParam.actualValue);
                        editController->setParamNormalized(guiParam.controlID, normalizedValue);
                    }
                }

                // --- clean up
				for (unsigned int i = 0; i < guiUpdateData.guiParameters.size(); i++)
                    guiUpdateData.guiParameters.pop_back();

                break;
            }
            default:
                break;
        }
    }

protected:
    VST3Plugin* editController = nullptr; ///< our parent plugin
};


/**
\class CustomViewController
\ingroup AU-Shell

\brief
The CustomViewController is part of the safe ICustomView feature in ASPiK. The CustomViewController maintains an ICustomView pointer.
When the GUI registers and de-registers custom views during creation or destuction, the plugin shell is responsible for making sure 
that the original ICustomView pointer registered with the plugin core object *never goes out of scope* and this object is part of that system.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/// --- GUI -> Plugin interface
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
	ICustomView* customViewIF = nullptr;	///< the currently active ICustomView interface, registered from the GUI
};

/**
\class GUIPluginConnector
\ingroup VST-Shell

\brief
The GUIPluginConnector interface creates a safe message mechanism for the GUI to issue requests to the plugin shell.
The following messages are processed via functions:

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
*/
class GUIPluginConnector : public IGUIPluginConnector
{
public:
	/** construct with two pointers */
	GUIPluginConnector(PluginCore* _pluginCore, VST3Plugin* _editController)
    {
        pluginCore = _pluginCore;
        editController = _editController;
    }

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
	virtual void parameterChanged(int32_t controlID, double actualValue, double normalizedValue)
    {
        if(pluginCore)
            pluginCore->guiParameterChanged(controlID, actualValue);

        if(!editController) return;

        // --- set parameter on object
        editController->setParamNormalized(controlID, normalizedValue);

        // --- perform the operation
        editController->performEdit(controlID, normalizedValue);
    }

	/** get VST parameter in normalized form */
    virtual double getNormalizedPluginParameter(int32_t controlID)
    {
        if(!editController) return 0.0;

        Parameter* param = editController->getParameterObject(controlID);
        if(!param) return 0.0;

        return param->getNormalized();
    }

	/**  get plugin parameter as actual value */
	virtual double getActualPluginParameter(int32_t controlID)
	{
		if (!editController) return 0.0;

		Parameter* param = editController->getParameterObject(controlID);
		if (!param) return 0.0;

		// --- get normalized and convert
		double normalizedValue = param->getNormalized();
		return param->toPlain(normalizedValue);
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
    PluginCore* pluginCore = nullptr;	///< the core object
    VST3Plugin* editController = nullptr;	///< the VST3

	// --- this is for supporting the persistent interface pointer for the core object
	//     and is required by ASPiK Specifications
	typedef std::map<std::string, CustomViewController*> customViewControllerMap;///< map of custom view controllers
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

	customViewControllerMap customSubControllerMap;///< map of custom sub-controllers

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
\class VSTMIDIEventQueue
\ingroup VST-Shell
\brief
The VSTMIDIEventQueue interface queues incoming MIDI messages and blasts them out during the buffer processing phase.

NOTES:
- this is a simple object because the VST spec automatically delivers queues of MIDI messages
- so this provides a kind of thin wrapper around those messages to deliver to the core

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class VSTMIDIEventQueue : public IMidiEventQueue
{
public:
    VSTMIDIEventQueue(PluginCore* _pluginCore)
    {
        pluginCore = _pluginCore;
    };

    virtual ~VSTMIDIEventQueue(){}

public:
    /** set a new list from VST host*/
    void setEventList(IEventList* _inputEvents)
    {
        inputEvents = _inputEvents;
        currentEventIndex = 0;
    }

	/** get count of messages in current list */
	virtual unsigned int getEventCount()
    {
        if (inputEvents)
            return inputEvents->getEventCount();

        return 0;
    }

    /** send MIDI event at this sample offset to core */
    virtual bool fireMidiEvents(unsigned int sampleOffset)
    {
        if (!inputEvents)
           return false;

        Event e = { 0 };
        bool eventOccurred = false;
        bool haveEvents = false;
        if (inputEvents->getEvent(currentEventIndex, e) == kResultTrue)
            haveEvents = true;
        else
            return false;

        const unsigned char MIDI_NOTE_OFF = 0x80;
        const unsigned char MIDI_NOTE_ON = 0x90;
        const unsigned char MIDI_POLY_PRESSURE = 0xA0;

        while (haveEvents)
        {
            if (inputEvents->getEvent(currentEventIndex, e) == kResultTrue)
            {
                if (e.sampleOffset != sampleOffset)
                    return false;

                // --- process Note On or Note Off messages
                switch (e.type)
                {
                    // --- NOTE ON
                    case Event::kNoteOnEvent:
                    {
                        midiEvent event;
                        event.midiMessage = (unsigned int)MIDI_NOTE_ON;
                        event.midiChannel = (unsigned int)e.noteOn.channel;
                        event.midiData1 = (unsigned int)e.noteOn.pitch;
                        event.midiData2 = (unsigned int)(127.0*e.noteOn.velocity);
                        event.midiSampleOffset = e.sampleOffset;
                        eventOccurred = true;

                        // --- send to core for processing
                        if(pluginCore)
                            pluginCore->processMIDIEvent(event);
                        break;
                    }

                    // --- NOTE OFF
                    case Event::kNoteOffEvent:
                    {
                        // --- get the channel/note/vel
                        midiEvent event;
                        event.midiMessage = (unsigned int)MIDI_NOTE_OFF;
                        event.midiChannel = (unsigned int)e.noteOff.channel;
                        event.midiData1 = (unsigned int)e.noteOff.pitch;
                        event.midiData2 = (unsigned int)(127.0*e.noteOff.velocity);
                        event.midiSampleOffset = e.sampleOffset;
                        eventOccurred = true;

                        // --- send to core for processing
                        if(pluginCore)
                            pluginCore->processMIDIEvent(event);

                        break;
                    }

                    // --- polyphonic aftertouch 0xAn
                    case Event::kPolyPressureEvent:
                    {
                        midiEvent event;
                        event.midiMessage = (unsigned int)MIDI_POLY_PRESSURE;
                        event.midiChannel = (unsigned int)e.polyPressure.channel;
                        event.midiData1 = (unsigned int)e.polyPressure.pitch;
                        event.midiData2 = (unsigned int)(127.0*e.polyPressure.pressure);
                        event.midiSampleOffset = e.sampleOffset;
                        eventOccurred = true;

                        // --- send to core for processing
                        if(pluginCore)
                            pluginCore->processMIDIEvent(event);

                        break;
                    }
                } // switch

                // --- have next event?
                if (inputEvents->getEvent(currentEventIndex + 1, e) == kResultTrue)
                {
                    if (e.sampleOffset == sampleOffset)
                    {
                        // --- avance current index
                        currentEventIndex++;
                    }
                    else
                        haveEvents = false;
                }
                else
                    haveEvents = false;
            }
        }

        return eventOccurred;
    }

protected:
    PluginCore* pluginCore = nullptr; ///< the core object
    IEventList* inputEvents = nullptr;	///< the current event list for this buffer cycle
    unsigned int currentEventIndex = 0;	///< index of current event
};

/**
\class VST3UpdateHandler
\ingroup VST-Shell
\brief
Little update handler object for VST-approved GUI updating

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class VST3UpdateHandler: public FObject
{
public:
    VST3UpdateHandler(VSTGUI::ControlUpdateReceiver* _receiver, VST3Plugin* _editController){ receiver = _receiver; editController = _editController; }
    ~VST3UpdateHandler(){}

    virtual void PLUGIN_API update (FUnknown* changedUnknown, int32 message)
    {
        if(message == IDependent::kChanged && receiver && editController)
        {
            double normalizedValue = editController->getParamNormalized (receiver->getControlID());
            receiver->updateControlsWithNormalizedValue(normalizedValue);
        }
    }

private:
    VSTGUI::ControlUpdateReceiver* receiver = nullptr;
    VST3Plugin* editController = nullptr;

};


/**
\class PluginEditor
\ingroup VST-Shell
\brief
The VST GUI for the plugin. This is needed because VST3 requires an IPlugView GUI, which is VST3 specific (involves VST3 SDK files)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginEditor: public CPluginView, public VSTGUI::PluginGUI, public IGUIWindowFrame
{
public:
    PluginEditor(VSTGUI::UTF8StringPtr _xmlFile, PluginCore* _pluginCore, GUIPluginConnector* _guiPluginConnector, PluginHostConnector* _pluginHostConnector, VST3Plugin* editController);
    virtual ~PluginEditor();

    // --- aet of update handlers, specific to VST3: this will allow us to
    //     remotely update the GUI in a threadsafe and VST-approved manner
    typedef std::map<int32_t, VST3UpdateHandler*> UpdaterHandlerMap;
    UpdaterHandlerMap updateHandlers;

    //---from IPlugView------- VST3 Specific
    IPlugFrame* plugFrame;
    const ViewRect& getRect() const { return rect; }
    void setRect(const ViewRect& r)  { rect = r; }
    bool isAttached() const  { return systemWindow != 0; }
    virtual void attachedToParent() override {}
    virtual void removedFromParent() override {}

    virtual tresult PLUGIN_API attached(void* parent, FIDString type) override;
    virtual tresult PLUGIN_API removed() override;
    virtual tresult PLUGIN_API onWheel(float distance) override { return kResultFalse; }

    virtual tresult PLUGIN_API isPlatformTypeSupported(FIDString type) override;
    virtual tresult PLUGIN_API onSize(ViewRect* newSize) override;
    virtual tresult PLUGIN_API getSize(ViewRect* size) override;

    virtual tresult PLUGIN_API onFocus(TBool /*state*/)  override { return kResultFalse; }
    virtual tresult PLUGIN_API setFrame(IPlugFrame* frame) override;// { plugFrame = frame; return kResultTrue; }
    virtual tresult PLUGIN_API canResize()  override{ return kResultFalse /*kResultTrue*/; }
    virtual tresult PLUGIN_API checkSizeConstraint(ViewRect* rect)  override
    {
        if (showGUIEditor)
            return kResultTrue;

        // --- clamp it
        ViewRect viewRect = getRect();
        rect->right = viewRect.right;
        rect->bottom = viewRect.bottom;

        return kResultFalse;
    }

    virtual bool setWindowFrameSize(double left = 0, double top = 0, double right = 0, double bottom = 0)  override //CRect* newSize)
    {
        ViewRect vr(0, 0, right, bottom);
        setRect(vr);
        if (plugFrame)
            plugFrame->resizeView(this, &vr);
        return true;
    }

    virtual bool getWindowFrameSize(double& left, double&  top, double&  right, double&  bottom)  override
    {
        ViewRect viewRect = getRect();
        left = 0.0;
        top = 0.0;
        right = viewRect.getWidth();
        bottom = viewRect.getHeight();
        return true;
    }

protected:
    PluginCore* pluginCore = nullptr;						///< the core
    GUIPluginConnector* guiPluginConnector = nullptr;		///< GUI Plugin interface
    PluginHostConnector* pluginHostConnector = nullptr;		///< Plugin Host interface
    VST3Plugin* editController = nullptr;	///< parent VST3
};



}}} // namespaces

#endif





