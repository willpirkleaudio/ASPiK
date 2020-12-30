
#include "plugingui.h"

// --- custom data view example; include more custom views here
#include "customviews.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <AssertMacros.h>

#ifdef AUPLUGIN
// --- for au event granularity/interval; these are recommended defaults
const Float32 AUEVENT_GRANULARITY = 0.0005;
const Float32 AUEVENT_INTERVAL = 0.0005;
enum {kBeginEdit, kEndEdit, kValueChanged, kNumEventTypes};

// --- this is called when presets change for us to synch GUI
void EventListenerDispatcher(void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inHostTime, Float32 inValue)
{
    VSTGUI::PluginGUI *pEditor = (VSTGUI::PluginGUI*)inRefCon;
    if(!pEditor)
        return;

    // --- kAudioUnitEvent_ParameterValueChange
	// --- update the GUI control
    if(pEditor)
        pEditor->dispatchAUControlChange(inEvent->mArgument.mParameter.mParameterID, inValue, true);
}
#endif

namespace VSTGUI
{
    void* gBundleRef = 0;
    static int openCount = 0;

	static void CreateVSTGUIBundleRef();
	static void ReleaseVSTGUIBundleRef();

// --- BundleRef Functions for VSTGUI4 only ------------------------------------------------------------------------
void CreateVSTGUIBundleRef()
{
    openCount++;
    if(gBundleRef)
    {
        CFRetain(gBundleRef);
        return;
    }
#if TARGET_OS_IPHONE
    gBundleRef = CFBundleGetMainBundle();
    CFRetain(gBundleRef);
#else
    Dl_info info;
    if(dladdr((const void*)CreateVSTGUIBundleRef, &info))
    {
        if(info.dli_fname)
        {
            std::string name;
            name.assign (info.dli_fname);
            for (int i = 0; i < 3; i++)
            {
                int delPos = name.find_last_of('/');
                if (delPos == -1)
                {
                    fprintf (stdout, "Could not determine bundle location.\n");
                    return; // unexpected
                }
                name.erase (delPos, name.length () - delPos);
            }
            CFURLRef bundleUrl = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)name.c_str(), name.length (), true);
            if (bundleUrl)
            {
                gBundleRef = CFBundleCreate (0, bundleUrl);
                CFRelease (bundleUrl);
            }
        }
    }
#endif
}
void ReleaseVSTGUIBundleRef()
{
    openCount--;
    if(gBundleRef)
        CFRelease (gBundleRef);
    if(openCount == 0)
        gBundleRef = 0;
}

#elif WINDOWS
#pragma warning(disable : 4189)	// --- local variable is initialized but not referenced
#ifdef RAFXPLUGIN
void* hInstance; // VSTGUI hInstance
extern void* moduleHandle;
#endif

#ifdef AAXPLUGIN
extern void* hInstance;
#if WINDOWS_VERSION
#include <windows.h>
#include "vstgui/lib/platform/win32/win32frame.h"
#endif
#endif

#ifdef VSTPLUGIN
void* hInstance; // VSTGUI hInstance
extern void* moduleHandle;
#endif

