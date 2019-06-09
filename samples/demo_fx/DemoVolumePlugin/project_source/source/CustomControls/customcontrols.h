// -----------------------------------------------------------------------------
//    ASPiK Custom Controls File:  customcontrols.h
//
/**
    \file   customcontrols.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for ASPiK custom control objects (knobs, buttons, meters, etc...)
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef __CustomControls__
#define __CustomControls__

#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/cknob.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/controls/cslider.h"
#include "vstgui/lib/controls/cvumeter.h"
#include "vstgui/lib/controls/cxypad.h"
#include "vstgui/vstgui.h"
#include "vstgui/lib/vstguibase.h"
#include "guiconstants.h"

namespace VSTGUI {

 /**
 \enum mouseAction
 \ingroup Constants-Enums
 \brief
 Use this enum to set the mouse behavior for kick button.

 - enum mouseAction {mouseDirUpAndDown, mouseDirUp, mouseDirDown};

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
enum mouseAction {mouseDirUpAndDown, mouseDirUp, mouseDirDown};

/**
\class CKickButtonEx
\ingroup Custom-Controls
\brief
The CKickButtonEx object extends the VSTGUI CKickButton object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CKickButtonEx : public CKickButton
{
public:
	CKickButtonEx(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;

	/**
	\brief set the mouse behavior (down, up, or down/up)
	\param mode - behavior flag (see mouseAction enumeration)
	*/
	void setMouseMode(unsigned int mode){mouseBehavior = mode;}

private:
	float entryState = 0.0;			///< entry state
	unsigned int mouseBehavior = 0; ///< mouse behavior
};

/**
\class TextButtonEx
\ingroup Custom-Controls
\brief
The TextButtonEx object extends the VSTGUI CTextButton object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class TextButtonEx : public CTextButton
{
public:
	TextButtonEx(const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title = 0, CTextButton::Style = kKickStyle);

	void draw(CDrawContext* context) override;

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse moved event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief set the mouse behavior (down, up, or down/up)
	\param mode - behavior flag (see mouseAction enumeration)
	*/
	void setMouseMode(unsigned int uMode) { mouseBehavior = uMode; }

private:
	float entryState = 0.0;			///< entry state
	unsigned int mouseBehavior = 0; ///< mouse behavior
};

/**
\class CAnimKnobEx
\ingroup Custom-Controls
\brief
The CAnimKnobEx object extends the VSTGUI CAnimKnob object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CAnimKnobEx : public CAnimKnob
{
public:
	CAnimKnobEx(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps,
			CCoord heightOfOneImage, CBitmap* background, const CPoint &offset,
			bool bSwitchKnob = false);

	virtual void draw (CDrawContext* pContext) override;

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse moved event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;

    bool checkDefaultValue (CButtonState button) override;
	virtual void valueChanged() override;

	/**
	\brief set max discrete switching value
	\param f - the max value as a float
	*/
	void setSwitchMax(float f){maxControlValue = f;}

	/**
	\brief query if control is in "switch" mode
	\returns true if in "switch" mode
	*/
    bool isSwitchKnob(){return switchKnob;}

	/**
	\brief sets the AAX flag for this control
	\param b flag to enable AAX mode
	*/
    void setAAXKnob(bool b){aaxKnob = b;}

	/**
	\brief query if control wants Pro Tools keyboard modifiers
	\returns true if in "switch" mode
	*/
	bool isAAXKnob(){return aaxKnob;}

protected:
	bool switchKnob = false;
    bool aaxKnob = false;
	float maxControlValue = 1.0;
	virtual ~CAnimKnobEx(void);

private:
    CPoint firstPoint;
    CPoint lastPoint;
    float  startValue;
    float  fEntryState;
    float  range;
    float  coef;
    CButtonState   oldButton;
    bool   modeLinear;

};

