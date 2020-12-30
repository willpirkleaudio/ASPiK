 // -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugingui.h
//
/**
    \file   plugingui.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for ASPiK GUI object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#pragma once
#ifndef __PluginGUI__
#define __PluginGUI__

//#define DOXYDOC 1

#ifdef DOXYDOC

#define AUPLUGIN 1
#define AAXPLUGIN 1
#define VSTPLUGIN 1
#define RAFXPLUGIN 1

#endif

// --- custom VSTGUI4 derived classes
#include "customcontrols.h"

// --- VSTGUI
#include "vstgui/vstgui.h"
#include "vstgui/uidescription/uiviewswitchcontainer.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/lib/crect.h"
// 4.9
#include "vstgui/uidescription/cstream.h"

#if VSTGUI_LIVE_EDITING
#include "vstgui/uidescription/editing/uieditcontroller.h"
#include "vstgui/uidescription/editing/uieditmenucontroller.h"
#endif

#ifdef AUPLUGIN
#import <CoreFoundation/CoreFoundation.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#endif

#ifdef AAXPLUGIN
#include "AAX_IEffectParameters.h"
#include "AAXtoVSTGUIButtonState.h"
#endif

// --- plugin stuff
#include "pluginparameter.h"

// --- std::
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <map>

// --- const for host choice knob mode
const uint32_t kHostChoice = 3;

namespace VSTGUI {

/**
\class ControlUpdateReceiver
\ingroup ASPiK-GUI
\brief
The ControlUpdateReceiver object is the connection mechanism between PluginParameter objects and their connected
GUI control objects. It was originally designed almost identically to the example code in the VSTGUI4
SDK for the VST3 version. However, once the AU, AAX, VST3 and RAFX2 APIs were consolidated, this object changed significantly.
If you are interested in creating your own version, check out the VST3 files that are included with the vstgui SDK.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class ControlUpdateReceiver
{
public:
	/** constructs a receiver object with the control and parameter pointers */
    ControlUpdateReceiver(CControl* control, PluginParameter* pluginParameterPtr, bool _isControlListener)
    {
       hasRefGuiControl = pluginParameterPtr ? true : false;

        if(hasRefGuiControl)
            refGuiControl = *pluginParameterPtr;

        addControl(control);
        isControlListener = _isControlListener;
    }

	/** forgets the stored control pointers so that they self-destruct properly */
    ~ControlUpdateReceiver()
    {
		for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* ctrl = *it;
            ctrl->forget();
        }
    }

	/** check to see if a control exists in our list of GUI controls
	\param control pointer to control to check
	\return true if we already store it, false otherwise
	*/
    bool hasControl(CControl* control)
    {
        return std::find(guiControls.begin (), guiControls.end (), control) != guiControls.end ();
    }

	/** add a control to our list of GUI controls
	\param control pointer to control to add
	*/
	void addControl(CControl* control)
    {
        if(hasControl(control))
            return;

        control->remember();
        guiControls.push_back(control);

        // --- set default value (rather than from XML, which is a pain because it must be normalized)
        if(hasRefGuiControl)
        {
            float normalizedValue = (float)refGuiControl.getDefaultValueNormalized();
            control->setDefaultValue(normalizedValue);
        }
    }

	/** remove a contro from our list of GUI controls
	\param control pointer to control to check
	*/
	void removeControl(CControl* control)
    {
        for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl == control)
            {
                ctrl->forget();
                guiControls.erase(it);
                return;
            }
        }
    }

	/** check to see if a control is currently editing; this is part of the system that
	    prevents a control from receiving multiple edit commands from our loop
	\return true if the control is editing (being adjusted by the user - does not include automation)
	*/
	bool controlInRxGroupIsEditing()
    {
        for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl && ctrl->isEditing())
                return true;
        }
        return false;
    }

	/** get actual parameter value using a normalized version
	\param normalizedValue input value
	\return the actual value calculated from the normalized version
	*/
	float getActualValueWithNormalizedValue(float normalizedValue)
    {
        if (hasRefGuiControl)
        {
            if (normalizedValue == (float)refGuiControl.getDefaultValueNormalized())
            {
                return (float)refGuiControl.getDefaultValue();
            }
        }

        return (float)refGuiControl.getControlValueWithNormalizedValue(normalizedValue);
    }

	/** get normalized parameter value using an actual version
	\param actualValue input value
	\return the nromalized value calculated from the actual version
	*/
	float getNormalizedValueWithActualValue(float actualValue)
    {
		return (float)refGuiControl.getNormalizedControlValueWithActualValue(actualValue);
    }

	/** update all controls that share a commmon tag (control ID) with the input control
	\param control input control
	*/
	void updateControlsWithControl(CControl* control)
    {
        updateControlsWithActualValue(getActualValueWithNormalizedValue(control->getValueNormalized()), control);
    }

	/** update all controls that share a commmon tag (control ID) with the input value
	\param normalizedValue normalized input value
	\param control input control
	*/
	void updateControlsWithNormalizedValue(float normalizedValue, CControl* control = nullptr)
    {
		// --- is this the defalt value?
		if (hasRefGuiControl)
		{
			if (normalizedValue == (float)refGuiControl.getDefaultValueNormalized())
			{
				updateControlsWithActualValue(refGuiControl.getDefaultValue(), control);
				return;
			}
		}

        updateControlsWithActualValue(getActualValueWithNormalizedValue(normalizedValue), control);
    }

	/** initialize the control with the current plugin value (initialization routine)
	\param control input control
	*/
	void initControl(CControl* control)
    {
		float actualValue = (float)refGuiControl.getControlValue();
        updateControlsWithActualValue(actualValue, control);
    }

	/** update the control with an acutal value (e.g. from automation)
	\param actualValue input value
	\param control input control
	*/
	void updateControlsWithActualValue(float actualValue, CControl* control = nullptr)
    {
        // --- eliminiate glitching from AAX parameter loop
        if(!control && controlInRxGroupIsEditing())
            return;

        // --- store on our reference control
        refGuiControl.setControlValue(actualValue);

        // --- synchoronize all controls that share this controlID
        for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* guiCtrl = *it;
            if(guiCtrl && control != guiCtrl) // nned the check for XY pads
            {
                // --- need to take care of multiple control types, slight differences in setup
                CTextLabel* label = dynamic_cast<CTextLabel*>(guiCtrl);
                COptionMenu* oMenu = dynamic_cast<COptionMenu*>(guiCtrl);
                CXYPadEx* xyPad = dynamic_cast<CXYPadEx*>(guiCtrl);
                CVuMeter* meter = dynamic_cast<CVuMeter*>(guiCtrl);

                if(meter) /// ignore meters; this is handled in idle() for consistency
                {
                    continue;
                }
                else if(label)
                {
                    label->setText(refGuiControl.getControlValueAsString().c_str());
                    label->invalid();
                }
                else if(oMenu)
                {
                    // --- current rafx GUI Designer does not set max to anything
					guiCtrl->setMin((float)refGuiControl.getGUIMin());
					guiCtrl->setMax((float)refGuiControl.getGUIMin());

                    // --- load for first time, NOT dynamic loading here
                    oMenu->removeAllEntry();

                    for (uint32_t i = 0; i < refGuiControl.getStringCount(); i++)
                    {
                        oMenu->addEntry(refGuiControl.getStringByIndex(i).c_str(), i);
                    }

					oMenu->setValue((float)refGuiControl.getControlValue());
                }
                else if(xyPad)
                {
                    float x = 0.f;
                    float y = 0.f;
                    xyPad->calculateXY(xyPad->getValue(), x, y);

                    // --- retrieve the X and Y tags on the CXYPadEx
                    int32_t tagX = xyPad->getTagX();
                    int32_t tagY = xyPad->getTagY();

                    if(tagX == refGuiControl.getControlID())
						x = (float)refGuiControl.getControlValueNormalized();
                    if(tagY == refGuiControl.getControlID())
						y = (float)refGuiControl.getControlValueNormalized();

                    if(tagX >= 0 && tagY >= 0 && !guiCtrl->isEditing())
                        xyPad->setValue(xyPad->calculateValue(x, y));
                }
                else if(!guiCtrl->isEditing())
					guiCtrl->setValueNormalized((float)refGuiControl.getControlValueNormalized());

                guiCtrl->invalid();
            }
        }
    }

	/** get a PluginParameter that corresponds to a GUI control
	    NOTE: this is a constant reference that is unmutable
	\return underlying const PluginParameter
	*/
	const PluginParameter getGuiControl(){return refGuiControl;}

	/** get the control ID of the underlying parameter
	\return underlying control ID
	*/
	int32_t getControlID(){ return refGuiControl.getControlID(); }

	/** test to see if control and its container are both visible, for AAX only as part of the
	    modifier buttons exclusive to Pro Tools
	\return true if control and container are visible
	*/
    inline bool controlAndContainerVisible(CControl* ctrl)
    {
        if(!ctrl) return false;

        bool stickyVisible = ctrl->isVisible();

        if(!stickyVisible)
            return false;

        // --- check parents
        CView* parent = ctrl->getParentView();
        while(parent)
        {
            stickyVisible = parent->isVisible();
            if(!stickyVisible)
                return false;

            parent = parent->getParentView();
        }
        return stickyVisible;
    }

	/** get the ID of a control based on its mouse coordinates, for AAX only as part of the
	modifier buttons exclusive to Pro Tools
	\param where the mouse coordinates
	\return the control ID of the located control, or -1 if not located
	*/
	int getControlID_WithMouseCoords(const CPoint& where)
    {
        CPoint mousePoint = where;

        for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl && controlAndContainerVisible(ctrl))
            {
                CPoint point = ctrl->frameToLocal(mousePoint);
                CRect rect = ctrl->getViewSize();
                if(rect.pointInside(point))
                {
                    int tag = ctrl->getTag();
                    if(isReservedTag(tag))
                        return -1;
                    else
                        return tag;
                }
            }
        }
        return -1;
    }

	/** get a pointer to a control based on its mouse coordinates, for AAX only as part of the
	modifier buttons exclusive to Pro Tools
	\param where the mouse coordinates
	\return a pointer of the located control, or nullptr if not located
	*/
	CControl* getControl_WithMouseCoords(const CPoint& where)
    {
        CPoint mousePoint = where;

        for(std::vector<CControl*>::iterator it = guiControls.begin(); it != guiControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl && controlAndContainerVisible(ctrl))
            {
                CPoint point = ctrl->frameToLocal(mousePoint);
                CRect rect = ctrl->getViewSize();
                if(rect.pointInside(point))
                    return ctrl;
            }
        }
        return nullptr;
    }