namespace VSTGUI
{
#endif // MAC ---END  BundleRef Functions for VSTGUI4 only ------------------------------------------------------------------------

const bool ENABLE_CUSTOM_VIEWS = true;

/**
\brief PluginGUI constructor; note that this maintains both Mac and Windows contexts, the bundle ref for Mac
and the external void* for Windows.

Operation: \n
- creates UIDescription from the XML file; stores both the file and the description object (they get used or written later)
- calls description->parse() to convert XML into objects
- initializes main attributes
- sets up the GUI timer for a 50 millisecond repaint interval
*/
PluginGUI::PluginGUI(UTF8StringPtr _xmlFile) :
	IController(),
	IViewAddedRemovedObserver(),
	IMouseObserver(),
	IKeyboardHook(),
	VSTGUIEditorInterface(),
	CBaseObject(),
	IGUIView()
{
#if MAC
	CreateVSTGUIBundleRef();
#elif WINDOWS
#ifndef AAXPLUGIN // VST or RAFX Win
	hInstance = moduleHandle;
#endif
#endif

#ifdef AUPLUGIN
    m_AU = nullptr;
#endif

	// --- create description from XML file
	xmlFile = _xmlFile;
	description = new UIDescription(_xmlFile);
	if (!description->parse())
	{
		description->forget();
		description = nullptr;
	}

    // --- set attributes
    guiPluginConnector = nullptr;
    guiDesignerWidth = 1024.0;
    guiDesignerHeight = 768.0;
	guiWidth = 400.0;
	guiHeight = 200.0;
	zoomFactor = 1.0;
	showGUIEditor = false;
	createNewView = true;
    setGUIWindowFrame(nullptr);
    knobAction = kLinearMode; // --- Default is linear

    // --- create timer
    timer = new CVSTGUITimer(dynamic_cast<CBaseObject*>(this));
}

/**
\brief PluginGUI destructor

Operation: \n
- destroys UIDescription
- destroys timer
*/
PluginGUI::~PluginGUI()
{
	// --- destroy description
	if (description)
		description->forget();

	// --- kill timer
	if (timer)
		timer->forget();
#if MAC
	ReleaseVSTGUIBundleRef();
#endif
}

/**
\brief creates the GUI control objects, creates outer frame, inserts contents into window

Operation:\n
- make a copy of the parameter control list for local use
- pick up key attribute values from XML file
- create the frame
- create and embed the controls
- initialize controls to match current state of underlying parameters 
- start the timer

\param _viewName name for window title
\param parent the window pointer: for Windows it is HWND and for MacOS it is NSView*
\param pluginParameterPtr a list of the plugin parameters; this object makes a COPY of that information and does NOT hold a pointer to it
\param platformType the window type (Windows or MacOS)
\param _guiPluginConnector an interface pointer on the plugin shell that creates, maintains, and destroys this object
\param data optional OS-specific data (for AU it is the AU reference, etc...)
*/
bool PluginGUI::open(UTF8StringPtr _viewName, void* parent, const std::vector<PluginParameter*>* pluginParameterPtr, const PlatformType& platformType, IGUIPluginConnector* _guiPluginConnector, void* data)
{
#ifdef AUPLUGIN
	m_AU = AudioUnit(data);
#endif

	guiPluginConnector = _guiPluginConnector;
	viewName = _viewName;

	if (!pluginParameterPtr)
		return false;

	// --- create our control list of PluginParameter*s
	deleteGUIControlList();
	int size = (int)pluginParameterPtr->size();
	for (int i = 0; i < size; i++)
	{
		PluginParameter* ctrl = new PluginParameter(*(*pluginParameterPtr)[i]);
		pluginParameters.push_back(ctrl);
	}

	// --- add the preset file writer (not a plugin parameter, but a GUI parameter - does not need to be stored/refreshed
	PluginParameter* piParam = new PluginParameter(WRITE_PRESET_FILE, "Preset", "SWITCH_OFF,SWITCH_ON", "SWITCH_OFF");
	piParam->setIsDiscreteSwitch(true);
	pluginParameters.push_back(piParam);

	// --- set knob action
	UIAttributes* attributes = description->getCustomAttributes("Settings", true);
	if (attributes)
	{
		int32_t value = 0;
		attributes->getIntegerAttribute("KnobAction", value);
		knobAction = value;
	}

	// --- GUI Designer sizing
	attributes = description->getCustomAttributes("UIEditController", true);
	if (attributes)
	{
		if (attributes->hasAttribute("EditorSize"))
		{
			CRect r;
			if (attributes->getRectAttribute("EditorSize", r))
			{
				guiDesignerWidth = r.getWidth();
				guiDesignerHeight = r.getHeight();
			}
		}
	}

	// --- get the GUI size for scaling purposes (bonus parameter)
	const UIAttributes* viewAttributes = description->getViewAttributes("Editor");
	if (viewAttributes)
	{
		if (viewAttributes->hasAttribute("size"))
		{
			CPoint p;
			if (viewAttributes->getPointAttribute("size", p))
			{
				guiWidth = (float)p.x;
				guiHeight = (float)p.y;
			}
		}
	}

	// --- create empty frame
	const CRect frameSize(0, 0, 0, 0);
	guiEditorFrame = new CFrame(frameSize, this);
	guiEditorFrame->open(parent, platformType);

	// --- save immediately so getFrame() will work
	frame = guiEditorFrame;

	guiEditorFrame->setTransparency(true);
	guiEditorFrame->registerMouseObserver((IMouseObserver*)this);
	guiEditorFrame->setViewAddedRemovedObserver((IViewAddedRemovedObserver*)this);

#if VSTGUI_LIVE_EDITING
	guiEditorFrame->registerKeyboardHook((IKeyboardHook*)this);
#endif
	guiEditorFrame->enableTooltips(true);

	// --- one time API-specific inits
	preCreateGUI();

	// --- create the views, size the frame
	if (!createGUI(showGUIEditor))
	{
		frame->forget();
		return false;
	}

	// --- begin timer
	if (timer)
	{
		timer->setFireTime((uint32_t)GUI_METER_UPDATE_INTERVAL_MSEC);
		timer->start();
	}

	// --- for custom views
	if (guiPluginConnector)
		guiPluginConnector->guiDidOpen();

	return true;
}



/**
\brief prepares the GUI control objects for destruction, cleans up

Operation:\n
- guiPluginConnector->guiWillClose() to notify the shell that the GUI is getting ready to shut down
- clear control receiver list
- stop the timer
- null the frame pointer
- final frame forget
*/
void PluginGUI::close()
{
    // --- for custom views, null interface pointers
    if(guiPluginConnector)
        guiPluginConnector->guiWillClose();

	if (frame)
	{
#if VSTGUI_LIVE_EDITING
		guiEditorFrame->unregisterKeyboardHook((IKeyboardHook*)this);
#endif
		guiEditorFrame->unregisterMouseObserver((IMouseObserver*)this);

#ifdef AUPLUGIN
        if (AUEventListener) __Verify_noErr(AUListenerDispose(AUEventListener));
        AUEventListener = NULL;
        m_AU = NULL;
#endif

        deleteControlUpdateReceivers();
        forgetWriteableControls();
        deleteGUIControlList();

		if(timer)
		    timer->stop();

        CFrame* oldFrame = frame;
        frame = 0;
        oldFrame->forget();
    }
}

/**
\brief safely sets the GUI control value based on the plugin parameter value

Notes:\n
- called when the control is created ( see ...TagDidChange() )
- in the case of ui-view-switch views, this will be called NOT at startup, unless it is view 0 in the stack
- uses the API-approved thread-safe mechanism for providing sync

\param controlID the ID value of the control to sync
*/
void PluginGUI::syncGUIControl(uint32_t controlID)
{
#ifdef AUPLUGIN
	double param = 0.0;
	if (guiPluginConnector)
	{
		param = guiPluginConnector->getActualPluginParameter(controlID);
		dispatchAUControlChange(controlID, param, false, false);
	}
#endif

#ifdef AAXPLUGIN
	double param = 0.0;
	if (guiPluginConnector)
	{
		param = guiPluginConnector->getNormalizedPluginParameter(controlID);
		updateGUIControlAAX(controlID, -1, (float)param, true); // true: use normalized version
	}
#endif

#ifdef VSTPLUGIN
	double paramNormalizedValue = 0.0;

	if (guiPluginConnector)
	{
		paramNormalizedValue = guiPluginConnector->getNormalizedPluginParameter(controlID);
		updateGUIControlVST(controlID, (float)paramNormalizedValue);
	}
#endif

#ifdef RAFXPLUGIN
	double paramNormalizedValue = 0.0;

	if (guiPluginConnector)
	{
		paramNormalizedValue = guiPluginConnector->getNormalizedPluginParameter(controlID);
		updateGUIControlRAFX(controlID, (float)paramNormalizedValue);
	}
#endif

	// --- built-in bonus parameter for scaling GUI
	if (controlID == SCALE_GUI_SIZE)
	{
		ControlUpdateReceiver* receiver = getControlUpdateReceiver(controlID);
		if (receiver)
		{
			PluginParameter param = receiver->getGuiControl();
			scaleGUISize((uint32_t)param.getControlValue());
		}
	}
}


/**
\brief returns the size into the pass-by-reference variables
\param width return value of width
\param height return value of height
*/
void PluginGUI::getSize(float& width, float& height)
{
	CRect rect;
	rect = frame->getViewSize();
	width = (float)rect.getWidth();
	height = (float)rect.getHeight();
}

/** 
\brief scales the GUI; this is the handler for the special scaling GUI control

\param controlValue the scaling value from the secret control
*/
void PluginGUI::scaleGUISize(uint32_t controlValue)
{
	float scalePercent = 100.f;
	switch (controlValue)
	{
	case tinyGUI:
	{
		scalePercent = 65.f;
		break;
	}
	case verySmallGUI:
	{
		scalePercent = 75.f;
		break;
	}
	case smallGUI:
	{
		scalePercent = 85.f;
		break;
	}
	case normalGUI:
	{
		scalePercent = 100.f;
		break;
	}
	case largeGUI:
	{
		scalePercent = 125.f;
		break;
	}
	case veryLargeGUI:
	{
		scalePercent = 150.f;
		break;
	}
	}

	double width = guiWidth;
	double height = guiHeight;
	zoomFactor = scalePercent / 100.f;
	width *= zoomFactor;
	height *= zoomFactor;

	getFrame()->setSize(width, height);
	getFrame()->setTransform(CGraphicsTransform().scale(zoomFactor, zoomFactor));
	//getFrame()->setZoom(zoomFactor);
	getFrame()->invalid();

	CRect rect(0, 0, 0, 0);
	rect.right = width;
	rect.bottom = height;

	// --- resize
	if (guiWindowFrame)
	{
		guiWindowFrame->setWindowFrameSize(rect.left, rect.top, rect.right, rect.bottom);
	}
}

/**
\brief writes the current state of all GUI controls into a text file so that yuu
	   can simply cut and paste the C++ code into the core object file.
*/
void PluginGUI::writeToPresetFile()
{
	if (!guiPluginConnector)
		return;

	// --- preset name is name that user chooses for file; one file per preset
	std::string presetName;

	std::string savePath;
	std::string saveFile = "Preset_X";
	saveFile.append(".txt");

	CNewFileSelector* fileSelector = CNewFileSelector::create(frame, CNewFileSelector::kSelectSaveFile);
	if (fileSelector == 0)
		return;

	fileSelector->setTitle("Save Preset File");

	// --- NOTE: setDefaultExtension() causes a crash
	// fileSelector->setDefaultExtension(CFileExtension("VSTGUI UI Description", "uidesc"));

	// --- only one option
	fileSelector->setDefaultSaveName(saveFile.c_str());

	if (fileSelector->runModal())
	{
		UTF8StringPtr filePathSelected = fileSelector->getSelectedFile(0);
		if (filePathSelected)
		{
			std::string path(filePathSelected);
			std::string ext;

			size_t sep = path.find_last_of("\\/");
			if (sep != std::string::npos)
				path = path.substr(sep + 1, path.size() - sep - 1);

			size_t dot = path.find_last_of(".");
			if (dot != std::string::npos)
			{
				presetName = path.substr(0, dot);
				ext = path.substr(dot, path.size() - dot);
			}
			else
			{
				presetName = path;
				ext = "";
			}

			savePath = filePathSelected;
		}
	}
	fileSelector->forget();

	if (savePath.empty())
		return;

	CFileStream stream;
	if (stream.open(savePath.c_str(), CFileStream::kWriteMode | CFileStream::kTruncateMode))
	{
		stream.seek(0, SeekableStream::kSeekEnd);

		stream << "\tint index = 0;\t/*** declare this once at the top of the presets function, comment out otherwise */\n";
		std::string comment = "\t// --- Plugin Preset: ";
		comment.append(presetName);
		comment.append("\n");
		stream << comment;

		std::string declaration = "\tPresetInfo* ";
		declaration.append(presetName);
		declaration.append(" = new PresetInfo(index++, \"");
		declaration.append(presetName);
		declaration.append("\");\n");
		stream << declaration;

		// --- initialize
		std::string presetStr2 = "\tinitPresetParameters(";
		presetStr2.append(presetName);
		presetStr2.append("->presetParameters);\n");
		stream << presetStr2;

		size_t size = this->pluginParameters.size();
		for (unsigned int i = 0; i<size - 1; i++) // size-1 because last parameter is WRITE_PRESET_FILE
		{
			PluginParameter* piParam = pluginParameters[i];
			if (piParam)
			{
				std::string paramName = "\t// --- ";
				paramName.append(piParam->getControlName());
				paramName.append("\n");

				uint32_t id = piParam->getControlID();
				double value = guiPluginConnector->getActualPluginParameter(piParam->getControlID());

				std::string presetStr2 = "\tsetPresetParameter(";
				presetStr2.append(presetName);
				presetStr2.append("->presetParameters, ");

				std::ostringstream controlID_ss;
				std::ostringstream controlValue_ss;

				controlID_ss << id;
				controlValue_ss << value;
				std::string controlID(controlID_ss.str());
				std::string controlValue(controlValue_ss.str());

				presetStr2.append(controlID);
				presetStr2.append(", ");
				presetStr2.append(controlValue);
				presetStr2.append(");");
				presetStr2.append(paramName); // has \n
				stream << presetStr2;
			}
		}
		// --- add the preset
		if (size > 1)
		{
			std::string presetStr3 = "\taddPreset(";
			presetStr3.append(presetName);
			presetStr3.append(");\n\n");
			stream << presetStr3;
		}
	}
}

/**
\brief perform idling operation; called directly from timer thread

Operation:\n
- send the timer ping message
- send process loop output data to any output-only receivers (meters)
- issue the repaint message to the outer frame
*/
void PluginGUI::idle()
{
    if(!showGUIEditor)
    {
        // --- for custom views that need animation
        if(guiPluginConnector)
            guiPluginConnector->guiTimerPing();

        for(std::vector<CControl*>::iterator it = writeableControls.begin(); it != writeableControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl)
            {
                double param = 0.0;
                if(guiPluginConnector)
                {
                    param = guiPluginConnector->getNormalizedPluginParameter(ctrl->getTag());
                    ctrl->setValue((float)param);
                    ctrl->invalid();
                }
            }
        }
    }

    // --- update frame - important; this updates all children
    if(frame)
        frame->idle();
}


/**
\brief one-time pre-create init, currently used for AU only

Operation:\n
- AU: setup the event listener system (see FX book for details)
*/
void PluginGUI::preCreateGUI()
{
	if (!frame)
		return;

#ifdef AUPLUGIN
	// --- setup event listener
	if (m_AU)
	{
		// --- create the event listener and tell it the name of our Dispatcher function
		//     EventListenerDispatcher
		__Verify_noErr(AUEventListenerCreate(EventListenerDispatcher, this,
			CFRunLoopGetCurrent(), kCFRunLoopDefaultMode,
			AUEVENT_GRANULARITY, AUEVENT_INTERVAL,
			&AUEventListener));

		// --- setup automation handlers
		//
		//     see also: controlBeginEdit() -> begin gesture message sent to host
		//               controlEndEdit()   -> end gesture message sent to host
		//               valueChanged()     -> param change message sent to host
		for (int i = 0; i< kNumEventTypes; i++)
		{
			// --- start with first control 0
			AudioUnitEvent auEvent;

			// --- parameter 0
			AudioUnitParameter parameter = { m_AU, 0, kAudioUnitScope_Global, 0 };

			// --- set param & add it
			auEvent.mArgument.mParameter = parameter;
			if (i == kBeginEdit)
				auEvent.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
			else if (i == kEndEdit)
				auEvent.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
			else if (i == kValueChanged)
				auEvent.mEventType = kAudioUnitEvent_ParameterValueChange;

			__Verify_noErr(AUEventListenerAddEventType(AUEventListener, this, &auEvent));

			// --- add the rest of the params
			int nParams = pluginParameters.size();
			for (int i = 1; i<nParams; i++)
			{
				PluginParameter* piParam = pluginParameters[i];
				if (piParam)
				{
					auEvent.mArgument.mParameter.mParameterID = piParam->getControlID();
					__Verify_noErr(AUEventListenerAddEventType(AUEventListener, this, &auEvent));
				}
			}
		}
	}
#endif
}