/**
\class CVerticalSliderEx
\ingroup Custom-Controls
\brief
The CVerticalSliderEx object extends the VSTGUI CVerticalSlider object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CVerticalSliderEx : public CVerticalSlider
{
public:
	CVerticalSliderEx (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kBottom);

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse moved event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;

	bool checkDefaultValue (CButtonState button) override;

	/**
	\brief set max discrete switching value
	\param f - the max value as a float
	*/
	void setSwitchMax(float f){maxControlValue = f;}

	/**
	\brief query if control is in "switch" mode
	\returns true if in "switch" mode
	*/
	bool isSwitchSlider(){return switchSlider;}

	/**
	\brief sets the control into "switch" mode
	\param b flag to enable switch mode
	*/
	void setSwitchSlider(bool b){switchSlider = b;}

	/**
	\brief sets the AAX flag for this control
	\param b flag to enable AAX mode
	*/
	void setAAXSlider(bool b){aaxSlider = b;}

	/**
	\brief query if control wants Pro Tools keyboard modifiers
	\returns true if in "switch" mode
	*/
	bool isAAXSlider(){return aaxSlider;}

	CLASS_METHODS(CVerticalSliderEx, CControl)

protected:
	~CVerticalSliderEx ();
	bool switchSlider;
    bool aaxSlider;
	float maxControlValue;

private:
    CCoord	delta;
    float	oldVal;
    float	startVal;
    CButtonState oldButton;
    CPoint mouseStartPoint;
};


/**
\class CHorizontalSliderEx
\ingroup Custom-Controls
\brief
The CHorizontalSliderEx object extends the VSTGUI CHorizontalSlider object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CHorizontalSliderEx : public CHorizontalSlider
{
public:
	CHorizontalSliderEx (const CRect& size, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset = CPoint (0, 0), const int32_t style = kLeft);

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse moved event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;

	bool checkDefaultValue (CButtonState button) override;

	/**
	\brief set max discrete switching value
	\param f - the max value as a float
	*/
	void setSwitchMax(float f){maxControlValue = f;}

	/**
	\brief sets the control into "switch" mode
	\param b flag to enable switch mode
	*/
	void setSwitchSlider(bool b){switchSlider = b;}

	/**
	\brief query if control is in "switch" mode
	\returns true if in "switch" mode
	*/
	bool isSwitchSlider(){return switchSlider;}

	/**
	\brief sets the AAX flag for this control
	\param b flag to enable AAX mode
	*/
	void setAAXSlider(bool b){aaxSlider = b;}

	/**
	\brief query if control wants Pro Tools keyboard modifiers
	\returns true if in "switch" mode
	*/
	bool isAAXSlider(){return aaxSlider;}

	CLASS_METHODS(CHorizontalSliderEx, CControl)

protected:
	~CHorizontalSliderEx ();
	bool switchSlider;
	float maxControlValue;
    bool aaxSlider;

private:
    CCoord	delta;
    float	oldVal;
    float	startVal;
    CButtonState oldButton;
    CPoint mouseStartPoint;
};

