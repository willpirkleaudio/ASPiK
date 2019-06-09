// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  antilogtaperdelegate.h
//
/**
    \file   antilogtaperdelegate.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface and implementation of an anti-log taper object for
    		AAX plugins

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef	__ANTILOGTAPERDELEGATE_H
#define __ANTILOGTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()

#include "guiconstants.h"

template <typename T, int32_t RealPrecision=1000>

/**
 \class AntiLogTaperDelegate
 \ingroup AAX-Shell
 \brief
 The AntiLogTaperDelegate object encapsulates an anti-log parameter. Note that the standard log potentiometer in electronics is actually anti-log!
 This is used in the construction of AAX parameters

 NOTES:
 - uses MMA Concave Transform & Inverse Concave Transform for anti-log bahavior

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included with ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class AntiLogTaperDelegate : public AAX_ITaperDelegate<T>
{
public:
	AntiLogTaperDelegate(T minValue=0, T maxValue=1);

	// --- Virtual Overrides
	AntiLogTaperDelegate<T, RealPrecision>*	Clone() const;
	T		GetMinimumValue() const { return mMinValue; }
	T		GetMaximumValue() const	{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const;
	T		NormalizedToReal(double normalizedValue) const;
	double	RealToNormalized(T realValue) const;

protected:
	T	Round(double iValue) const;

	// --- fNormalizedParam = 0->1
	//     returns antilog scaled 0->1 value
	inline double calcAntiLogParameter(double fNormalizedParam) const
	{
        if(fNormalizedParam <= 0.0) return 0.0;
        if(fNormalizedParam >= 1.0) return 1.0;

        // --- MMA Concave Transform Inverse
        return (kCTCorrFactorAntiUnity)*(-pow(10.0, (-fNormalizedParam / kCTCoefficient)) + 1.0);
    }

	// --- fPluginValue = 0->1 antilog scaled value
	//     returns normal 0-> value
	inline double calcAntiLogPluginValue(double fPluginValue) const
	{
        if(fPluginValue <= 0.0) return 0.0;
        if(fPluginValue >= 1.0) return 1.0;

        // --- MMA Concave Transform
        float transformed = -kCTCoefficient*kCTCorrFactorAntiLogScale*log10(1.0 - fPluginValue + kCTCorrFactorZero) + kCTCorrFactorAntiLog;
        if(transformed >= 1.0) transformed = 1.0;
        return transformed;
	}

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	AntiLogTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
AntiLogTaperDelegate<T, RealPrecision>::AntiLogTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
AntiLogTaperDelegate<T, RealPrecision>*		AntiLogTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AntiLogTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		AntiLogTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
{
	if (RealPrecision)
		value = Round(value);		//reduce the precision to get proper rounding behavior with integers.

	if (value > mMaxValue)
		return mMaxValue;
	if (value < mMinValue)
		return mMinValue;
	return value;
}

template <typename T, int32_t RealPrecision>
T		AntiLogTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	normalizedValue = calcAntiLogPluginValue(normalizedValue);
	double doubleRealValue = normalizedValue*(mMaxValue - mMinValue) + mMinValue;

	T realValue = (T) doubleRealValue;

    return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	AntiLogTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	double normValue = (realValue - mMinValue)/(mMaxValue - mMinValue);
	return calcAntiLogParameter(normValue);
}

#endif // __ANTILOGTAPERDELEGATE_H
