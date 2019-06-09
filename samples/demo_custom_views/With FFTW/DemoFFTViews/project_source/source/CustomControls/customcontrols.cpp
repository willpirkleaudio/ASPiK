// -----------------------------------------------------------------------------
//    ASPiK Custom Controls File:  customcontrols.cpp
//
/**
    \file   customcontrols.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implemenation file for ASPiK custom control objects (knobs, buttons, meters, etc...)
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "customcontrols.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cdrawcontext.h"

#include <cmath>
#pragma warning (disable : 4244) // conversion from 'int' to 'float', possible loss of data for knob/slider switch views (this is what we want!)

namespace VSTGUI {

/**
\brief CKickButtonEx constructor

\param size - the control rectangle
\param listener - the control's listener (usuall PluginGUI object)
\param tag - the control ID value
\param background - the control's custom graphics file
\param offset - (x,y) offset point
*/
CKickButtonEx::CKickButtonEx(const VSTGUI::CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
: CKickButton(size, listener, tag, background, offset)
{
	mouseBehavior = mouseDirUpAndDown;
}

CMouseEventResult CKickButtonEx::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	value = 1.0;
	entryState = value;
	beginEdit ();

	if(mouseBehavior == mouseDirUpAndDown || mouseBehavior == mouseDirDown)
	{
		if(value)
			valueChanged();
		//value = getMin();
		// valueChanged();
		if(isDirty())
			invalid();
		//	endEdit();
	}

	// onMouseMoved() and NOT noMouseDown(), see base class
	return onMouseMoved(where, buttons);
}

CMouseEventResult CKickButtonEx::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	//if(value) // removed stuff
	//	valueChanged();
	value = getMin();
	if(mouseBehavior == mouseDirUpAndDown || mouseBehavior == mouseDirUp) // added
		valueChanged();
	if(isDirty())
		invalid();
	endEdit ();

	return kMouseEventHandled;
}

#if TARGET_OS_IPHONE
    static const float kCKnobRange = 300.f;
#else
    static const float kCKnobRange = 200.f;
#endif

/**
\brief TextButtonEx constructor

\param size - the control rectangle
\param listener - the control's listener (usuall PluginGUI object)
\param tag - the control ID value
\param title - the button text
\param style - button style (see CTextButton::Style)
*/
TextButtonEx::TextButtonEx(const VSTGUI::CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr title, CTextButton::Style style)
	: CTextButton(size, listener, tag, title, style)
{
	entryState = 0.0;
	mouseBehavior = mouseDirUpAndDown;
}

