// -----------------------------------------------------------------------------
//    ASPiK VST Shell File:  customparameters.h
//
/**
    \file   customparameters.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  definitions of log, anti-log and volt-octave custom parameter tapers

			- included in ASPiK(TM) VST Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and a
    		  VST Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#pragma once
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/base/ustring.h"
#include "guiconstants.h"
#include "math.h"

namespace Steinberg {
namespace Vst {
namespace ASPiK {

/**
\class PeakParameter
\ingroup VST-Shell
\brief
The PeakParameter object encapsulates a uni-polar parameter such as a metering variable.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included with ASPiK
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PeakParameter : public Parameter
{
public:
    PeakParameter (int32 flags, int32 id, const TChar* title);

    virtual void toString (ParamValue normValue, String128 string) const;
    virtual bool fromString (const TChar* string, ParamValue& normValue) const;
};


/**
\class LogParameter
\ingroup VST-Shell
\brief
The LogParameter object encapsulates a log parameter. Note that the standard log potentiometer in electronics is actually anti-log!

\author Will Pirkle http://www.willpirkle.com
\remark This object is included with ASPiK
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class LogParameter : public Parameter
{
public:
    LogParameter(const TChar* title, ParamID tag, const TChar* units = 0,
                 ParamValue minPlain = 0., ParamValue maxPlain = 1., ParamValue defaultValuePlain = 0.,
                 int32 stepCount = 0, int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);

    virtual void toString (ParamValue normValue, String128 string) const;
    virtual bool fromString (const TChar* string, ParamValue& normValue) const;
    virtual ParamValue toPlain(ParamValue _valueNormalized) const;
    virtual ParamValue toNormalized(ParamValue plainValue) const;
    virtual ParamValue getMin () const {return minPlain;}
    virtual void setMin (ParamValue value) {minPlain = value;}
    virtual ParamValue getMax () const {return maxPlain;}
    virtual void setMax (ParamValue value) {maxPlain = value;}

protected:
    ParamValue minPlain;
    ParamValue maxPlain;

    // fNormalizedParam = 0->1
    // returns anti-log scaled 0->1 value
    inline float calcLogParameter(float fNormalizedParam) const
    {
        if(fNormalizedParam <= 0.0) return 0.0;
        if(fNormalizedParam >= 1.0) return 1.0;

        // --- MMA Convex Transform Inverse
        return kCTCorrFactorAnitZero * ( pow (10.0, (fNormalizedParam - 1.0) / kCTCoefficient) - kCTCorrFactorZero);
    }

    // fPluginValue = 0->1 log scaled value
    // returns normal 0->1 value
    inline float calcLogPluginValue(float fPluginValue) const
    {
        if(fPluginValue <= 0.0) return 0.0;
        if(fPluginValue >= 1.0) return 1.0;

        // --- MMA Convex Transform
        return kCTCorrFactorUnity*(1.0 + kCTCoefficient*log10(fPluginValue + kCTCorrFactorZero));
    }


};

/**
\class AntiLogParameter
\ingroup VST-Shell
\brief
The AntiLogParameter object encapsulates an anti-log parameter. Note that the standard log potentiometer in electronics is actually anti-log!

\author Will Pirkle http://www.willpirkle.com
\remark This object is included with ASPiK
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class AntiLogParameter : public Parameter
{
public:
	AntiLogParameter(const TChar* title, ParamID tag, const TChar* units = 0,
					ParamValue minPlain = 0., ParamValue maxPlain = 1., ParamValue defaultValuePlain = 0.,
					int32 stepCount = 0, int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);

	virtual void toString (ParamValue normValue, String128 string) const;
	virtual bool fromString (const TChar* string, ParamValue& normValue) const;
	virtual ParamValue toPlain(ParamValue _valueNormalized) const;
	virtual ParamValue toNormalized(ParamValue plainValue) const;
	virtual ParamValue getMin () const {return minPlain;}
	virtual void setMin (ParamValue value) {minPlain = value;}
	virtual ParamValue getMax () const {return maxPlain;}
	virtual void setMax (ParamValue value) {maxPlain = value;}

protected:
	ParamValue minPlain;
	ParamValue maxPlain;

	// fNormalizedParam = 0->1
	// returns anti-log scaled 0->1 value
	inline float calcAntiLogParameter(float fNormalizedParam) const
	{
        if(fNormalizedParam <= 0.0) return 0.0;
        if(fNormalizedParam >= 1.0) return 1.0;

		// --- MMA Concave Transform Inverse
        return (kCTCorrFactorAntiUnity)*(-pow(10.0, (-fNormalizedParam / kCTCoefficient)) + 1.0);
	}

	// fPluginValue = 0->1 log scaled value
	// returns normal 0->1 value
	inline float calcAntiLogPluginValue(float fPluginValue) const
	{
        if(fPluginValue <= 0.0) return 0.0;
        if(fPluginValue >= 1.0) return 1.0;

		// --- MMA Concave Transform
        float transformed = -kCTCoefficient*kCTCorrFactorAntiLogScale*log10(1.0 - fPluginValue + kCTCorrFactorZero) + kCTCorrFactorAntiLog;
        if(transformed >= 1.0) transformed = 1.0;
        return transformed;
    }
};

/**
\class VoltOctaveParameter
\ingroup VST-Shell
\brief
The VoltOctaveParameter object encapsulates a Volt-per-Octave parameter for emulating analog synthesizer controls.
Also provide a smooth linear-in-octave control for any Frequency type of continuous control.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included with ASPiK
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class VoltOctaveParameter : public Parameter
{
public:
    VoltOctaveParameter(const TChar* title, ParamID tag, const TChar* units = 0,
                        ParamValue minPlain = 0., ParamValue maxPlain = 1., ParamValue defaultValuePlain = 0.,
                        int32 stepCount = 0, int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);

    virtual void toString (ParamValue normValue, String128 string) const;
    virtual bool fromString (const TChar* string, ParamValue& normValue) const;
    virtual ParamValue toPlain(ParamValue _valueNormalized) const;
    virtual ParamValue toNormalized(ParamValue plainValue) const;
    virtual ParamValue getMin () const {return minPlain;}
    virtual void setMin (ParamValue value) {minPlain = value;}
    virtual ParamValue getMax () const {return maxPlain;}
    virtual void setMax (ParamValue value) {maxPlain = value;}

protected:
    ParamValue minPlain;
    ParamValue maxPlain;

    // cooked to VA Scaled 0->1 param
    inline float calcVoltOctaveParameter(float fCookedParam) const
    {
        double dOctaves = log2(getMax()/getMin());
        return log2( fCookedParam/getMin() )/dOctaves;
    }

    // fPluginValue = 0->1
    // returns VA scaled version 0->1
    inline float calcVoltOctavePluginValue(float fPluginValue) const
    {
        double dOctaves = log2(getMax()/getMin());
        float fDisplay = getMin()*exp2(fPluginValue*dOctaves);
        float fDiff = getMax() - getMin();
        return (fDisplay - getMin())/fDiff;
    }
};


}}}