/**
\brief validates menu item selections to prevent crashing

Operation:\n
- test menu item; application specific

\param item the menu item to test

\return true if item is valid, false otherwise
*/
bool PluginGUI::validateCommandMenuItem(CCommandMenuItem* item)
{
#if VSTGUI_LIVE_EDITING
	if (item->getCommandCategory() == "File")
	{
		if (item->getCommandName() == "Save")
		{
			bool enable = false;
			UIAttributes* attributes = description->getCustomAttributes("ASPiKEditor", true);
			if (attributes)
			{
				const std::string* filePath = attributes->getAttributeValue("Path");
				if (filePath)
				{
					enable = true;
				}
			}
			else
			{
				attributes = description->getCustomAttributes("VST3Editor", true);
				if (attributes)
				{
					const std::string* filePath = attributes->getAttributeValue("Path");
					if (filePath)
					{
						enable = true;
					}
				}
			}
			item->setEnabled(enable);
			return true;
		}
	}
#endif
	return false;
}

/**
\brief message handler for GUI Designer menu

Operation:\n
- decode menu item and set flags as needed

\param item the menu item to decode

\return true if menu item command was exceuted, false otherwise
*/
bool PluginGUI::onCommandMenuItemSelected(CCommandMenuItem* item)
{
	auto& cmdCategory = item->getCommandCategory();
	auto& cmdName = item->getCommandName();

#if VSTGUI_LIVE_EDITING
	if (cmdCategory == "File")
	{
		if (cmdName == "Open UIDescription Editor")
		{
			showGUIEditor = true;
			createNewView = true;
			return true;
		}
		else if (cmdName == "Close UIDescription Editor")
		{
			showGUIEditor = false;
			createNewView = true;
			return true;
		}
		else if (cmdName == "Save")
		{
			save(false);
			item->setChecked(false);
			return true;
		}
		else if (cmdName == "Save As")
		{
			save(true);
			item->setChecked(false);
			return true;
		}
		else if (cmdName == "Increase Width")
		{
			// --- 10% UP
			guiDesignerWidth += guiDesignerWidth*0.10;
			showGUIEditor = true;
			createNewView = true;

			return true;
		}
		else if (cmdName == "Decrease Width")
		{
			// --- 10% DOWN
			guiDesignerWidth -= guiDesignerWidth*0.10;
			showGUIEditor = true;
			createNewView = true;

			return true;
		}
		else if (cmdName == "Increase Height")
		{
			// --- 10% UP
			guiDesignerHeight += guiDesignerHeight*0.10;
			showGUIEditor = true;
			createNewView = true;

			return true;
		}
		else if (cmdName == "Decrease Height")
		{
			// --- 10% DOWN
			guiDesignerHeight -= guiDesignerHeight*0.10;
			showGUIEditor = true;
			createNewView = true;

			return true;
		}
	}
#endif
	return false;
}


/**
\brief creates either the GUI or the GUI Designer

Operation:\n
- create the GUI Designer if the boolean flag is set
- otherwise create the normal GUI
- note that this is all handled with the description that we cahched in the constructor
- for the GUI designer, we create a new UIEditController 
- for the normal GUI, we create a new View

\param bShowGUIEditor create the drag-and-drop GUI designer

\return true if GUI is created, false if critical failure 
*/
bool PluginGUI::createGUI(bool bShowGUIEditor)
{
	createNewView = false;

	if (getFrame())
	{
		getFrame()->removeAll();

#if VSTGUI_LIVE_EDITING
		if (bShowGUIEditor)
		{
			guiEditorFrame->setTransform(CGraphicsTransform());
			nonEditRect = guiEditorFrame->getViewSize();
			description->setController((IController*)this);

			// --- persistent
			UIAttributes* attributes = description->getCustomAttributes("UIEditController", true);
			if (attributes)
			{
				std::string editorSize = getGUIDesignerSize();
				attributes->setAttribute("EditorSize", editorSize.c_str());
			}

			// --- new way, stopped bug with
			UIEditController* editController = new UIEditController(description);
			CView* view = editController->createEditView();
			if (view)
			{
				showGUIEditor = true;
				getFrame()->addView(view);
				int32_t autosizeFlags = view->getAutosizeFlags();
				view->setAutosizeFlags(0);

				// added WP
				getFrame()->setFocusDrawingEnabled(false);

				CRect rect(0, 0, 0, 0);
				rect.right = view->getWidth();
				rect.bottom = view->getHeight();

				getFrame()->setSize(view->getWidth(), view->getHeight());
				rect.right = view->getWidth();
				rect.bottom = view->getHeight();

				view->setAutosizeFlags(autosizeFlags);

				getFrame()->enableTooltips(true);
				CColor focusColor = kBlueCColor;
				editController->getEditorDescription()->getColor("focus", focusColor);
				getFrame()->setFocusColor(focusColor);
				getFrame()->setFocusDrawingEnabled(true);
				getFrame()->setFocusWidth(1);

				COptionMenu* fileMenu = editController->getMenuController()->getFileMenu();

				if (fileMenu)
				{
					CCommandMenuItem::Desc menuDesc;
					//Desc(const UTF8String& title, ICommandMenuItemTarget* target,
					//	const UTF8String& commandCategory = nullptr,
					//	const UTF8String& commandName = nullptr)

					CMenuItem* item = fileMenu->addEntry(new CCommandMenuItem({ "Save", this, "File", "Save" }), 0);
					item->setKey("s", kControl);
					item = fileMenu->addEntry(new CCommandMenuItem({ "Save As..", this, "File", "Save As" }), 1);
					item->setKey("s", kShift | kControl);
					item = fileMenu->addEntry(new CCommandMenuItem({ "Close Editor", this, "File", "Close UIDescription Editor" }));
					item->setKey("e", kControl);

					fileMenu->addSeparator();
					item = fileMenu->addEntry(new CCommandMenuItem({ "Increase Width", this, "File", "Increase Width" }));
					item->setKey(">", kShift);
					item = fileMenu->addEntry(new CCommandMenuItem({ "Decrease Width", this, "File", "Decrease Width" }));
					item->setKey("<", kShift);
					item = fileMenu->addEntry(new CCommandMenuItem({ "Increase Height", this, "File", "Increase Height" }));
					item->setKey(">", kShift | kAlt);
					item = fileMenu->addEntry(new CCommandMenuItem({ "Decrease Height", this, "File", "Decrease Height" }));
					item->setKey("<", kShift | kAlt);

				}

				// --- resize it (added WP)
				if (guiWindowFrame)
				{
					guiWindowFrame->setWindowFrameSize(rect.left, rect.top, rect.right, rect.bottom);
					guiWindowFrame->enableGUIDesigner(true);
				}

				return true;
			}
			editController->forget();
		}
		else
#endif
		{
			showGUIEditor = false;
			CView* view = description->createView(viewName.c_str(), this);
			if (view)
			{
				CCoord width = view->getWidth() * zoomFactor;
				CCoord height = view->getHeight() * zoomFactor;

				getFrame()->setSize(width, height);
				getFrame()->addView(view);
				getFrame()->setTransform(CGraphicsTransform().scale(zoomFactor, zoomFactor));
				//getFrame()->setZoom(zoomFactor);
				getFrame()->invalid();

				CRect rect(0, 0, 0, 0);
				rect.right = width;
				rect.bottom = height;

				// --- resize, in case coming back from editing
				if (guiWindowFrame)
				{
					guiWindowFrame->setWindowFrameSize(rect.left, rect.top, rect.right, rect.bottom);//->resizeView(this, &viewRect) == Steinberg::kResultTrue ? true : false;
					guiWindowFrame->enableGUIDesigner(false);
				}

				getFrame()->setFocusDrawingEnabled(false);
				return true;
			}
		}
	}
	return false;
}