protected:
    PluginParameter refGuiControl;			///< single parameter with this control tag
    std::vector<CControl*> guiControls;		///< list of controls that share the control tag with this one
    bool hasRefGuiControl = false;			///< internal flag
    bool isControlListener = false;			///< internal flag
};


/**
\class PluginGUI
\ingroup ASPiK-GUI
\brief
The PluginGUI object that maintains the entire GUI operation and has #defines to use with AAX, AU, VST3 and RAFX2 plugins.
Note the multiple inheritance inolved with this very complex object. The IGUIView interface is the only non-VSTGUI4 base
class. It allows for GUI resizing and manipulation, if and only if the host DAW allows and provides the mechanism for it.

Features:\n
- fully self-contained and stand-alone object
- uses interfaces for proper and safe communication with plugin shell (NEVER communicates with core or any other kernel object)
- has built-in zooming option for changing GUI size
- includes all necessary code to maintain host requirements (e.g. AU Event Listener system for AU)
- includes a built-in drag-and-drop GUI designer that works in any DAW/API/OS
- the GUI object is set to decode a single XML file that represents the GUI; you can extend this to load multiple, different XML files
- uses the VSTGUI4 XML parser object, UIDescription, and UIEditController objects; there is no hack-code and it uses VSTGUI4-approved
mechanisms
- allows for custom views
- allows for custom sub-controllers

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginGUI : public IController,
				  public IViewAddedRemovedObserver,
				  public IMouseObserver,
				  public IKeyboardHook,
				  public VSTGUIEditorInterface,
				  public CBaseObject,
				  public ICommandMenuItemTarget,
				  public IGUIView
{

public:
	/** construction with XML file*/
    PluginGUI(UTF8StringPtr _xmlFile);

	/** destruction */
	virtual ~PluginGUI();

	/** create the GUI inside of a host-supplied window frame */
    bool open(UTF8StringPtr _viewName,
			  void* parent,
			  const std::vector<PluginParameter*>* pluginParameterPtr,
			  const PlatformType& platformType = kDefaultNative,
			  IGUIPluginConnector* _guiPluginConnector = nullptr,
			  void* data = nullptr);

	/** destroy the GUI guts before frame is destoyed */
    void close();

	/** sync GUI control to current plugin value, thread-safe */
    void syncGUIControl(uint32_t controlID);

	/** get current GUI size */
    void getSize(float& width, float& height);

	/** scale the GUI */
	void scaleGUISize(uint32_t controlValue);

	/** for preset saving helper which writes preset code for you */
	void writeToPresetFile();

    /** clear the interface prior to shutdown; AU onlyl */
    void clearGUIPluginConnector(){ guiPluginConnector = nullptr; }
    
