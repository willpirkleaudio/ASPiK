// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  voltoctavetaperdelegate.h
//
/**
    \file   voltoctavetaperdelegate.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface and implementation of a volt/octave taper object for
    		AAX plugins

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef	__VOLTOCTAVETAPERDELEGATE_H
#define __VOLTOCTAVETAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX_UtilsNative.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor(), log()

template <typename T, int32_t RealPrecision=1000>

/**
 \class AntiLogTaperDelegate
 \ingroup AAX-Shell
 \brief
 The AntiLogTaperDelegate object encapsulates an anti-log parameter. Note that the standard log potentiometer in electronics is actually anti-log!
 This is used in the construction of AAX parameters

 NOTES:
 - uses uses Will Pirkle's volt/octave calcuation

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included with ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class VoltOctaveTaperDelegate : public AAX_ITaperDelegate<T>
{
public:
		VoltOctaveTaperDelegate(T minValue=0, T maxValue=1);

	// --- Virtual Overrides
	VoltOctaveTaperDelegate<T, RealPrecision>*	Clone() const;
	T		GetMinimumValue() const { return mMinValue; }
	T		GetMaximumValue() const { return mMaxValue; }
	T		ConstrainRealValue(T value)	const;
	T		NormalizedToReal(double normalizedValue) const;
	double	RealToNormalized(T realValue) const;

protected:
	T	Round(double iValue) const;

	// --- cooked to V/O Scaled 0->1 param
	inline double calcVoltOctaveParameter(double fCookedParam) const
	{
        double dOctaves = log2(getMax()/getMin());
        return log2( fCookedParam/getMin() )/dOctaves;
	}

	// --- fPluginValue = 0->1
	//     returns V/O scaled version 0->1
	inline double calcVoltOctavePluginValue(double fPluginValue) const
	{
        double dOctaves = log2(getMax()/getMin());
        float fDisplay = getMin()*exp2(fPluginValue*dOctaves);
        float fDiff = getMax() - getMin();
        return (fDisplay - getMin())/fDiff;
	}

    double getMin() const {return (double)mMinValue;}
    double getMax() const {return (double)mMaxValue;}

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	VoltOctaveTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
VoltOctaveTaperDelegate<T, RealPrecision>::VoltOctaveTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
VoltOctaveTaperDelegate<T, RealPrecision>*		VoltOctaveTaperDelegate<T, RealPrecision>::Clone() const
{
	return new VoltOctaveTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		VoltOctaveTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
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
T		VoltOctaveTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	normalizedValue = calcVoltOctavePluginValue(normalizedValue);
	double doubleRealValue = normalizedValue*(mMaxValue - mMinValue) + mMinValue;
	T realValue = (T) doubleRealValue;

	return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	VoltOctaveTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	return calcVoltOctaveParameter(realValue);
}

#endif // __VOLTOCTAVETAPERDELEGATE_H