/**
\brief save GUI state in XML file

Operation:\n
- shows the platform dependent File Save dialog box
- stores the current XML data in the user's .uidesc file
- NOTE: first time through the user ALWAYS must supply the path to the .uidesc file via Save As

\param saveAs show the save-as dialog box
*/
void PluginGUI::save(bool saveAs)
{
	if (!showGUIEditor) return;

	int32_t flags = 0;
#if VSTGUI_LIVE_EDITING
	UIEditController* editController = dynamic_cast<UIEditController*> (getViewController(frame->getView(0)));
	if (editController)
	{
		UIAttributes* attributes = editController->getSettings();
		bool val;
		if (attributes->getBooleanAttribute(UIEditController::kEncodeBitmapsSettingsKey, val) && val == true)
		{
			flags |= UIDescription::kWriteImagesIntoXMLFile;
		}
		if (attributes->getBooleanAttribute(UIEditController::kWriteWindowsRCFileSettingsKey, val) && val == true)
		{
			flags |= UIDescription::kWriteWindowsResourceFile;
		}
	}
#endif

	// --- the ASPiK Editor Attribute
	UIAttributes* attributes = description->getCustomAttributes("ASPiKEditor", true);
	if(!attributes) // --- this is for VERY old versions of pre-ASPiK
		attributes = description->getCustomAttributes("VST3Editor", true);
	vstgui_assert(attributes);
	const std::string* filePath = attributes->getAttributeValue("Path");
	if (!filePath) saveAs = true;
	if (filePath && filePath->empty()) saveAs = true;

	std::string savePath;

	if (saveAs)
	{
		CNewFileSelector* fileSelector = CNewFileSelector::create(frame, CNewFileSelector::kSelectSaveFile);
		if (fileSelector == 0)
			return;

		fileSelector->setTitle("Save UIDescription File");

		// --- NOTE: setDefaultExtension() causes a crash
		// fileSelector->setDefaultExtension(CFileExtension("VSTGUI UI Description", "uidesc"));
		if (filePath && !filePath->empty())
		{
			fileSelector->setInitialDirectory(filePath->c_str());
		}

		// --- only one option
		fileSelector->setDefaultSaveName("PluginGUI.uidesc");

		if (fileSelector->runModal())
		{
			UTF8StringPtr filePathSelected = fileSelector->getSelectedFile(0);
			if (filePathSelected)
			{
				attributes->setAttribute("Path", filePathSelected);
				attributes = description->getCustomAttributes("UIEditController", true);
				std::string editorSize = getGUIDesignerSize();
				attributes->setAttribute("EditorSize", editorSize.c_str());
				savePath = filePathSelected;
			}
		}
		fileSelector->forget();
	}
	else
	{
		std::string editorSize = getGUIDesignerSize();
		UIAttributes* attributes = description->getCustomAttributes("UIEditController", true);
		attributes->setAttribute("EditorSize", editorSize.c_str());

		if (filePath && !filePath->empty())
			savePath = *filePath;
	}
	if (savePath.empty())
		return;

	// --- save file
	description->save(savePath.c_str(), flags);
}

/**
\brief returns mode

Modes:\n
- kLinearMode: mouse moves vertically
- kRelativCircularMode: mouse moves in circle relative to control, as long as in control rect
- kCircularMode: mouse traces circular outline of knob

\return the knob mode
*/
int32_t PluginGUI::getKnobMode() const
{
	if (knobAction == kHostChoice)
		return kLinearMode; // DEFAULT MODE!
	else if (knobAction == kLinearMode)
		return kLinearMode;
	else if (knobAction == kRelativCircularMode)
		return kRelativCircularMode;
	else if (knobAction == kCircularMode)
		return kCircularMode;

	return kLinearMode;
}

/**
\brief get size string

\return GUI Designer size as a string for XML
*/
std::string PluginGUI::getGUIDesignerSize()
{
	std::string editorSize = "0,0,";

	std::stringstream strW;
	strW << guiDesignerWidth;
	editorSize.append((strW.str().c_str()));

	editorSize.append(",");

	std::stringstream strH;
	strH << guiDesignerHeight;
	editorSize.append((strH.str().c_str()));

	return editorSize;
}


/**
\brief control finder, for AAX only, part of Pro Tools automation keyboard shortcut

\param where mouse coordinates

\return control ID value of control under the mouse pointer
*/
int PluginGUI::getControlID_WithMouseCoords(const CPoint& where)
{
	for (ControlUpdateReceiverMap::const_iterator it = controlUpdateReceivers.begin(), end = controlUpdateReceivers.end(); it != end; ++it)
	{
		// it->second is receiver
		ControlUpdateReceiver* receiver = it->second;
		if (receiver)
		{
			int controlID = receiver->getControlID_WithMouseCoords(where);
			if (controlID != -1)
				return controlID;
			// else keep looking
		}
	}

	return -1;
}

/**
\brief control finder, for AAX only, part of Pro Tools automation keyboard shortcut

\param where mouse coordinates

\return pointer to control under the mouse 
*/
CControl* PluginGUI::getControl_WithMouseCoords(const CPoint& where)
{
	for (ControlUpdateReceiverMap::const_iterator it = controlUpdateReceivers.begin(), end = controlUpdateReceivers.end(); it != end; ++it)
	{
		// it->second is receiver
		ControlUpdateReceiver* receiver = it->second;
		if (receiver)
		{
			CControl* control = receiver->getControl_WithMouseCoords(where);
			if (control)
				return control;
			// else keep looking
		}
	}

	return nullptr;
}

/**
\brief mouse down handler for Pro Tools keyboard shortcuts

Operation:\n
- This handles several ProTools-specific mouse events
- ctrl-alt-cmd: enable automation
- alt: set control to default value (VSTGUI uses ctrl and this overrides it)

\param controlID ID tag for control that will receive mouse down message
\param buttons button state flags

\return handled or not handled
*/
CMouseEventResult PluginGUI::sendAAXMouseDown(int controlID, const CButtonState& buttons)
{
#ifdef AAXPLUGIN
	VSTGUI::CMouseEventResult result = VSTGUI::kMouseEventNotHandled;
	const AAX_CVSTGUIButtonState aax_buttons(buttons, aaxViewContainer);

	if (aaxViewContainer && controlID >= 0)
	{
		std::stringstream str;
		str << controlID + 1;

		if (aaxViewContainer->HandleParameterMouseDown(str.str().c_str(), aax_buttons.AsAAX()) == AAX_SUCCESS)
		{
			result = kMouseEventHandled;
		}
	}
	return result;

#endif
	return kMouseEventNotHandled;
}

/**
\brief mouse moved handler for Pro Tools keyboard shortcuts

Operation:\n
- This handles ProTools mouse move event for automation

\param controlID ID tag for control that will receive mouse move message
\param buttons button state flags
\param where the mouse coordinates

\return handled or not handled
*/
CMouseEventResult PluginGUI::sendAAXMouseMoved(int controlID, const CButtonState& buttons, const CPoint& where)
{
#ifdef AAXPLUGIN
	VSTGUI::CMouseEventResult result = VSTGUI::kMouseEventNotHandled;
	const AAX_CVSTGUIButtonState aax_buttons(buttons, aaxViewContainer);

	if (aaxViewContainer && controlID >= 0)
	{
		std::stringstream str;
		str << controlID + 1;

		if (aaxViewContainer->HandleParameterMouseDrag(str.str().c_str(), aax_buttons.AsAAX()) == AAX_SUCCESS)
		{
			result = kMouseEventHandled;
		}
	}

	return result;

#endif
	return kMouseEventNotHandled;
}

/**
\brief mouse down handler for AU automation

Operation:\n
- This handles AU mouse events
- option: go to default value

\param control control that receives message
\param buttons button state flags

\return handled or not handled
*/
CMouseEventResult PluginGUI::sendAUMouseDown(CControl* control, const CButtonState& buttons)
{
	if (!control) return kMouseEventNotHandled;

#ifdef AUPLUGIN
	VSTGUI::CMouseEventResult result = VSTGUI::kMouseEventNotHandled;

	if (buttons.isLeftButton() && (buttons & kAlt))
	{
		CButtonState newState(kLButton | kControl);
		control->checkDefaultValue(newState);
		result = kMouseEventHandled;
	}
	else if (buttons.isLeftButton() && (buttons & kControl))
	{
		result = kMouseEventNotImplemented;
	}
	return result;

#endif
	return kMouseEventNotHandled;
}


#ifdef AAXPLUGIN
/**
\brief set the plugin shelll parameter from GUI control using thread-safe mechanism

\param control the GUI control issuing the change
\param tag the control ID
\param actualValue parameter value in actual (plain) form
\param normalizedValue parameter value in normalized form
*/
void PluginGUI::setAAXParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue)
{
	// --- notification for custom GUI updates & safe parameter set()
	if (guiPluginConnector)
	{
		guiPluginConnector->parameterChanged(tag, actualValue, normalizedValue);
		guiPluginConnector->setNormalizedPluginParameter(tag, normalizedValue);
	}
}

/**
\brief set the GUI control from the plugin shell GUI object (thread-safe)

\param tag the control ID
\param actualPluginValue parameter value in actual (plain) form
\param normalizedValue parameter value in normalized form
\param useNormalized force normalized value
*/
void PluginGUI::updateGUIControlAAX(int tag, float actualPluginValue, float normalizedValue, bool useNormalized)
{
	ControlUpdateReceiver* receiver = getControlUpdateReceiver(tag);
	if (receiver)
	{
		// --- internal calls to this need the normalized values (thread safety)
		if (useNormalized)
			receiver->updateControlsWithNormalizedValue(normalizedValue);
		else
			receiver->updateControlsWithActualValue(actualPluginValue); // external updates use actual value
	}
}

#endif


#ifdef AUPLUGIN

/**
\brief set the GUI control from the AU event generator; this is part of the thread-safe event system.
	   This gets called when automation is running.

\param tag the control ID
\param actualPluginValue parameter value in actual (plain) form
\param message not used
\param fromEventListener not used
*/
void PluginGUI::dispatchAUControlChange(int tag, float actualPluginValue, int message, bool fromEventListener)//, bool checkUpdateGUI)
{
	ControlUpdateReceiver* receiver = getControlUpdateReceiver(tag);
	if (receiver)
		receiver->updateControlsWithActualValue(actualPluginValue);
}

