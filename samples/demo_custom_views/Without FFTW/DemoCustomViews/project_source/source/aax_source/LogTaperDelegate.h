// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  logtaperdelegate.h
//
/**
    \file   logtaperdelegate.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface and implementation of a log taper object for
    		AAX plugins

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef	__LOGTAPERDELEGATE_H
#define __LOGTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()
#include "guiconstants.h"

template <typename T, int32_t RealPrecision=1000>

/**
 \class LogTaperDelegate
 \ingroup AAX-Shell
 \brief
 The LogTaperDelegate object encapsulates a log parameter. Note that the standard log potentiometer in electronics is actually anti-log!
 This is used in the construction of AAX parameters

 NOTES:
  - uses MMA Concave Transform & Inverse Concave Transform for log bahavior

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included with ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class LogTaperDelegate : public AAX_ITaperDelegate<T>
{
public:
	LogTaperDelegate(T minValue=0, T maxValue=1);

	// --- Virtual Overrides
	LogTaperDelegate<T, RealPrecision>*	Clone() const;
	T		GetMinimumValue() const { return mMinValue; }
	T		GetMaximumValue() const { return mMaxValue; }
	T		ConstrainRealValue(T value)	const;
	T		NormalizedToReal(double normalizedValue) const;
	double	RealToNormalized(T realValue) const;

protected:
	T	Round(double iValue) const;

	// --- fNormalizedParam = 0->1
	//     returns log scaled 0->1 value
	inline double calcLogParameter(double fNormalizedParam) const
	{
        if(fNormalizedParam <= 0.0) return 0.0;
        if(fNormalizedParam >= 1.0) return 1.0;

        // --- MMA Convex Transform Inverse
        return kCTCorrFactorAnitZero * ( pow (10.0, (fNormalizedParam - 1) / kCTCoefficient) - kCTCorrFactorZero);
	}

	// --- fPluginValue = 0->1 log scaled value
	//     returns normal 0-> value
	inline double calcLogPluginValue(double fPluginValue) const
	{
        if(fPluginValue <= 0.0) return 0.0;
        if(fPluginValue >= 1.0) return 1.0;

        // --- MMA Convex Transform
        return kCTCorrFactorUnity*(1.0 + kCTCoefficient*log10(fPluginValue + kCTCorrFactorZero));
	}

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	LogTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
LogTaperDelegate<T, RealPrecision>::LogTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
LogTaperDelegate<T, RealPrecision>*		LogTaperDelegate<T, RealPrecision>::Clone() const
{
	return new LogTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		LogTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
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
T		LogTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	normalizedValue = calcLogPluginValue(normalizedValue);
	double doubleRealValue = normalizedValue*(mMaxValue - mMinValue) + mMinValue;

	T realValue = (T) doubleRealValue;

    return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	LogTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	double normValue = (realValue - mMinValue)/(mMaxValue - mMinValue);
	return calcLogParameter(normValue);
}

#endif // __LOGTAPERDELEGATE_H
