// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxtovstguibuttonstate.h
//
/**
    \file   aaxtovstguibuttonstate.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  provided by AVID in the VSTGUI example in SDK

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef VolTestAAA_AAXtoVSTGUIButtonState_h
#define VolTestAAA_AAXtoVSTGUIButtonState_h

#include "AAX_CEffectGUI.h"
#include "AAX_IACFEffectParameters.h"
#include "AAX_IViewContainer.h"


//==============================================================================
// CAAXButtonState
//==============================================================================

static uint32_t SynthesizeModifiers(const VSTGUI::CButtonState& inButtonState, AAX_IViewContainer* inViewContainer)
{
    uint32_t aax_mods = 0;

    if (inButtonState & VSTGUI::kAlt) { aax_mods |= AAX_eModifiers_Option; }
    if (inButtonState & VSTGUI::kApple) { aax_mods |= AAX_eModifiers_Control; } // VSTGUI modifier definitions are flipped on Mac.
    if (inButtonState & VSTGUI::kControl) { aax_mods |= AAX_eModifiers_Command; } // VSTGUI modifier definitions are flipped on Mac.
    if (inButtonState & VSTGUI::kShift) { aax_mods |= AAX_eModifiers_Shift; }
    if (inButtonState & VSTGUI::kRButton) { aax_mods |= AAX_eModifiers_SecondaryButton; }

    // It is best practice to always query the host as
    // well, since the host's key handler may have
    // prevented some modifier key states from reaching
    // the plug-in.
    if (inViewContainer)
    {
        uint32_t aaxViewMods = 0;
        inViewContainer->GetModifiers (&aaxViewMods);
        aax_mods |= aaxViewMods;
    }

    return aax_mods;
}

static VSTGUI::CButtonState SynthesizeButtonState(const VSTGUI::CButtonState& inButtonState, uint32_t inModifiers)
{
    VSTGUI::CButtonState buttons(inButtonState);

    if (AAX_eModifiers_Shift & inModifiers) { buttons |= VSTGUI::kShift; }
    if (AAX_eModifiers_Control & inModifiers) { buttons |= VSTGUI::kApple; } // VSTGUI modifier definitions are flipped on Mac.
    if (AAX_eModifiers_Option & inModifiers) { buttons |= VSTGUI::kAlt; }
    if (AAX_eModifiers_Command & inModifiers) { buttons |= VSTGUI::kControl; } // VSTGUI modifier definitions are flipped on Mac.
    if (AAX_eModifiers_SecondaryButton & inModifiers) { buttons |= VSTGUI::kRButton; }

    return buttons;
}

//==============================================================================
// CAAXButtonState
// Helper class combining representations of the AAX modifier key mask and
// the VSTGUI::CButtonState mask. Includes logic to fix missing modifier
// key states that are removed by host filtering, e.g. by the host key hook.
class AAX_CVSTGUIButtonState
{
public:
    AAX_CVSTGUIButtonState(const VSTGUI::CButtonState& inButtonState, AAX_IViewContainer* inViewContainer)
    : mModifiers(SynthesizeModifiers(inButtonState, inViewContainer))
    , mButtonState(SynthesizeButtonState(inButtonState, mModifiers))
    {
    }

    const VSTGUI::CButtonState& AsVST() const { return mButtonState; }
    uint32_t AsAAX() const { return mModifiers; }

private:
    uint32_t mModifiers;
    VSTGUI::CButtonState mButtonState;

private:
    AAX_CVSTGUIButtonState(); // Unimplemented
};





#endif