/**
\brief set the AU event from the GUI congrol; this is part of the thread-safe event system.
This is called to safely set the GUI control value on the plugin shell's parameter list.

\param control the control issuing the change
\param tag the control ID
\param actualValue actual control value (AU is always actual value)
*/
void PluginGUI::setAUEventFromGUIControl(CControl* control, int tag, float actualValue)
{
	// --- notification only for custom GUI updates
	if (guiPluginConnector)
		guiPluginConnector->parameterChanged(tag, actualValue, actualValue);

	ControlUpdateReceiver* receiver = getControlUpdateReceiver(tag);
	if (!receiver) return;

	if (getGuiControlWithTag(tag))
	{
		AudioUnitParameter param = { m_AU, static_cast<AudioUnitParameterID>(tag), kAudioUnitScope_Global, 0 };
		AUParameterSet(AUEventListener, control, &param, actualValue, 0);
	}
}
#endif

#ifdef VSTPLUGIN

/**
\brief set the VST parameter from the GUI control

\param control the control issuing the change
\param tag the control ID
\param actualValue actual control value 
\param normalizedValue normalized control value
*/
void PluginGUI::setVSTParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue)
{
	// --- notification parameter sync and for custom GUI updates
	if (guiPluginConnector)
		guiPluginConnector->parameterChanged(tag, actualValue, normalizedValue);
}

/**
\brief set the GUI control from the VST parameter

\param tag the control ID
\param normalizedValue normalized control value
*/
void PluginGUI::updateGUIControlVST(int tag, float normalizedValue)
{
	ControlUpdateReceiver* receiver = getControlUpdateReceiver(tag);
	if (receiver)
	{
		receiver->updateControlsWithNormalizedValue(normalizedValue);
	}
}

#endif

#ifdef RAFXPLUGIN
/**
\brief set the RAFX2 parameter from the GUI control

\param control the control issuing the change
\param tag the control ID
\param actualValue actual control value
\param normalizedValue normalized control value
*/
void PluginGUI::setRAFXParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue)
{
	// --- notification parameter sync and for custom GUI updates
	if (guiPluginConnector)
		guiPluginConnector->parameterChanged(tag, actualValue, normalizedValue);
}

/**
\brief set the GUI control from the RAFX2 parameter

\param tag the control ID
\param normalizedValue normalized control value
*/
void PluginGUI::updateGUIControlRAFX(int tag, float normalizedValue)
{
	ControlUpdateReceiver* receiver = getControlUpdateReceiver(tag);
	if (receiver)
	{
		receiver->updateControlsWithNormalizedValue(normalizedValue);
	}
}
#endif


/**
\brief incoming VSTGUI4 message handler 

\param sender pointer to sending object
\param message message as a string
\result notified or not
*/
CMessageResult PluginGUI::notify(CBaseObject* sender, IdStringPtr message)
{
	if(message == CVSTGUITimer::kMsgTimer)
	{
		if(createNewView)
			createGUI(showGUIEditor);

        // --- timer ping for control updates
        if(frame)
          idle();
	}

	return kMessageNotified;
}

/**
\brief THE function that all controls pour their control changes into. The result of this function
	   is to push the GUI control change safely into the plugin shell owner.

Operation:\n
- check control for a built-in feature: GUI scaling or Preset File writing
- check xy-pad as these are double-parameter objects
- find the control reciever: update all connected controls (controls with same ID tag)
- issue the control change message to the shell (threa-safe and API-specific) 

\param pControl control that issues change notification
*/
void PluginGUI::valueChanged(VSTGUI::CControl* pControl)
{
	// --- handle changes not bound to plugin variables (tab controls, joystick)
	if (guiPluginConnector && guiPluginConnector->checkNonBoundValueChange(pControl->getTag(), pControl->getValueNormalized()))
		return;

	float actualValue = 0.f;

	// --- check default controls
	if (pControl->getTag() == SCALE_GUI_SIZE)
	{
		// --- scale the GUI
		scaleGUISize((uint32_t)pControl->getValue());

		// --- update variable for persistence
		ControlUpdateReceiver* receiver = getControlUpdateReceiver(pControl->getTag());
		float normalizedValue = pControl->getValueNormalized();
		if (receiver)
		{
			// --- this CONTROL update uses the control normalized value, no taper
			receiver->updateControlsWithNormalizedValue(pControl->getValueNormalized(), pControl);
			actualValue = receiver->getActualValueWithNormalizedValue(pControl->getValueNormalized());
			setPluginParameterFromGUIControl(pControl, pControl->getTag(), actualValue, normalizedValue);
		}
		return;
	}

	// --- save preset code for easy pasting into non-RackAFX projects
	if (pControl->getTag() == WRITE_PRESET_FILE)
	{
		// --- save it
		writeToPresetFile();

		// --- reset button (file open dialog in front of it can cause graphic to stick on Windows)
		pControl->setValue(0.0);

		return;
	}

	// --- check for trackpad and joystick
	CXYPadEx* xyPad = dynamic_cast<CXYPadEx*>(pControl);
	if (xyPad)
	{
		float x = 0.f;
		float y = 0.f;
		xyPad->calculateXY(pControl->getValue(), x, y);

		ControlUpdateReceiver* receiver = getControlUpdateReceiver(xyPad->getTagX());
		if (receiver)
		{
			receiver->updateControlsWithNormalizedValue(x, pControl);
			actualValue = receiver->getActualValueWithNormalizedValue(x);
		}

		setPluginParameterFromGUIControl(pControl, xyPad->getTagX(), actualValue, x);

		receiver = getControlUpdateReceiver(xyPad->getTagY());
		if (receiver)
		{
			receiver->updateControlsWithNormalizedValue(y, pControl);
			actualValue = receiver->getActualValueWithNormalizedValue(y);
		}

		setPluginParameterFromGUIControl(pControl, xyPad->getTagY(), actualValue, y);
		return;
	}

	ControlUpdateReceiver* receiver = getControlUpdateReceiver(pControl->getTag());
	float normalizedValue = pControl->getValueNormalized();

	if (receiver)
	{
        float num = 0.0;

		// --- need to handle labels (or edits) differently
		CTextLabel* label = dynamic_cast<CTextLabel*>(pControl);
		if (label)
		{
            // --- the edit string MUST be numeric for ASPiK parameters
            //     you can use a custom view if you need edit controls for text strings
			std::string strLabel(label->getText());
            std::stringstream ss;
            ss << strLabel;
            ss >> num;
            if(num == 0.0 && strLabel[0] != '0')
            {
                PluginParameter refGuiControl = receiver->getGuiControl();
                label->setText(refGuiControl.getControlValueAsString().c_str());
                return;
            }
            
            // --- continue tests
			actualValue = stof(strLabel);
            
            // --- is value out of bounds?
            PluginParameter refGuiControl = receiver->getGuiControl();
            actualValue = fmin(actualValue, refGuiControl.getMaxValue());
            actualValue = fmax(actualValue, refGuiControl.getMinValue());

			// --- update
			receiver->updateControlsWithActualValue(actualValue, label);

			// --- add units string to edit display
            refGuiControl = receiver->getGuiControl(); // needs to be done again to get updated value
			label->setText(refGuiControl.getControlValueAsString().c_str());
            label->invalid();
            
			// --- get the normalized value, no taper
			normalizedValue = refGuiControl.getControlValueNormalized();
		}
		else
		{

			// --- this CONTROL update uses the control normalized value, no taper
			//  receiver->updateControlsWithNormalizedValue(pControl->getValueNormalized(), pControl);
			receiver->updateControlsWithNormalizedValue(normalizedValue, pControl);

			// --- get the actual value, with tapers applied
			//  actualValue = receiver->getActualValueWithNormalizedValue(pControl->getValueNormalized());
			actualValue = receiver->getActualValueWithNormalizedValue(normalizedValue);

#ifndef VSTPLUGIN // vst needs the control's normalized value, without taper

			// --- get the normalized value, with tapers applied (nned RAFX?!!?)
			normalizedValue = receiver->getNormalizedValueWithActualValue(actualValue);
#endif
		}
	}

	setPluginParameterFromGUIControl(pControl, pControl->getTag(), actualValue, normalizedValue);
}

/**
\brief start of control/auomation notification

NOTE:\n
- this function implementation is different for each API
- the begin-autamation commmand is issued to the varisou APIs that require it

\param pControl control that has started the edit
*/
void PluginGUI::controlBeginEdit(VSTGUI::CControl* pControl)
{

#ifdef AAXPLUGIN
	if (guiPluginConnector)
		guiPluginConnector->beginParameterChangeGesture(pControl->getTag());
#endif

#ifdef AUPLUGIN
	// --- automation handlers
	AudioUnitEvent auEvent;
	auEvent.mArgument.mParameter.mAudioUnit = m_AU;
	auEvent.mArgument.mParameter.mParameterID = pControl->getTag();
	auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
	auEvent.mArgument.mParameter.mElement = 0;
	auEvent.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
	AUEventListenerNotify(AUEventListener, NULL, &auEvent);
#endif

	return;
}

/**
\brief end of control/auomation notification

NOTE:\n
- this function implementation is different for each API
- the end-autamation commmand is issued to the varisou APIs that require it

\param pControl control that has started the edit
*/
void PluginGUI::controlEndEdit(VSTGUI::CControl* pControl)//;// {}
{

#ifdef AAXPLUGIN
	if (guiPluginConnector)
		guiPluginConnector->endParameterChangeGesture(pControl->getTag());
#endif

#ifdef AUPLUGIN
	// --- automation handlers
	AudioUnitEvent auEvent;
	auEvent.mArgument.mParameter.mAudioUnit = m_AU;
	auEvent.mArgument.mParameter.mParameterID = pControl->getTag();

	auEvent.mArgument.mParameter.mScope = kAudioUnitScope_Global;
	auEvent.mArgument.mParameter.mElement = 0;
	auEvent.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
	AUEventListenerNotify(AUEventListener, NULL, &auEvent);
#endif

	return;
}