CMouseEventResult TextButtonEx::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if (isEditing())
	{
		if (where.x >= getViewSize().left && where.y >= getViewSize().top  &&
		where.x <= getViewSize().right && where.y <= getViewSize().bottom)
		{
			if (style == kOnOffStyle)
				value = entryState == getMin() ? getMax() : getMin();
			else
				value = entryState == getMin() ? getMin() : getMax();
		}
		else
			value = entryState == getMin() ? getMin() : getMax();

		invalid();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

void TextButtonEx::draw(CDrawContext* context)
{
	bool highlight = value > 0.5 ? true : false;
	context->setDrawMode(kAntiAliasing);
	context->setLineWidth(frameWidth);

	auto lineWidth = getFrameWidth();
	if (lineWidth < 0.)
		lineWidth = context->getHairlineSize();

	context->setLineStyle(CLineStyle(CLineStyle::kLineCapRound, CLineStyle::kLineJoinRound));
	context->setFrameColor(highlight ? frameColorHighlighted : frameColor);
	CRect r(getViewSize());
	r.inset(frameWidth / 2., frameWidth / 2.);
	if (gradient && gradientHighlighted)
	{
		CGraphicsPath* path = getPath(context, lineWidth);
		if (path)
		{
			CGradient* drawGradient = highlight ? gradientHighlighted : gradient;
			if (drawGradient)
				context->fillLinearGradient(path, *drawGradient, r.getTopLeft(), r.getBottomLeft(), false);
			context->drawGraphicsPath(path, CDrawContext::kPathStroked);
		}
	}
	CRect titleRect = getViewSize();
	titleRect.inset(frameWidth / 2., frameWidth / 2.);

	CBitmap* iconToDraw = highlight ? (iconHighlighted ? iconHighlighted : icon) : (icon ? icon : iconHighlighted);
	CDrawMethods::drawIconAndText(context, iconToDraw, iconPosition, getTextAlignment(), getTextMargin(), titleRect, title, getFont(), highlight ? getTextColorHighlighted() : getTextColor());
	setDirty(false);
}

CMouseEventResult TextButtonEx::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	if (isEditing())
	{
		if (style == kKickStyle)
			value = 0.0;

		if (value != entryState)
		{
			entryState = value;
			if (style == kKickStyle)
			{
				value = getMin();  // set button to UNSELECTED state
				if (mouseBehavior == mouseDirUpAndDown || mouseBehavior == mouseDirUp)
				{
					// endEdit ();
					valueChanged();
					setDirty();
				}
			}
			else
				valueChanged();

			if (isDirty())
				invalid();
		}
		endEdit();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult TextButtonEx::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if (style == kKickStyle)
	{
		value = 1.0;
	}

	entryState = value;
	beginEdit();

	if (style == kKickStyle)
	{
		if (mouseBehavior == mouseDirUpAndDown || mouseBehavior == mouseDirDown)
			valueChanged();
	}

	return onMouseMoved(where, buttons);
}


CAnimKnobEx::CAnimKnobEx (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint &offset, bool bSwitchKnob)
: CAnimKnob (size, listener, tag, subPixmaps, heightOfOneImage, background, offset)
, switchKnob(bSwitchKnob)
{
	maxControlValue = 1.0;
    aaxKnob = false;
}
CAnimKnobEx::~CAnimKnobEx(void)
{

}

void CAnimKnobEx::draw(CDrawContext* pContext)
{
	if(getDrawBackground ())
	{
		CPoint where (0, 0);

		// --- mormalize to the switch for clicky behaviour
		if(switchKnob)
		{
			value *= maxControlValue;
            value = int(value);// + 0.5f);
			value /= maxControlValue;
		}

		if(value >= 0.f && heightOfOneImage > 0.)
		{
			CCoord tmp = heightOfOneImage * (getNumSubPixmaps () - 1);
			if (bInverseBitmap)
				where.y = floor ((1. - value) * tmp);
			else
				where.y = floor (value * tmp);
			where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
		}

		// --- draw it
		getDrawBackground()->draw(pContext, getViewSize(), where);

	}

	setDirty (false);
}

bool CAnimKnobEx::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXKnob() ? kAlt : kDefaultValueModifier;

#if TARGET_OS_IPHONE
    if (button.isDoubleClick ())
#else
        if (button.isLeftButton () && button.getModifierState () == modder)
#endif
        {
            float defValue = getDefaultValue ();
            if (defValue != getValue ())
            {
                // begin of edit parameter
                beginEdit ();

                setValue (defValue);
                valueChanged ();

                // end of edit parameter
                endEdit ();
            }
            return true;
        }
    return false;
}

CMouseEventResult CAnimKnobEx::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!buttons.isLeftButton ())
        return kMouseEventNotHandled;

    beginEdit ();

    if(checkDefaultValue(buttons))
    {
        endEdit ();
        return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
    }

    firstPoint = where;
    lastPoint (-1, -1);
    startValue = getOldValue ();

    modeLinear = false;
    fEntryState = value;
    range = kCKnobRange;
    coef = (getMax () - getMin ()) / range;
    oldButton = buttons;

    int32_t mode    = kCircularMode;
    int32_t newMode = getFrame()->getKnobMode();
    int32_t zoomer = isAAXKnob() ? kControl : kZoomModifier;

    if(kLinearMode == newMode)
    {
        if(!(buttons & kAlt))
            mode = newMode;
    }
    else if(buttons & kAlt)
    {
        mode = kLinearMode;
    }

    if (mode == kLinearMode)
    {
        if (buttons & zoomer)
            range *= zoomFactor;
        lastPoint = where;
        modeLinear = true;
        coef = (getMax () - getMin ()) / range;
    }
    else
    {
        CPoint where2 (where);
        where2.offset (-getViewSize ().left, -getViewSize ().top);
        startValue = valueFromPoint (where2);
        lastPoint = where;
    }

    return kMouseEventHandled;
}