protected:
	/** the udpate and repaint function */
	virtual void idle();

	/** do any API specific stuff that is required prior to the GUI creation */
	void preCreateGUI();

	/** create the GUI, or the GUI Designer if user is launching it */
	bool createGUI(bool bShowGUIEditor);

	/** save the current state in XML */
	void save(bool saveAs = false);

	/** get the current knob behavior (important - users are very aware of this) */
	virtual int32_t getKnobMode() const override;

	/** destroy the parameter list */
    void deleteGUIControlList()
    {
        for(std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it) {
            delete *it;
        }
        pluginParameters.clear();
    }

	/**
	\brief find the local PluginParameter that is connected to the same control ID

	\param tag the control ID

	\returns the connected PluginParameter* or nullptr if not found
	*/
	PluginParameter* getGuiControlWithTag(int tag)
    {
        for(std::vector<PluginParameter*>::iterator it = pluginParameters.begin(); it != pluginParameters.end(); ++it) {
            PluginParameter* ctrl = *it;
            if(ctrl->getControlID() == tag)
                return ctrl;
        }
        return nullptr;
    }

	// --- protected variables
    IGUIPluginConnector* guiPluginConnector = nullptr; ///< the plugin shell interface that arrives with the open( ) function; OK if NULL for standalone GUIs
	UIDescription* description = nullptr; ///< the description version of the XML file
	std::string viewName;			///< name
	std::string xmlFile;			///< the XML file name

	uint32_t numUIControls = 0;		///< control counter
	double zoomFactor = 1.0;		///< scaling factor for built-in scaling
	CVSTGUITimer* timer;			///< timer object (this is platform dependent)

	CPoint minSize;		///< the min size of the GUI window
	CPoint maxSize;		///< the max size of the GUI window
	CRect nonEditRect;	///< non-edit area for GUI designer

	// --- flags for showing view
	bool showGUIEditor = false; ///< show the GUI designer
	bool createNewView = true;	///< show the normal GUI

	double guiDesignerWidth;	///< GUI Designer's frame size
	double guiDesignerHeight;	///< GUI Designer's frame size
	float guiWidth;				///< embedded GUI size
	float guiHeight;			///< embedded GUI size

    IGUIWindowFrame* guiWindowFrame = nullptr;	///< interface to allow plugin shell to resize our window
	CFrame* guiEditorFrame = nullptr;			///< pointer to our frame
	uint32_t knobAction = kLinearMode;			///< knob mode

	/** get size as string */
	std::string getGUIDesignerSize();

	/** get the control ID value using current mouse location */
	int getControlID_WithMouseCoords(const CPoint& where);

	/** get the control pointer value using current mouse location */
    CControl* getControl_WithMouseCoords(const CPoint& where);

	/** Pro Tools: mouse down */
	CMouseEventResult sendAAXMouseDown(int controlID, const CButtonState& buttons);

	/** Pro Tools: mouse moved */
	CMouseEventResult sendAAXMouseMoved(int controlID, const CButtonState& buttons, const CPoint& where);

	/** AU: mouse down */
	CMouseEventResult sendAUMouseDown(CControl* control, const CButtonState& buttons);

	/** AU: mouse moved (not used) */
	CMouseEventResult sendAUMouseMoved(CControl* control, const CButtonState& buttons, const CPoint& where) { return kMouseEventNotImplemented; }

	/**
	\brief safely set a plugin shell parameter with a GUI control

	NOTE:\n
	- uses the API-specific and approved thread-safe mechanism to set a plugin parameter

	\param control pointer to the control that generated the notification
	\param tag the control ID
	\param actualValue the actual (aka plain) parameter value
	\param normalizedValue the normalied version of the parameter value
	*/
	void setPluginParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue)
	{
#ifdef AUPLUGIN
		setAUEventFromGUIControl(control, tag, actualValue);
#endif

#ifdef AAXPLUGIN
		setAAXParameterFromGUIControl(control, tag, actualValue, normalizedValue);
#endif

#ifdef VSTPLUGIN
		setVSTParameterFromGUIControl(control, tag, actualValue, normalizedValue);
#endif

#ifdef RAFXPLUGIN
		setRAFXParameterFromGUIControl(control, tag, actualValue, normalizedValue);
#endif
}