/**
\brief called when a control is being removed from the GUI, or when it is being created (step 1)

NOTE:\n
- remove the control from receiver list
- check and remove from writeable (meter) controls

\param pControl control whose tag will change
*/
void PluginGUI::controlTagWillChange(VSTGUI::CControl* pControl)
{
	if (pControl->getTag() != -1 && pControl->getListener() == this)
	{
		ControlUpdateReceiver* receiver = getControlUpdateReceiver(pControl->getTag());
		if (receiver)
		{
			receiver->removeControl(pControl);
			checkRemoveWriteableControl(pControl);
		}
	}

	return;
}

/**
\brief called when a control is being created on the GUI (step 2)

NOTE:\n
- add the control to our receiver list
- deal with XY pads that have two parameters

\param pControl control whose tag has changed
*/
void PluginGUI::controlTagDidChange(VSTGUI::CControl* pControl)
{
	if (pControl->getTag() == -1 || pControl->getListener() != this)
		return;

	if (pControl->getTag() == XY_TRACKPAD)
	{
		bool transmitter = pControl->getListener() == this ? true : false;

		// --- check for trackpad and joystick
		CXYPadEx* xyPad = dynamic_cast<CXYPadEx*>(pControl);
		if (xyPad)
		{
			// --- retrieve the X and Y tags on the xyPad CView
			int32_t tagX = xyPad->getTagX();
			int32_t tagY = xyPad->getTagY();

			if (tagX != -1)
			{
				ControlUpdateReceiver* receiver = getControlUpdateReceiver(tagX);
				PluginParameter* ctrl = getGuiControlWithTag(tagX);
				checkAddWriteableControl(ctrl, pControl);

				if (receiver)
				{
					receiver->addControl(pControl);
					syncGUIControl(tagX);
				}
				else
				{
					ControlUpdateReceiver* receiver = new ControlUpdateReceiver(pControl, ctrl, transmitter);
					controlUpdateReceivers.insert(std::make_pair(tagX, receiver));
					syncGUIControl(tagX);
				}
			}

			if (tagY != -1)
			{
				ControlUpdateReceiver* receiver = getControlUpdateReceiver(tagY);
				PluginParameter* ctrl = getGuiControlWithTag(tagY);
				checkAddWriteableControl(ctrl, pControl);

				if (receiver)
				{
					receiver->addControl(pControl);
					syncGUIControl(tagY);
				}
				else
				{
					ControlUpdateReceiver* receiver = new ControlUpdateReceiver(pControl, ctrl, transmitter);
					controlUpdateReceivers.insert(std::make_pair(tagY, receiver));
					syncGUIControl(tagY);
				}
			}
			return;
		}
	}

	if (pControl->getTag() == -1) return;
	bool transmitter = pControl->getListener() == this ? true : false;

	ControlUpdateReceiver* receiver = getControlUpdateReceiver(pControl->getTag());
	PluginParameter* ctrl = getGuiControlWithTag(pControl->getTag());
	checkAddWriteableControl(ctrl, pControl);

	if (receiver)
	{
		receiver->addControl(pControl);
		syncGUIControl(pControl->getTag());
	}
	else
	{
		// --- ctrl may be NULL for tab controls and other non-variable linked items
		ControlUpdateReceiver* receiver = new ControlUpdateReceiver(pControl, ctrl, transmitter);
		controlUpdateReceivers.insert(std::make_pair(pControl->getTag(), receiver));
		syncGUIControl(pControl->getTag());
	}
	return;
}


/**
\brief add your custom views here; this is where you can create and register the views outside of 
the createView( ) method below.

Operation:\n
- decode the view name string
- use the new operator to create an instance of your custom view here.

\param viewname customm view name string (as defined in XML file)
\param rect size of control
\param listener parent listener
\param tag the control ID value
*/
CView* PluginGUI::createUserCustomView(std::string viewname, const CRect rect, IControlListener* listener, int32_t tag)
{
	if (viewname.compare("CustomWaveView") == 0)
	{
		// --- create our custom view
		return new WaveView(rect, listener, tag);
	}

	if (viewname.find("ASPiK_DynamicMenu") != std::string::npos)
	{
		// --- create our custom view
		return new CustomOptionMenu(rect, listener, tag);
	}

	if (viewname.find("ASPiK_DynamicLabel") != std::string::npos)
	{
		// --- create our custom view
		return new CustomTextLabel(rect);
	}

	if (viewname.compare("CustomSpectrumView") == 0)
	{
#ifdef HAVE_FFTW
		// --- create our custom view
		return new SpectrumView(rect, listener, tag);
#else
		return new WaveView(rect, listener, tag);
#endif
	}

	return nullptr;
}

/**
\brief this is called for every view obeject in the XML file that will be visible to check and see
	   if you want to supply your own object, that you create with the new operator; that object
	   takes the place of the stock GUI object, allowing you to customize the view behavior. This 
	   is called the "Custom View" paradigm and along with the sub-controller make VSTGUI4 extremely
	   powerful. 

Operation:\n
- decode the view name string
- use the new operator to create an instance of the custom view
- the views here are all built-in to ASPiK plugins!

\param viewname customm view name string (as defined in XML file)
\param rect size of control
\param listener parent listener
\param tag the control ID value
*/
CView* PluginGUI::createView(const UIAttributes& attributes, const IUIDescription* description)
{
    const std::string* customViewName = attributes.getAttributeValue(IUIDescription::kCustomViewName);
    if (!customViewName) return nullptr;
    const std::string viewname(customViewName->c_str());
    if (viewname.empty()) return nullptr;
    
    // --- USER CUSTOM VIEWS
    //
    if (ENABLE_CUSTOM_VIEWS)
    {
        // --- our wave view testing object
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        if (originString && sizeString)
        {
            // --- create the rect
            CPoint origin;
            CPoint size;
            parseSize(*sizeString, size);
            parseSize(*originString, origin);
            
            const CRect rect(origin, size);
            
            int32_t tag = -1;
            IControlListener* listener = this;
            
            // --- get tag
            if (tagString)
            {
                tag = description->getTagForName(tagString->c_str());
                listener = description->getControlListener(tagString->c_str());
            }
            
            // --- try a user view first
            CView* userCV = createUserCustomView(viewname, rect, listener, tag);
            if (userCV)
            {
                // --- if the view has the ICustomView interface, we register it with the plugin for updates
                if (hasICustomView(userCV))
                {
                    if (guiPluginConnector)
                        guiPluginConnector->registerCustomView(viewname, dynamic_cast<ICustomView*>(userCV));
                }
                
                return userCV;
            }
        }
        
        // --- else keep trying other custom views (below)
    }
    
    
    // --- example of custom control
    if (viewname == "CustomKnobView")
    {
        // --- our wave view testing object
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* offsetString = attributes.getAttributeValue("background-offset");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        const std::string* bitmapString = attributes.getAttributeValue("bitmap");
        const std::string* heightOneImageString = attributes.getAttributeValue("height-of-one-image");
        const std::string* subPixmapsString = attributes.getAttributeValue("sub-pixmaps");
        if (!sizeString) return nullptr;
        if (!originString) return nullptr;
       // if (!offsetString) return nullptr;
        if (!tagString) return nullptr;
        if (!bitmapString) return nullptr;
        if (!heightOneImageString) return nullptr;
        if (!subPixmapsString) return nullptr;
        
        // --- create the rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- get listener
        IControlListener* listener = description->getControlListener(tagString->c_str());
        
        // --- get tag
        int32_t tag = description->getTagForName(tagString->c_str());
        
        // --- subPixmaps
        int32_t subPixmaps = strtol(subPixmapsString->c_str(), 0, 10);
        
        // --- height of one image
        CCoord heightOfOneImage = strtod(heightOneImageString->c_str(), 0);
        
        // --- bitmap
        std::string BMString = *bitmapString;
        BMString += ".png";
        UTF8StringPtr bmp = BMString.c_str();
        CResourceDescription bmpRes(bmp);
        CBitmap* pBMP = new CBitmap(bmpRes);
        
        // --- offset
        CPoint offset(0.0, 0.0);
        if (offsetString)
            parseSize(*offsetString, offset);
        
        const CPoint offsetPoint(offset);
        
        PluginParameter* piParam = getGuiControlWithTag(tag);
        if (!piParam)
        {
            if (pBMP) pBMP->forget();
            return nullptr;
        }
        
        CustomKnobView* customKnob = new CustomKnobView(rect, listener, tag, subPixmaps, heightOfOneImage, pBMP, offsetPoint, false);
        
        // --- if the view has the ICustomView interface, we register it with the plugin for updates
        if (hasICustomView(customKnob))
        {
            if (guiPluginConnector)
                guiPluginConnector->registerCustomView(viewname, (ICustomView*)customKnob);
        }
        
        if (pBMP) pBMP->forget();
        
        return customKnob;
    }
    
    // --- BUILT-IN CUSTOM VIEWS
    int nTP = (int)viewname.find("TrackPad_");
    if(nTP >= 0)
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
        
        // --- rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- decoding code
        int x = (int)viewname.find("_X");
        int y = (int)viewname.find("_Y");
        int len = (int)viewname.length();
        
        if(x < 0 || y < 0 || len < 0)
            return nullptr;
        
        if(x < y && y < len)
        {
            std::string strX = viewname.substr(x + 2, y - 2 - x);
            std::string strY = viewname.substr(y + 2, len - 2 - y);
            int32_t _tagX = atoi(strX.c_str());
            int32_t _tagY = atoi(strY.c_str());
            
            CXYPadEx* p = new CXYPadEx(rect);
            if(!p) return nullptr;
            
            // --- save tags
            p->setTagX(_tagX);
            p->setTagY(_tagY);
            
            return p;
        }
    }
    
    if (viewname == "CustomKickButton" ||
        viewname == "CustomKickButtonDU" ||
        viewname == "CustomKickButtonU" ||
        viewname == "CustomKickButtonD")
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* bitmapString = attributes.getAttributeValue("bitmap");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        const std::string* offsetString = attributes.getAttributeValue("background-offset");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
        if(!bitmapString) return nullptr;
        if(!tagString) return nullptr;
      //  if(!offsetString) return nullptr;
        
        // --- rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- tag
        int32_t tag = description->getTagForName(tagString->c_str());
        
        // --- listener "hears" the control
        const char* controlTagName = tagString->c_str();
        IControlListener* listener = description->getControlListener(controlTagName);
        
        // --- bitmap
        std::string BMString = *bitmapString;
        BMString += ".png";
        UTF8StringPtr bmp = BMString.c_str();
        CResourceDescription bmpRes(bmp);
        CBitmap* pBMP = new CBitmap(bmpRes);
        
        // --- offset
        CPoint offset(0.0, 0.0);
        if (offsetString)
            parseSize(*offsetString, offset);
        
        const CPoint offsetPoint(offset);
      
        CKickButtonEx* p = new CKickButtonEx(rect, listener, tag, pBMP, offsetPoint);
        if (p)
        {
            if (viewname == "CustomKickButtonDU")
                p->setMouseMode(mouseDirUpAndDown);
            else if (viewname == "CustomKickButtonD")
                p->setMouseMode(mouseDirDown);
            else if (viewname == "CustomKickButtonU")
                p->setMouseMode(mouseDirUp);
            else
                p->setMouseMode(mouseDirUp); // old
        }
        if (pBMP) pBMP->forget();
        
        return p;
    }
    
    if (viewname == "CustomTextButton" ||
        viewname == "CustomTextButtonDU" ||
        viewname == "CustomTextButtonU" ||
        viewname == "CustomTextButtonD")
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        const std::string* titleString = attributes.getAttributeValue("title");
        if (!sizeString) return nullptr;
        if (!originString) return nullptr;
        if (!tagString) return nullptr;
        if (!titleString) return nullptr;
        
        // --- rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- tag
        int32_t tag = description->getTagForName(tagString->c_str());
        
        // --- listener "hears" the control
        const char* controlTagName = tagString->c_str();
        IControlListener* listener = description->getControlListener(controlTagName);
        
        TextButtonEx* p = new TextButtonEx(rect, listener, tag, titleString->c_str(), CTextButton::Style::kKickStyle);
        if (p)
        {
            if (viewname == "CustomTextButtonDU")
                p->setMouseMode(mouseDirUpAndDown);
            else if (viewname == "CustomTextButtonD")
                p->setMouseMode(mouseDirDown);
            else if (viewname == "CustomTextButtonU")
                p->setMouseMode(mouseDirUp);
            else
                p->setMouseMode(mouseDirUp); // old
        }
        return p;
    }
    
    
    // --- special knobs
    if (viewname == "KnobSwitchView" || viewname == "UniversalAPIKnob")
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* offsetString = attributes.getAttributeValue("background-offset");
        const std::string* bitmapString = attributes.getAttributeValue("bitmap");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        const std::string* heightOneImageString = attributes.getAttributeValue("height-of-one-image");
        const std::string* subPixmapsString = attributes.getAttributeValue("sub-pixmaps");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
       // if(!offsetString) return nullptr;
        if(!bitmapString) return nullptr;
        if(!tagString) return nullptr;
        if(!heightOneImageString) return nullptr;
        if(!subPixmapsString) return nullptr;
        
        bool isSwitchControl = viewname == "KnobSwitchView" ? true : false;
        bool isUniversalAPIControl = viewname == "UniversalAPIKnob" ? true : false;
        if (isUniversalAPIControl)
            int eliminatecompilerwarning = 1;
        
        // --- rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- listener "hears" the control
        const char* controlTagName = tagString->c_str();
        IControlListener* listener = description->getControlListener(controlTagName);
        
        // --- tag
        int32_t tag = description->getTagForName(tagString->c_str());
        
        // --- subPixmaps
        int32_t subPixmaps = strtol(subPixmapsString->c_str(), 0, 10);
        
        // --- height of one image
        CCoord heightOfOneImage = strtod(heightOneImageString->c_str(), 0);
        
        // --- bitmap
        std::string BMString = *bitmapString;
        BMString += ".png";
        UTF8StringPtr bmp = BMString.c_str();
        CResourceDescription bmpRes(bmp);
        CBitmap* pBMP = new CBitmap(bmpRes);
        
        // --- offset
        // --- offset
        CPoint offset(0.0, 0.0);
        if (offsetString)
            parseSize(*offsetString, offset);

        const CPoint offsetPoint(offset);
        
        PluginParameter* piParam = getGuiControlWithTag(tag);
        if (!piParam)
        {
            if (pBMP) pBMP->forget();
            return nullptr;
        }
        
        // --- the knobswitch; more well behaved than the VSTGUI4 object IMO
        CAnimKnobEx* p = new CAnimKnobEx(rect, listener, tag, subPixmaps, heightOfOneImage, pBMP, offsetPoint, isSwitchControl);
        if(isSwitchControl)
            p->setSwitchMax((float)piParam->getMaxValue());
        
