// -----------------------------------------------------------------------------
//    ASPiK VST Shell File:  customparameters.cpp
//
/**
    \file   customparameters.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implementations of log, anti-log and volt-octave custom parameter tapers

			- included in ASPiK(TM) VST Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and a
    		  VST Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "customparameters.h"

namespace Steinberg {
namespace Vst {
namespace ASPiK {

//------------------------------------------------------------------------
PeakParameter::PeakParameter (int32 flags, int32 id, const TChar* title)
{
    UString (info.title, USTRINGSIZE (info.title)).assign (title);

    info.flags = flags;
    info.id = id;
    info.stepCount = 0;
    info.defaultNormalizedValue = 0.5f;
    info.unitId = kRootUnitId;

    setNormalized (1.f);
}

//------------------------------------------------------------------------
void PeakParameter::toString (ParamValue normValue, String128 string) const
{
    String str;
    if (normValue > 0.0001)
    {
        str.printf ("%.3f", (float)normValue);
    }
    else
    {
        str.assign ("-");
        str.append (kInfiniteSymbol);
    }
    str.toWideString (kCP_Utf8);
    str.copyTo16 (string, 0, 128);
}

//------------------------------------------------------------------------
bool PeakParameter::fromString (const TChar* string, ParamValue& normValue) const
{
    return false;
}



//------------------------------------------------------------------------
LogParameter::LogParameter(const TChar* title, ParamID tag, const TChar* units,
                           ParamValue minPlain, ParamValue maxPlain, ParamValue defaultValuePlain,
                           int32 stepCount, int32 flags, UnitID unitID)
: minPlain (minPlain)
, maxPlain (maxPlain)
{
    UString (info.title, tStrBufferSize (String128)).assign (title);

    UString uUnits (info.units, tStrBufferSize (String128));
    if (units)
    {
        uUnits.assign (units);
    }

    info.stepCount = stepCount;
    info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
    info.flags = flags;
    info.id = tag;
    info.unitId = unitID;
}

//------------------------------------------------------------------------
void LogParameter::toString(ParamValue normValue, String128 string) const
{
    UString128 wrapper;
    wrapper.printFloat(toPlain(normValue), precision);
    wrapper.copyTo(string, 128);
}

//------------------------------------------------------------------------
bool LogParameter::fromString (const TChar* string, ParamValue& normValue) const
{
    UString wrapper ((TChar*)string, strlen16 (string));
    if (wrapper.scanFloat (normValue))
    {
        normValue = toNormalized(normValue);
        return true;
    }
    return false;
}

// --- convert 0->1 to cooked value
ParamValue LogParameter::toPlain(ParamValue _valueNormalized) const
{
    _valueNormalized = calcLogPluginValue(_valueNormalized);
    return _valueNormalized*(getMax() - getMin()) + getMin();
}

// --- convert cooked value to 0->1 value
ParamValue LogParameter::toNormalized(ParamValue plainValue) const
{
    ParamValue normValue = (plainValue - getMin())/(getMax() - getMin());
    return calcLogParameter(normValue);
}






//------------------------------------------------------------------------
AntiLogParameter::AntiLogParameter(const TChar* title, ParamID tag, const TChar* units,
								ParamValue minPlain, ParamValue maxPlain, ParamValue defaultValuePlain,
								int32 stepCount, int32 flags, UnitID unitID)
: minPlain (minPlain)
, maxPlain (maxPlain)
{
	UString (info.title, tStrBufferSize (String128)).assign (title);

	UString uUnits (info.units, tStrBufferSize (String128));
	if (units)
	{
		uUnits.assign (units);
	}

	info.stepCount = stepCount;
	info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
	info.flags = flags;
	info.id = tag;
	info.unitId = unitID;
}

//------------------------------------------------------------------------
void AntiLogParameter::toString(ParamValue normValue, String128 string) const
{
	UString128 wrapper;
	wrapper.printFloat(toPlain(normValue), precision);
	wrapper.copyTo(string, 128);
}

//------------------------------------------------------------------------
bool AntiLogParameter::fromString (const TChar* string, ParamValue& normValue) const
{
	UString wrapper ((TChar*)string, strlen16 (string));
	if (wrapper.scanFloat (normValue))
	{
		normValue = toNormalized(normValue);
		return true;
	}
	return false;
}

// --- convert 0->1 to cooked value
ParamValue AntiLogParameter::toPlain(ParamValue _valueNormalized) const
{
	_valueNormalized = calcAntiLogPluginValue(_valueNormalized);
	return _valueNormalized*(getMax() - getMin()) + getMin();
}

// --- convert cooked value to 0->1 value
ParamValue AntiLogParameter::toNormalized(ParamValue plainValue) const
{
	ParamValue normValue = (plainValue - getMin())/(getMax() - getMin());
	return calcAntiLogParameter(normValue);
}



//------------------------------------------------------------------------
VoltOctaveParameter::VoltOctaveParameter(const TChar* title, ParamID tag, const TChar* units,
                                         ParamValue minPlain, ParamValue maxPlain, ParamValue defaultValuePlain,
                                         int32 stepCount, int32 flags, UnitID unitID)
: minPlain (minPlain)
, maxPlain (maxPlain)
{
    UString (info.title, tStrBufferSize (String128)).assign (title);

    UString uUnits (info.units, tStrBufferSize (String128));
    if (units)
    {
        uUnits.assign (units);
    }

    info.stepCount = stepCount;
    info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
    info.flags = flags;
    info.id = tag;
    info.unitId = unitID;
}

//------------------------------------------------------------------------
void VoltOctaveParameter::toString(ParamValue normValue, String128 string) const
{
    UString128 wrapper;
    wrapper.printFloat(toPlain(normValue), precision);
    wrapper.copyTo(string, 128);
}

//------------------------------------------------------------------------
bool VoltOctaveParameter::fromString (const TChar* string, ParamValue& normValue) const
{
    UString wrapper ((TChar*)string, strlen16 (string));
    if (wrapper.scanFloat (normValue))
    {
        normValue = toNormalized(normValue);
        return true;
    }
    return false;
}

// --- convert 0->1 to cooked value
ParamValue VoltOctaveParameter::toPlain(ParamValue _valueNormalized) const
{
    _valueNormalized = calcVoltOctavePluginValue(_valueNormalized);
    return _valueNormalized*(getMax() - getMin()) + getMin();
}

// --- convert cooked value to 0->1 value
ParamValue VoltOctaveParameter::toNormalized(ParamValue plainValue) const
{
	float value = calcVoltOctaveParameter(plainValue);
	value = fmax(value, 0.0);
	value = fmin(value, 1.0);
	return value;
}


}}}