////////// ------------------------------------------------------------------------------------ //
////////// --- @AAX PLUGINS ONLY --------------------------------------------------------------- //
////////// ------------------------------------------------------------------------------------ //
#ifdef AAXPLUGIN
public:
	/**
	\brief AAX ONLY: plugin shell GUI object sets this after creation
	\param _aaxViewContainer the AAX_IViewContainer* that the parent object exposes
	*/
    void setAAXViewContainer(AAX_IViewContainer* _aaxViewContainer){ aaxViewContainer = _aaxViewContainer;}

	/** AAX ONLY: thread-safe parameter set for AAX */
	void setAAXParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue);

	/** AAX ONLY: update GUI control for AAX */
	void updateGUIControlAAX(int tag, float actualPluginValue, float normalizedValue = 0.f, bool useNormalized = false);
protected:
#endif


#ifdef AUPLUGIN
    AudioUnit m_AU;							///< AU ONLY: the AU plugin reference
    AUEventListenerRef AUEventListener;		///< AU ONLY: the event listener token

public:
	/** AU ONLY:au event dispatcher to GUI (see FX book) */
    void dispatchAUControlChange(int tag, float actualPluginValue, int message = -1, bool fromEventListener = false);

	/** AU ONLY: store the parent AU reference; part of AU event lisener system
	\param inAU reference to the AU plugin (can not be used to call functions)
	*/
	void setAU(AudioUnit inAU){m_AU = inAU;}