#ifdef AAXPLUGIN
        if(isUniversalAPIControl)
            p->setAAXKnob(true);
#endif
        if (pBMP) pBMP->forget();
        return p;
    }
    
    // --- special sliders
    if (viewname == "UniversalAPISlider")
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* offsetString = attributes.getAttributeValue("handle-offset");
        const std::string* bitmapString = attributes.getAttributeValue("bitmap");
        const std::string* handleBitmapString = attributes.getAttributeValue("handle-bitmap");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        const std::string* styleString = attributes.getAttributeValue("orientation");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
       // if(!offsetString) return nullptr;
        if(!bitmapString) return nullptr;
        if(!handleBitmapString) return nullptr;
        if(!tagString) return nullptr;
        if(!styleString) return nullptr;
        
        bool isUniversalAPIControl = viewname == "UniversalAPISlider" ? true : false;
        if(isUniversalAPIControl)
            int eliminatecompilerwarning = 1;
        
        // --- rect
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        // --- listener
        const char* controlTagName = tagString->c_str();
        IControlListener* listener = description->getControlListener(controlTagName);
        
        // --- tag
        int32_t tag = description->getTagForName(tagString->c_str());
        
        // --- bitmap
        std::string BMString = *bitmapString;
        BMString += ".png";
        UTF8StringPtr bmp = BMString.c_str();
        CResourceDescription bmpRes(bmp);
        CBitmap* pBMP_back = new CBitmap(bmpRes);
        
        std::string BMStringH = *handleBitmapString;
        BMStringH += ".png";
        UTF8StringPtr bmpH = BMStringH.c_str();
        CResourceDescription bmpResH(bmpH);
        CBitmap* pBMP_hand = new CBitmap(bmpResH);
        
        // --- offset
        CPoint offset(0.0, 0.0);
        if (offsetString)
            parseSize(*offsetString, offset);

        const CPoint offsetPoint(offset);
        
        PluginParameter* piParam = getGuiControlWithTag(tag);
        if (!piParam) return nullptr;
        
        // --- the knobswitch
        if (strcmp(styleString->c_str(), "vertical") == 0)
        {
            CVerticalSliderEx* p = new CVerticalSliderEx(rect, listener, tag, 0, 1, pBMP_hand, pBMP_back, offsetPoint);
#ifdef AAXPLUGIN
            if(isUniversalAPIControl)
                p->setAAXSlider(true);
#endif
            if (pBMP_back) pBMP_back->forget();
            if (pBMP_hand) pBMP_hand->forget();
            return p;
        }
        else
        {
            CHorizontalSliderEx* p = new CHorizontalSliderEx(rect, listener, tag, 0, 1, pBMP_hand, pBMP_back, offsetPoint);
#ifdef AAXPLUGIN
            if(isUniversalAPIControl)
                p->setAAXSlider(true);
#endif
            if (pBMP_back) pBMP_back->forget();
            if (pBMP_hand) pBMP_hand->forget();
            return p;
        }
        return nullptr;
    }
    
    // --- meters
    std::string customView(viewname);
    std::string analogMeter("AnalogMeterView");
    std::string invAnalogMeter("InvertedAnalogMeterView");
    int nAnalogMeter = (int)customView.find(analogMeter);
    int nInvertedAnalogMeter = (int)customView.find(invAnalogMeter);
    
    if (nAnalogMeter >= 0)
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* ONbitmapString = attributes.getAttributeValue("bitmap");
        const std::string* OFFbitmapString = attributes.getAttributeValue("off-bitmap");
        const std::string* numLEDString = attributes.getAttributeValue("num-led");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
        if(!ONbitmapString) return nullptr;
        if(!OFFbitmapString) return nullptr;
        if(!numLEDString) return nullptr;
        if(!tagString) return nullptr;
        
        
        CPoint origin;
        CPoint size;
        parseSize(*sizeString, size);
        parseSize(*originString, origin);
        
        const CRect rect(origin, size);
        
        std::string onBMString = *ONbitmapString;
        onBMString += ".png";
        UTF8StringPtr onbmp = onBMString.c_str();
        CResourceDescription bmpRes(onbmp);
        CBitmap* onBMP = new CBitmap(bmpRes);
        
        std::string offBMString = *OFFbitmapString;
        offBMString += ".png";
        UTF8StringPtr offbmp = offBMString.c_str();
        CResourceDescription bmpRes2(offbmp);
        CBitmap* offBMP = new CBitmap(bmpRes2);
        
        int32_t nbLed = strtol(numLEDString->c_str(), 0, 10);
        
        CVuMeterEx* p = nullptr;
        
        if (nInvertedAnalogMeter >= 0)
            p = new CVuMeterEx(rect, onBMP, offBMP, nbLed, true, true); // inverted, analog
        else
            p = new CVuMeterEx(rect, onBMP, offBMP, nbLed, false, true); // inverted, analog
        
        // --- decode our stashed variables
        // decode hieght one image and zero db frame
        int nX = (int)customView.find("_H");
        int nY = (int)customView.find("_Z");
        int len = (int)customView.length();
        std::string sH = customView.substr(nX + 2, nY - 2 - nX);
        std::string sZ = customView.substr(nY + 2, len - 2 - nY);
        
        p->setHtOneImage(atof(sH.c_str()));
        p->setImageCount(atof(numLEDString->c_str()));
        p->setZero_dB_Frame(atof(sZ.c_str()));
        
        if (onBMP) onBMP->forget();
        if (offBMP) offBMP->forget();
        
        // --- connect meters/variables
        int32_t tag = description->getTagForName(tagString->c_str());
        
        PluginParameter* piParam = getGuiControlWithTag(tag);
        if (!piParam) return nullptr;
        
        // --- set detector
        float fSampleRate = 1.f / (GUI_METER_UPDATE_INTERVAL_MSEC*0.001f);
        p->initDetector(fSampleRate, (float)piParam->getMeterAttack_ms(),
                        (float)piParam->getMeterRelease_ms(), true,
                        piParam->getDetectorMode(),
                        piParam->getLogMeter());
        
        return p;
    }
    
    if (viewname == "InvertedMeterView" || viewname == "MeterView")
    {
        const std::string* sizeString = attributes.getAttributeValue("size");
        const std::string* originString = attributes.getAttributeValue("origin");
        const std::string* ONbitmapString = attributes.getAttributeValue("bitmap");
        const std::string* OFFbitmapString = attributes.getAttributeValue("off-bitmap");
        const std::string* numLEDString = attributes.getAttributeValue("num-led");
        const std::string* tagString = attributes.getAttributeValue("control-tag");
        if(!sizeString) return nullptr;
        if(!originString) return nullptr;
        if(!ONbitmapString) return nullptr;
        if(!OFFbitmapString) return nullptr;
        if(!numLEDString) return nullptr;
        if(!tagString) return nullptr;
        
        if (sizeString && originString && ONbitmapString && OFFbitmapString && numLEDString)
        {
            CPoint origin;
            CPoint size;
            parseSize(*sizeString, size);
            parseSize(*originString, origin);
            
            const CRect rect(origin, size);
            
            std::string onBMString = *ONbitmapString;
            onBMString += ".png";
            UTF8StringPtr onbmp = onBMString.c_str();
            CResourceDescription bmpRes(onbmp);
            CBitmap* onBMP = new CBitmap(bmpRes);
            
            std::string offBMString = *OFFbitmapString;
            offBMString += ".png";
            UTF8StringPtr offbmp = offBMString.c_str();
            CResourceDescription bmpRes2(offbmp);
            CBitmap* offBMP = new CBitmap(bmpRes2);
            
            int32_t nbLed = strtol(numLEDString->c_str(), 0, 10);
            
            bool bInverted = false;
            
            if (viewname == "InvertedMeterView")
                bInverted = true;
            
            CVuMeterEx* p = new CVuMeterEx(rect, onBMP, offBMP, nbLed, bInverted, false); // inverted, analog
            
            if (onBMP) onBMP->forget();
            if (offBMP) offBMP->forget();
            
            // --- connect meters/variables
            int32_t tag = description->getTagForName(tagString->c_str());
            
            PluginParameter* piParam = getGuiControlWithTag(tag);
            if (!piParam) return nullptr;
            
            // --- set detector
            float fSampleRate = 1.f / (GUI_METER_UPDATE_INTERVAL_MSEC*0.001f);
            p->initDetector(fSampleRate, (float)piParam->getMeterAttack_ms(),
                            (float)piParam->getMeterRelease_ms(), true,
                            piParam->getDetectorMode(),
                            piParam->getLogMeter());
            
            return p;
        }
    }
    
    return nullptr;
}
    

