// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxplugingui.h
//
/**
    \file   aaxplugingui.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for the AAXPluginGUI object which creates and
    		destroys the ASPiK GUI object as well as pumps synchronized
    		parameter change information as part of the monolithic parameter
    		update loop.

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#pragma once

#ifndef AAX_GUI_H
#define AAX_GUI_H

#include "AAX_CEffectGUI.h"
#include "AAXPluginParameters.h"
#include "plugingui.h"

#if defined AAXPLUGIN && !defined _WINDOWS && !defined _WINDLL
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "AAX_IViewContainer.h"

/**
 \class AAXPluginGUI
 \ingroup AAX-Shell
 \brief
 The AAXPluginGUI is the GUI object for AAX. It creates and destroys the ASPiK GUI. It also synchronizes GUI and
 parameters as described in detail in the book source below.

 NOTES:
 - implements the ASPiK IGUIWindowFrame inteface by simply subclassing from it
 - AAX GUI binding is specific to the monolithic plugin parameter paradigm as described in
   detail in the book source below.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
class AAXPluginGUI : public AAX_CEffectGUI, public IGUIWindowFrame
{
public:
	AAXPluginGUI(void);
	~AAXPluginGUI(void);

    /** creation mechanism */
	static AAX_IEffectGUI* AAX_CALLBACK Create( void );

	// --- AAX_CEffectGUI overrides
	virtual void CreateViewContainer(); ///< AAX_CEffectGUI override
    virtual void DeleteViewContainer(); ///< AAX_CEffectGUI override
	virtual void CreateViewContents(); ///< AAX_CEffectGUI override
	virtual AAX_Result ParameterUpdated(const char* iParameterID); ///< AAX_CEffectGUI override
	virtual AAX_Result Draw(AAX_Rect* iDrawRect); ///< AAX_CEffectGUI override
	virtual AAX_Result TimerWakeup(void); ///< AAX_CEffectGUI override

	/** for sizing the GUI */
	virtual AAX_Result GetViewSize(AAX_Point* oEffectViewSize) const;  ///< for sizing the GUI

    /** IGUIWindowFrame - this allows us to set the view size for the GUI designer (only) */
    virtual bool setWindowFrameSize(double left = 0, double top = 0, double right = 0, double bottom = 0)
    {
        AAX_IViewContainer* iVC = GetViewContainer();
        if(iVC)
        {
            // --- NOTE: height, width
            AAX_Point size( (float)(bottom - top), (float)(right - left) );
            iVC->SetViewSize(size);
            return true;
        }

        return false;
    }

    /** for sizing the GUI */
    virtual bool getWindowFrameSize(double& left, double&  top, double&  right, double&  bottom)
    {
        left = 0;
        top = 0;
        right = (VSTGUI::CCoord)guiWidth;
        bottom = (VSTGUI::CCoord)guiHeight;
        return true;
    }

  protected:
    int guiWidth = 100;             ///< size parameter
    int guiHeight = 100;            ///< size parameter
	pluginCustomData customData;    ///< custom data struct used for moving information from parameters during early construction phase
    VSTGUI::PluginGUI* pluginGUI;           ///< the ASPiK GUI - NOTE that this is maintained only for constructing and destroying the GUI
    bool pureCustomGUI = false;     ///< not used
};



#endif