CMouseEventResult CAnimKnobEx::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CAnimKnobEx::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
    if(isEditing())
    {
        float middle = (getMax () - getMin ()) * 0.5f;

        int32_t zoomer = isAAXKnob() ? kControl : kZoomModifier;

        if (where != lastPoint)
        {
            lastPoint = where;
            if (modeLinear)
            {
                CCoord diff = (firstPoint.y - where.y) + (where.x - firstPoint.x);
                if (buttons != oldButton)
                {
                    range = kCKnobRange;
                    if (buttons & zoomer)
                        range *= zoomFactor;

                    float coef2 = (getMax () - getMin ()) / range;
                    fEntryState += (float)(diff * (coef - coef2));
                    coef = coef2;
                    oldButton = buttons;
                }
                value = (float)(fEntryState + diff * coef);
                bounceValue ();
            }
            else
            {
                where.offset (-getViewSize ().left, -getViewSize ().top);
                value = valueFromPoint (where);
                if (startValue - value > middle)
                    value = getMax ();
                else if (value - startValue > middle)
                    value = getMin ();
                else
                    startValue = value;
            }
            if (value != getOldValue ())
                valueChanged ();
            if (isDirty ())
                invalid ();
        }
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

void CAnimKnobEx::valueChanged()
{
	CControl::valueChanged();
}

CVerticalSliderEx::CVerticalSliderEx(const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CVerticalSlider(rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
	switchSlider = false;
    aaxSlider = false;
	maxControlValue = 1.0;
}

CVerticalSliderEx::~CVerticalSliderEx()
{

}

bool CVerticalSliderEx::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXSlider() ? kAlt : kDefaultValueModifier;

#if TARGET_OS_IPHONE
    if (button.isDoubleClick ())
#else
        if (button.isLeftButton () && button.getModifierState () == modder)
#endif
        {
            float defValue = getDefaultValue ();
            if (defValue != getValue ())
            {
                // begin of edit parameter
                beginEdit ();

                setValue (defValue);
                valueChanged ();

                // end of edit parameter
                endEdit ();
            }
            return true;
        }
    return false;
}

CMouseEventResult CVerticalSliderEx::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    CRect handleRect;
    delta = calculateDelta (where, getMode () != kFreeClickMode ? &handleRect : 0);
    if (getMode () == kTouchMode && !handleRect.pointInside (where))
        return kMouseEventNotHandled;

    oldVal    = getMin () - 1;
    oldButton = buttons;

    if ((getMode () == kRelativeTouchMode && handleRect.pointInside (where)) || getMode () != kRelativeTouchMode)
    {
        if(checkDefaultValue(buttons))
        {
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
        }
    }

    startVal = getValue ();
    beginEdit ();
    mouseStartPoint = where;

    int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

    if (buttons & zoomer)
        return kMouseEventHandled;

    return onMouseMoved (where, buttons);
}