/**
\brief for advanced users: you can create and even register sub-controllers here

Operation:\n
- the example here is for a set of knobs that are linked together
- see www.willpirkle.com for more information

\param name sub-controller name string as defined in the XML file
\param description UIDescription of object

\return an IController* to sub-controller (all sub-controllers must expose this interface)
*/
IController* PluginGUI::createSubController(UTF8StringPtr name, const IUIDescription* description)
{
	std::string strName(name);
	int findIt = strName.find("KnobLinkController");
	if (findIt >= 0)
	{
		// --- create the sub-controller
		KnobLinkController* knobLinker = new KnobLinkController(this);

		// --- if the sub-controller has the ICustomView interface,
		//     we register it with the plugin for updates (unusual for a sub-controller)
		if (hasICustomView(knobLinker))
		{
			if (guiPluginConnector)
				guiPluginConnector->registerSubcontroller(strName, dynamic_cast<ICustomView*>(knobLinker));
		}

		return knobLinker;
	}

	return nullptr;
}


/**
\brief find the receiver for a given tag; there can be only one receiver for any tag

\param tag the control ID value

\return a const naked pointer to the receiver object
*/
ControlUpdateReceiver* PluginGUI::getControlUpdateReceiver(int32_t tag) const
{
	if (tag != -1)
	{
		ControlUpdateReceiverMap::const_iterator it = controlUpdateReceivers.find(tag);
		if (it != controlUpdateReceivers.end())
		{
			return it->second;
		}
	}
	return 0;
}

/**
\brief called before GUI control is removed from the view

Operation:\n
- de-register custom views
- remove control from our lists

\param frame owner frame
\param view pointer to view being removed
*/
void PluginGUI::onViewRemoved(CFrame* frame, CView* view)
{
	// --- check to de-register a custom view
	if (hasICustomView(view) && guiPluginConnector)
		guiPluginConnector->deRegisterCustomView(dynamic_cast<ICustomView*>(view));

	CControl* control = dynamic_cast<CControl*> (view);
	if (control && control->getTag() != -1)
	{
		// --- check for trackpad and joystick
		CXYPadEx* xyPad = dynamic_cast<CXYPadEx*>(control);
		if (xyPad)
		{
			// --- retrieve the X and Y tags on the CXYPadEx
			int32_t tagX = xyPad->getTagX();
			int32_t tagY = xyPad->getTagY();

			if (tagX != -1)
			{
				ControlUpdateReceiver* receiver = getControlUpdateReceiver(tagX);
				if (receiver)
				{
					receiver->removeControl(control);
					checkRemoveWriteableControl(control);
				}
			}
			if (tagY != -1)
			{
				ControlUpdateReceiver* receiver = getControlUpdateReceiver(tagY);
				if (receiver)
				{
					receiver->removeControl(control);
					checkRemoveWriteableControl(control);
				}
			}
			return;
		}

		ControlUpdateReceiver* receiver = getControlUpdateReceiver(control->getTag());
		if (receiver)
		{
			receiver->removeControl(control);
			checkRemoveWriteableControl(control);
		}
	}
}


/**
\brief message handler for mouse down event

Operation:\n
- for right mouse button, open the "Edit UI Description" menu item
- for AU and AAX, call their handlers for all other messages

\param frame owning frame
\param where current location of mouse coordinates
\param buttons button state

\return handled or not handled
*/
CMouseEventResult PluginGUI::onMouseDown(CFrame* frame, const CPoint& where, const CButtonState& buttons)
{
    CMouseEventResult result = kMouseEventNotHandled;
    
    // --- added kShift because control + left click behaves as right-click for MacOS
    //     to support the ancient one-mouse paradigm; in ProTools, control + move is for Fine Adjustment
    if (buttons.isRightButton() && buttons & kShift)
    {
        int t=0;
        
#if VSTGUI_LIVE_EDITING
        COptionMenu* controllerMenu = 0;// (delegate && editingEnabled == false) ? delegate->createContextMenu(where, this) : 0;
        if (showGUIEditor == false)
        {
            if (controllerMenu == 0)
                controllerMenu = new COptionMenu();
            else
                controllerMenu->addSeparator();
            
			CMenuItem* item = controllerMenu->addEntry(new CCommandMenuItem({ "Open UIDescription Editor", this, "File", "Open UIDescription Editor" }));
            item->setKey("e", kControl);
        }
        
        if (controllerMenu)
        {
            controllerMenu->setStyle(controllerMenu->isPopupStyle() | controllerMenu->isMultipleCheckStyle());
            controllerMenu->popup(frame, where);
            result = kMouseEventHandled;
        }
        
        if (controllerMenu)
            controllerMenu->forget();
        
        return result;
#endif
    }
    
    // --- try other overrides
    if (!showGUIEditor)
    {
        int controlID = getControlID_WithMouseCoords(where);
        if (sendAAXMouseDown(controlID, buttons) == kMouseEventHandled)
            return kMouseEventHandled;
        
        CControl* control = getControl_WithMouseCoords(where);
        if (sendAUMouseDown(control, buttons) == kMouseEventHandled)
            return kMouseEventHandled;
    }
    return result;
}
    

/**
\brief message handler for mouse move event

Operation:\n
- for AU and AAX, call their specialized handlers 

\param frame owning frame
\param where current location of mouse coordinates
\param buttons button state

\return handled or not handled
*/
CMouseEventResult PluginGUI::onMouseMoved(CFrame* frame, const CPoint& where, const CButtonState& buttons)
{
	CMouseEventResult result = kMouseEventNotHandled;
	if (!showGUIEditor)
	{
		int controlID = getControlID_WithMouseCoords(where);
		if (sendAAXMouseMoved(controlID, buttons, where) == kMouseEventHandled)
			return kMouseEventHandled;

		CControl* control = getControl_WithMouseCoords(where);
		if (sendAUMouseDown(control, buttons) == kMouseEventHandled)
			return kMouseEventHandled;
	}
	return result;
}



}