protected:
	/** AU ONLY: GUI to au event listener (see FX book) */
    void setAUEventFromGUIControl(CControl* control, int tag, float normalizedValue);
#endif

#ifdef VSTPLUGIN
	/** VST3 ONLY: GUI to VST3 shell (see FX book) */
	void setVSTParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue);

	/** VST3 ONLY: VST3 shell to GUI (see FX book) */
	void updateGUIControlVST(int tag, float normalizedValue);
#endif

#ifdef RAFXPLUGIN
	/** RAFX2 ONLY: GUI to RAFX2 shell (see FX book) */
	void setRAFXParameterFromGUIControl(CControl* control, int tag, float actualValue, float normalizedValue);

	/** RAFX2 ONLY: RAXF2 shell to GUI (see FX book) */
	void updateGUIControlRAFX(int tag, float normalizedValue);
#endif


public:
	/** VSTGUI4 message notification */
	CMessageResult notify(CBaseObject* sender, IdStringPtr message) override;

	/** IControlListener:: required override */
	virtual void valueChanged(VSTGUI::CControl* pControl) override;

	/** unused override */
	virtual int32_t controlModifierClicked(VSTGUI::CControl* pControl, VSTGUI::CButtonState button) override { return 0; }	///< return 1 if you want the control to not handle it, otherwise 0

	/** the user has started to change the GUI control */
	virtual void controlBeginEdit(VSTGUI::CControl* pControl) override;

	/** the user has finished changing the GUI control */
	virtual void controlEndEdit(VSTGUI::CControl* pControl) override;

	/** the GUI control's tag will change (start of creation) */
	virtual void controlTagWillChange(VSTGUI::CControl* pControl) override;

	/** the GUI control's tag did change (end of creation) */
	virtual void controlTagDidChange(VSTGUI::CControl* pControl) override;
#if DEBUG
	/** unused override */
	virtual char controlModifierClicked(VSTGUI::CControl* pControl, long button) { return 0; }