CMouseEventResult CVerticalSliderEx::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    oldButton = 0;
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CVerticalSliderEx::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	CRect rect = getViewSize();
	CPoint siderSize = getSliderSize();
	CPoint offsetHandle = getOffsetHandle();
	double widthOfSlider = siderSize.x;
	double heightOfSlider = siderSize.y;

	float rangeHandle = 1.0;

	if (getStyle() & kHorizontal)
	{
		rangeHandle = rect.getWidth() - (widthOfSlider + offsetHandle.x * 2);
	}
	else
	{
		rangeHandle = rect.getHeight() - (heightOfSlider + offsetHandle.y * 2);
	}

	float zoomFactor = getZoomFactor();

	if (isEditing())
	{
		if (buttons & kLButton)
		{
			if (oldVal == getMin() - 1)
				oldVal = (value - getMin()) / getRange();

			int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

			if ((oldButton != buttons) && (buttons & zoomer))
			{
				oldVal = (value - getMin()) / getRange();
				oldButton = buttons;
			}
			else if (!(buttons & zoomer))
				oldVal = (value - getMin()) / getRange();

			float normValue;
			if (getStyle() & kHorizontal)
				normValue = (float)(where.x - delta) / (float)rangeHandle;
			else
				normValue = (float)(where.y - delta) / (float)rangeHandle;

			if (getStyle() & kRight || getStyle() & kBottom)
				normValue = 1.f - normValue;

			if (buttons & zoomer)
				normValue = oldVal + ((normValue - oldVal) / zoomFactor);

			setValueNormalized(normValue);

			if (isDirty())
			{
				valueChanged();
				invalid();
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}



CHorizontalSliderEx::CHorizontalSliderEx (const CRect &rect, IControlListener* listener, int32_t tag, int32_t iMinPos, int32_t iMaxPos, CBitmap* handle, CBitmap* background, const CPoint& offset, const int32_t style)
: CHorizontalSlider(rect, listener, tag, iMinPos, iMaxPos, handle, background, offset, style)
{
	switchSlider = false;
    aaxSlider = false;
	maxControlValue = 1.0;
}
CHorizontalSliderEx::~CHorizontalSliderEx()
{

}


bool CHorizontalSliderEx::checkDefaultValue (CButtonState button)
{
    int32_t modder = isAAXSlider() ? kAlt : kDefaultValueModifier;

#if TARGET_OS_IPHONE
    if (button.isDoubleClick ())
#else
        if (button.isLeftButton () && button.getModifierState () == modder)
#endif
        {
            float defValue = getDefaultValue ();
            if (defValue != getValue ())
            {
                // begin of edit parameter
                beginEdit ();

                setValue (defValue);
                valueChanged ();

                // end of edit parameter
                endEdit ();
            }
            return true;
        }
    return false;
}

CMouseEventResult CHorizontalSliderEx::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (!(buttons & kLButton))
        return kMouseEventNotHandled;

    CRect handleRect;
    delta = calculateDelta (where, getMode () != kFreeClickMode ? &handleRect : 0);
    if (getMode () == kTouchMode && !handleRect.pointInside (where))
        return kMouseEventNotHandled;

    oldVal    = getMin () - 1;
    oldButton = buttons;

    if ((getMode () == kRelativeTouchMode && handleRect.pointInside (where)) || getMode () != kRelativeTouchMode)
    {
        if(checkDefaultValue(buttons))
        {
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
        }
    }

    startVal = getValue ();
    beginEdit ();
    mouseStartPoint = where;

    int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

    if (buttons & zoomer)
        return kMouseEventHandled;

    return onMouseMoved (where, buttons);
}

CMouseEventResult CHorizontalSliderEx::onMouseUp(CPoint& where, const CButtonState& buttons)
{
    oldButton = 0;
    endEdit ();
    return kMouseEventHandled;
}

CMouseEventResult CHorizontalSliderEx::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	float zoomFactor = getZoomFactor();
	CRect rect = getViewSize();
	CPoint siderSize = getSliderSize();
	CPoint offsetHandle = getOffsetHandle();
	double widthOfSlider = siderSize.x;
	double heightOfSlider = siderSize.y;

	float rangeHandle = 1.0;

	if (getStyle() & kHorizontal)
	{
		rangeHandle = rect.getWidth() - (widthOfSlider + offsetHandle.x * 2);
	}
	else
	{
		rangeHandle = rect.getHeight() - (heightOfSlider + offsetHandle.y * 2);
	}


	if (isEditing())
	{
		if (buttons & kLButton)
		{
			if (oldVal == getMin() - 1)
				oldVal = (value - getMin()) / getRange();

			int32_t zoomer = isAAXSlider() ? kControl : kZoomModifier;

			if ((oldButton != buttons) && (buttons & zoomer))
			{
				oldVal = (value - getMin()) / getRange();
				oldButton = buttons;
			}
			else if (!(buttons & zoomer))
				oldVal = (value - getMin()) / getRange();

			float normValue;
			if (getStyle() & kHorizontal)
				normValue = (float)(where.x - delta) / (float)rangeHandle;
			else
				normValue = (float)(where.y - delta) / (float)rangeHandle;

			if (getStyle() & kRight || getStyle() & kBottom)
				normValue = 1.f - normValue;

			if (buttons & zoomer)
				normValue = oldVal + ((normValue - oldVal) / zoomFactor);

			setValueNormalized(normValue);

			if (isDirty())
			{
				valueChanged();
				invalid();
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}


CMeterDetector::CMeterDetector(void)
{
    attackTime_mSec = 0.0;
    releaseTime_mSec = 0.0;
    attackTime = 0.0;
    m_fReleaseTime = 0.0;
    detectorSampleRate = 44100;
    envelope = 0.0;
    detectMode = 0;
    peakEnvelope = -1.0;
    peakHold = false;
    analogTC = false;
    logDetector = false;
}

CMeterDetector::~CMeterDetector(void)
{
}

void CMeterDetector::prepareForPlay()
{
    envelope = 0.0;
}

void CMeterDetector::init(float samplerate, float attack_in_ms, float release_in_ms, bool bAnalogTC, unsigned int uDetect, bool bLogDetector)
{
    envelope = 0.0;
    detectorSampleRate = samplerate;
    analogTC = bAnalogTC;
    attackTime_mSec = attack_in_ms;
    releaseTime_mSec = release_in_ms;
    detectMode = uDetect;
    logDetector = bLogDetector;

    // set themdetectMode = uDetect;
    setAttackTime(attack_in_ms);
    setReleaseTime(release_in_ms);
}

void CMeterDetector::setAttackTime(float attack_in_ms)
{
    attackTime_mSec = attack_in_ms;

    if(analogTC)
        attackTime = (float)exp(ENVELOPE_ANALOG_TC/( attack_in_ms * detectorSampleRate * 0.001f));
    else
        attackTime = (float)exp(ENVELOPE_DIGITAL_TC/( attack_in_ms * detectorSampleRate * 0.001f));
}

void CMeterDetector::setReleaseTime(float release_in_ms)
{
    releaseTime_mSec = release_in_ms;

    if(analogTC)
        m_fReleaseTime = (float)exp(ENVELOPE_ANALOG_TC/( release_in_ms * detectorSampleRate * 0.001f));
    else
        m_fReleaseTime = (float)exp(ENVELOPE_DIGITAL_TC/( release_in_ms * detectorSampleRate * 0.001f));
}

void CMeterDetector::setTCModeAnalog(bool _analogTC)
{
    analogTC = _analogTC;
    setAttackTime(attackTime_mSec);
    setReleaseTime(releaseTime_mSec);
}


float CMeterDetector::detect(float input)
{
    switch(detectMode)
    {
        case ENVELOPE_DETECT_MODE_PEAK:
            input = (float)fabs(input);
            break;
        case ENVELOPE_DETECT_MODE_MS:
        case ENVELOPE_DETECT_MODE_RMS: // --- both MS and RMS require squaring the input
            input = (float)fabs(input) * (float)fabs(input);
            break;
        default:
            input = (float)fabs(input);
            break;
    }

    float currEnvelope = 0.0;
    if(input> envelope)
        currEnvelope = attackTime * (envelope - input) + input;
    else
        currEnvelope = m_fReleaseTime * (envelope - input) + input;

    if(currEnvelope > 0.0 && currEnvelope < FLT_MIN_PLUS) currEnvelope = 0;
    if(currEnvelope < 0.0 && currEnvelope > FLT_MIN_MINUS) currEnvelope = 0;

    // --- bound them; can happen when using pre-detector gains of more than 1.0
    currEnvelope = (float)fmin(currEnvelope, 1.f);
    currEnvelope = (float)fmax(currEnvelope,  0.f);

    // --- store envelope prior to sqrt for RMS version
    setEnvelope(currEnvelope);

    // --- if RMS, do the SQRT
    if(detectMode == ENVELOPE_DETECT_MODE_RMS)
        currEnvelope =  (float)pow(currEnvelope, 0.5f);

    // --- 16-bit scaling!
    if(logDetector)
    {
        if(currEnvelope <= 0)
            return 0;

        float fdB = 20.f*(float)log10(currEnvelope);
        fdB = (float)fmax(GUI_METER_MIN_DB, fdB);

        // --- convert to 0->1 value
        fdB += -GUI_METER_MIN_DB;
        return fdB/-GUI_METER_MIN_DB;
    }

    return envelope;
}


CVuMeterEx::CVuMeterEx(const CRect& size, CBitmap* onBitmap, CBitmap* offBitmap, int32_t nbLed, bool bInverted, bool bAnalogVU, int32_t style)
: CVuMeter(size, onBitmap, offBitmap, nbLed, style)
{
	isInverted = bInverted;
	isAnalogVU = bAnalogVU;
	subPixMaps = 80;
	heightOfOneImage = 65;
	zero_dB_Frame = 52;
}

CVuMeterEx::~CVuMeterEx(void)
{
}

void CVuMeterEx::setViewSize(const CRect& newSize, bool invalid)
{
	CControl::setViewSize (newSize, invalid);

	rectOn  = getViewSize();
	rectOff = getViewSize();
}

void CVuMeterEx::draw(CDrawContext *_pContext)
{
	if (!getOnBitmap())
		return;

	if(!isAnalogVU)
	{
		CRect _rectOn (rectOn);
		CRect _rectOff (rectOff);
		CPoint pointOn;
		CPoint pointOff;
		CDrawContext *pContext = _pContext;

		bounceValue();
        float newValue = 0.f;

        // --- for LED's as on/off single LEDs
        if (nbLed == 2)
        {
            if (value < 0.5f)
                newValue = 0.f;
            else
                newValue = 1.f;
        }
        else
        {
            newValue = getOldValue() - decreaseValue;
            if (newValue < value)
                newValue = value;
            setOldValue(newValue);

            // --- apply detector *after* storing value
            newValue = detector.detect(newValue);
        }

        if (style & kHorizontal)
		{
			if(!isInverted)
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMin () + newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getWidth ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOff(tmp, 0);
				_rectOff.left += tmp;
				_rectOn.right = tmp + rectOn.left;
			}
			else
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getWidth ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOn(tmp, 0);
				_rectOn.left += tmp;
				_rectOff.right = tmp + _rectOff.left;
			}
		}
		else
		{
			if(!isInverted)
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (getMax () - newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getHeight ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOn(0, tmp);
				_rectOff.bottom = tmp + rectOff.top;
				_rectOn.top += tmp;
			}
			else
			{
				CCoord tmp = (CCoord)(((int32_t)(nbLed * (newValue) + 0.5f) / (float)nbLed) * getOnBitmap()->getHeight ());
				// http://ehc.ac/p/vstgui/mailman/vstgui-devel/?page=43
				// [Vstgui-devel] CVuMeter display bug on Windows
				tmp = (CCoord)((long)(tmp + 0.5f));
				if(tmp < 1.0) tmp = 0.0;

				pointOff (0, tmp);
				_rectOn.bottom = tmp + _rectOn.top;
				_rectOff.top     += tmp;
			}
		}
		if(getOffBitmap())
			getOffBitmap()->draw (pContext, _rectOff, pointOff);

		getOnBitmap()->draw(pContext, _rectOn, pointOn);
	}
	else
	{
		if(getDrawBackground())
		{
            bounceValue();

            float newValue = getOldValue() - decreaseValue;
            if (newValue < value)
                newValue = value;
            setOldValue(newValue);

            // --- apply detector *after* storing value
            newValue = detector.detect(newValue);


			CPoint where (0, 0);
			if (value >= 0.f && heightOfOneImage > 0.)
			{
				CCoord tmp = heightOfOneImage * (subPixMaps - 1);
				if(isInverted)
				{
					double dTop = zero_dB_Frame/subPixMaps;
					where.y = floor ((dTop - newValue*dTop) * tmp);
				}
				else
					where.y = floor (newValue * tmp);
				where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
			}

			getDrawBackground()->draw (_pContext, getViewSize (), where);
		}
	}

	setDirty (false);
}
CXYPadEx::CXYPadEx(const CRect& size)
: CXYPad(size)
{
	vertX[0] = 0.0;
	vertY[0] = 0.5;

	vertX[1] = 0.5;
	vertY[1] = 0.0;

	vertX[2] = 1.0;
	vertY[2] = 0.5;

	vertX[3] = 0.5;
	vertY[3] = 1.0;

	lastX = 0.0;
	lastY = 0.0;

	tagX = -1;
	tagY = -1;

	isJoystickPad = false;
}

void CXYPadEx::setValue(float val)
{
	if(isJoystickPad)
	{
		float x, y;
		calculateXY(val, x, y);
		if(!isEditing())
			y = -1.f*y + 1.f;

		// --- this bounds to the diamond
		if(pointInPolygon(4, &vertX[0], &vertY[0], x, y))
		{
			val = calculateValue(x, y);
			CXYPad::setValue(val);
		}/// else ignore
        return;
	}

    CXYPad::setValue(val);
    /*
	else if(isEditing())
		CXYPad::setValue(val);
	else
	{
		float x, y;
		calculateXY(val, x, y);
		y = -1.f*y + 1.f;
		boundValues(x, y);
		val = calculateValue(x, y);
		CXYPad::setValue(val);
	}*/
}

CMouseEventResult CXYPadEx::onMouseDown(CPoint& where, const CButtonState& buttons)
{
	if(buttons.isLeftButton())
	{
		mouseChangeStartPoint = where;
		mouseChangeStartPoint.offset (-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);

		onMouseMoved (where, buttons);

		if(!isEditing())
		{
			beginEdit ();
		}

		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

CMouseEventResult CXYPadEx::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	CXYPad::onMouseUp(where, buttons);
	return kMouseEventHandled;
}
CMouseEventResult CXYPadEx::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
	if(!isJoystickPad)
		return CXYPad::onMouseMoved(where, buttons);

	if(buttons.isLeftButton ())
	{
		if(stopTrackingOnMouseExit)
		{
			if(!hitTest(where, buttons))
			{
				endEdit ();
				return kMouseMoveEventHandledButDontNeedMoreEvents;
			}
		}

		float x, y;
		CCoord width = getWidth() - getRoundRectRadius ();
		CCoord height = getHeight() - getRoundRectRadius ();
		where.offset(-getViewSize ().left - getRoundRectRadius() / 2., -getViewSize ().top - getRoundRectRadius() / 2.);

		x = (float)(where.x / width);
		y = (float)(where.y / height);

		// --- this bounds to the diamond
		if(!pointInPolygon(4, &vertX[0], &vertY[0], x, y))
		{
            x = lastX;
            y = lastY;

			setValue(calculateValue (x, y));
			if(listener && isDirty())
				listener->valueChanged(this);

			invalid ();
			lastMouseChangePoint = where;
			return kMouseEventHandled;
		}

		boundValues (x, y);

		lastX = x;
		lastY = y;

		setValue(calculateValue (x, y));
		if(listener && isDirty())
			listener->valueChanged(this);

		invalid ();
		lastMouseChangePoint = where;
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

void CXYPadEx::draw(CDrawContext* context)
{
	if(!isJoystickPad)
		return CXYPad::draw(context);

	const CRect& xyRect = getViewSize();

    CDrawContext::PointList pointList;
    CPoint pVertex0( xyRect.left, xyRect.top + 0.5*(float)(xyRect.getHeight()) );
    CPoint pVertex1( xyRect.left + 0.5*(float)(xyRect.getWidth()), xyRect.top );
    CPoint pVertex2( xyRect.right, xyRect.top + 0.5*(float)(xyRect.getHeight()) );
    CPoint pVertex3( xyRect.left + 0.5*(float)(xyRect.getWidth()), xyRect.bottom );
    CPoint pVertex4( xyRect.left, xyRect.top + 0.5*(float)(xyRect.getHeight()) );

    pointList.push_back(pVertex0);
    pointList.push_back(pVertex1);
    pointList.push_back(pVertex2);
    pointList.push_back(pVertex3);
    pointList.push_back(pVertex4);

    context->setLineWidth(2); // 2/win and 1/mac
    context->setFillColor(getBackColor());
    context->setFrameColor(getFrameColor());
    context->drawPolygon(pointList, kDrawFilledAndStroked);

    // --- now the axes(?)
    double centerX = xyRect.left + xyRect.getWidth()/2.0;
	double centerY = xyRect.top + xyRect.getHeight() / 2.0;

    context->setLineWidth(1);
    const CPoint p1(centerX, xyRect.top);
    const CPoint p2(centerX, xyRect.bottom);
    const CPoint p3(xyRect.left, centerY);
    const CPoint p4(xyRect.right, centerY);
    context->drawLine(p1, p2);
    context->drawLine(p3, p4);

	// --- this draws the puck
	float x, y;
	calculateXY(getValue(), x, y);

	CCoord width = getWidth() - getRoundRectRadius ();
	CCoord height = getHeight() - getRoundRectRadius ();
	CRect r(x*width, y*height, x*width, y*height);
	r.inset(-getRoundRectRadius()/2., -getRoundRectRadius()/2.);
	r.offset(getViewSize().left + getRoundRectRadius()/2., getViewSize().top + getRoundRectRadius()/2.);
	context->setFillColor(getFontColor());
	context->setDrawMode(kAntiAliasing);
	context->drawEllipse(r, kDrawFilled);
	setDirty(false);
}











}