/**
\class CMeterDetector
\ingroup Custom-Controls
\brief
The CMeterDetector object provides a dedicated detector for VU meter objects.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CMeterDetector
{
public:
    CMeterDetector(void);
    ~CMeterDetector(void);

public:

    // Call the Init Function to initialize and setup all at once; this can be called as many times
    // as you want
    void init(float samplerate, float attack_in_ms, float release_in_ms, bool bAnalogTC, unsigned int uDetect, bool bLogDetector);

    // these functions allow you to change modes and attack/release one at a time during
    // realtime operation
    void setTCModeAnalog(bool _analogTC);

    // THEN do these after init
    void setAttackTime(float attack_in_ms);
    void setReleaseTime(float release_in_ms);

    // --- see guiconstants.h
    // ENV_DETECT_MODE_PEAK		= 0;
    // ENV_DETECT_MODE_MS		= 1;
    // ENV_DETECT_MODE_RMS		= 2;
    // ENV_DETECT_MODE_NONE		= 3;
    void setDetectMode(unsigned int _detectMode) {detectMode = _detectMode;}

    void setSampleRate(float f)
    {
        detectorSampleRate = f;

        setAttackTime(attackTime_mSec);
        setReleaseTime(releaseTime_mSec);
    }

    void setLogDetect(bool b) {logDetector = b;}

    // call this to detect; it returns the peak ms or rms value at that instant
    float detect(float input);

    // call this from your prepareForPlay() function each time to reset the detector
    void prepareForPlay();

    void resetPeakHold(){ peakEnvelope = -1.0; }
    void setPeakHold(bool b) { peakHold = b; }

protected:
    float attackTime;
    float m_fReleaseTime;
    float attackTime_mSec;
    float releaseTime_mSec;
    float detectorSampleRate;
    float envelope;
    float peakEnvelope;
    bool  analogTC;
    bool  logDetector;
    unsigned int  detectMode;
    bool peakHold;

    void setEnvelope(float value)
    {
        if(!peakHold)
        {
            envelope = value;
            return;
        }

        // --- holding peak
        if(value > peakEnvelope)
        {
            peakEnvelope = value;
            envelope = value;
        }
    }
};

/**
\class CVuMeterEx
\ingroup Custom-Controls
\brief
The CVuMeterEx object extends the VSTGUI CVuMeter object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CVuMeterEx : public CVuMeter
{
public:
	CVuMeterEx(const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, int32_t nbLed, bool bInverted, bool bAnalogVU, int32_t style = kVertical);

	CVuMeterEx(void);
	~CVuMeterEx(void);

	// --- overrides!
	virtual void draw(CDrawContext* pContext) override;
	virtual void setViewSize (const CRect& newSize, bool invalid = true) override;

	inline void initDetector(float samplerate, float attack_in_ms, float release_in_ms, bool bAnalogTC, unsigned int uDetect, bool bLogDetector)
	{
		detector.init(samplerate, attack_in_ms, release_in_ms, bAnalogTC, uDetect, bLogDetector);
		detector.prepareForPlay();
	}

	void setHtOneImage(double d){heightOfOneImage = d;}
	void setImageCount(double d){subPixMaps = d;}
	void setZero_dB_Frame(double d){zero_dB_Frame = d;}

protected:
	bool isInverted;
	bool isAnalogVU;
	double zero_dB_Frame;
	double heightOfOneImage;
	double subPixMaps;

	CMeterDetector detector;
};

/**
\class CXYPadEx
\ingroup Custom-Controls
\brief
The CXYPadEx object extends the CXYPad CVuMeter object with extra functionality.\n
It is used in the PluginGUI object for creating custom views.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class CXYPadEx : public CXYPad
{
public:
	CXYPadEx(const CRect& size = CRect (0, 0, 0, 0));

	// --- for easy trackpad and XY stuff
public:
	void setTagX(int32_t tag){tagX = tag;}
	int32_t getTagX(){return tagX;}

	void setTagY(int32_t tag){tagY = tag;}
	int32_t getTagY(){return tagY;}

	// --- overrides
	void draw(CDrawContext* context) override;

	/**
	\brief handle mouse moved event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse up event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;

	/**
	\brief handle mouse down event
	\param where - coordinates of mouse event
	\param buttons - button state during mouse event
	*/
	CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
	virtual void setValue(float val) override;

    // --- for vector joystick operation
	inline int pointInPolygon(int nvert, float *vertx, float *verty, float testx, float testy)
	{
		int i, j, c = 0;
		for (i = 0, j = nvert-1; i < nvert; j = i++)
		{
			if ( ((verty[i]>testy) != (verty[j]>testy)) &&
				(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
			c = !c;
		}
	 return c;
	}

    static void calculateXY (float value, float& x, float& y)
    {
        x = std::floor (value * 1000.f + 0.5f) * 0.001f;
        y = std::floor ((value - x)  * 10000000.f + 0.5f) * 0.001f;
        y = -1.f*y + 1.f;
    }

    static float calculateValue (float x, float y)
    {
        y = -1.f*y + 1.f;
        x = std::floor (x * 1000.f + 0.5f) * 0.001f;
        y = std::floor (y * 1000.f + 0.5f) * 0.0000001f;
        return x + y;
    }

protected:
    int32_t tagX;
    int32_t tagY;
    bool isJoystickPad;

	float vertX[4];
	float vertY[4];

	float lastX;
	float lastY;
};


}

#endif