#endif

	/** first chance is for you to create and register a custom view */
	CView* createUserCustomView(std::string viewname, const CRect rect, IControlListener* listener, int32_t tag);

	/** create a custom view */
    CView* createView(const UIAttributes& attributes, const IUIDescription* description) override;

	/** create a custom sub-controller */
	IController* createSubController(UTF8StringPtr name, const IUIDescription* description) override;

	/**- get the receiver info */
    ControlUpdateReceiver* getControlUpdateReceiver(int32_t tag) const;

	/** IViewAddedRemovedObserver view added: unused*/
	void onViewAdded(CFrame* frame, CView* view) override {}

	/** IViewAddedRemovedObserver view removed*/
	void onViewRemoved(CFrame* frame, CView* view) override;

	/** IMouseObserver override not used */
	void onMouseEntered(CView* view, CFrame* frame) override {}

	/** IMouseObserver override not used */
	void onMouseExited(CView* view, CFrame* frame) override {}

	/** IMouseObserver mouse moved handler */
	CMouseEventResult onMouseDown(CFrame* frame, const CPoint& where, const CButtonState& buttons) override;

	/** IMouseObserver mouse moved handler */
	CMouseEventResult onMouseMoved(CFrame* frame, const CPoint& where, const CButtonState& buttons) override;

	/** IKeyboardHook key down handler, not used */
	int32_t onKeyDown(const VstKeyCode& code, CFrame* frame) override { return -1; }

	/** IKeyboardHook key up handler, not used */
	int32_t onKeyUp(const VstKeyCode& code, CFrame* frame) override { return -1; }

	/** ICommandMenuItemTarget called before the item is shown to validate its state */
	virtual bool validateCommandMenuItem(CCommandMenuItem* item);
	
	/** ICommandMenuItemTarget called when the item was selected */
	virtual bool onCommandMenuItemSelected(CCommandMenuItem* item);

	/**
	\brief set the interface pointer for resizing from the GUI
	- this must happen AFTER the open( ) function creates the frame

	\param frame the interface pointer
	*/
	virtual void setGUIWindowFrame(IGUIWindowFrame* frame) override
	{
		// --- we keep for resiging here
		guiWindowFrame = frame;
	}

	/**
	\brief simple helper function to get size from string
	\param str the string
	\param point the CPoint version of the string data
	*/
	inline static bool parseSize (const std::string& str, CPoint& point)
    {
        size_t sep = str.find (',', 0);
        if (sep != std::string::npos)
        {
            point.x = strtol (str.c_str (), 0, 10);
            point.y = strtol (str.c_str () + sep+1, 0, 10);
            return true;
        }
        return false;
    }

    // --- for Meters only right now, but could be used to mark any control as writeable
	/**
	\brief check to see if we already store this meter control

	\param control the control object

	\return true if we have control, false otherwise
	*/
	bool hasWriteableControl(CControl* control)
    {
        return std::find (writeableControls.begin (), writeableControls.end (), control) != writeableControls.end ();
    }

	/**
	\brief check to see if we already store this meter control and add it if we don't

	\param control the control object
	\param piParam the parameter object associated with the control
	*/
    void checkAddWriteableControl(PluginParameter* piParam, CControl* control)
    {
       if(!piParam)
            return;
        if(!control)
            return;
        if(!piParam->getIsWritable())
            return;
        if(!hasWriteableControl(control))
        {
            writeableControls.push_back(control);
            control->remember();
        }
    }

	/**
	\brief check to see if we already store this meter control and remove it if we do

	\param control the control object
	*/
	void checkRemoveWriteableControl(CControl* control)
    {
        if(!hasWriteableControl(control)) return;

        for(std::vector<CControl*>::iterator it = writeableControls.begin(); it != writeableControls.end(); ++it)
        {
            CControl* ctrl = *it;
            if(ctrl == control)
            {
                ctrl->forget();
				writeableControls.erase(it);
				return;
            }
        }
    }


	/**
	\brief delete all reciever objects
	*/
	void deleteControlUpdateReceivers()
	{
		for (ControlUpdateReceiverMap::const_iterator it = controlUpdateReceivers.begin(), end = controlUpdateReceivers.end(); it != end; ++it)
		{
			delete it->second;
		}
		controlUpdateReceivers.clear();
	}

	/**
	\brief forget all writeable (neter) controls
	*/
	void forgetWriteableControls()
	{
		for (std::vector<CControl*>::iterator it = writeableControls.begin(); it != writeableControls.end(); ++it)
		{
			CControl* ctrl = *it;
			ctrl->forget();
		}
        writeableControls.clear();
	}

	/**
	\brief simple helper function to test view for ICustomView

	\param view the view to test

	\return true if view exposes a custom view
	*/
	bool hasICustomView(CView* view)
	{
		ICustomView* customView = dynamic_cast<ICustomView*>(view);
		if (customView)
			return true;
		return false;
	}

	/**
	\brief simple helper function to test sub-controller for ICustomView

	\param view the view to test

	\return true if view exposes a custom view
	*/
	bool hasICustomView(IController* subController)
	{
		ICustomView* customView = dynamic_cast<ICustomView*>(subController);
		if (customView)
			return true;
		return false;
	}

private:
    typedef std::map<int32_t, ControlUpdateReceiver*> ControlUpdateReceiverMap; ///< map of control receivers
    ControlUpdateReceiverMap controlUpdateReceivers;
    std::vector<CControl*> writeableControls;		///< vector of meters
    std::vector<PluginParameter*> pluginParameters; ///< local COPY of parameters

#ifdef AAXPLUGIN
    AAX_IViewContainer* aaxViewContainer = nullptr; ///< required by AAX
#endif
};

}

#endif
