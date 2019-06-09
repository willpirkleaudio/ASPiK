// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  guiconstants.h
//
/**
    \file   guiconstants.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  globally utilized constants and enumerations
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef _guiconstants_h
#define _guiconstants_h

#define _MATH_DEFINES_DEFINED

#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>

// --- RESERVED PARAMETER ID VALUES
const unsigned int PLUGIN_SIDE_BYPASS = 131072; ///<RESERVED PARAMETER ID VALUE
const unsigned int XY_TRACKPAD = 131073;		///<RESERVED PARAMETER ID VALUE
const unsigned int VECTOR_JOYSTICK = 131074;	///<RESERVED PARAMETER ID VALUE
const unsigned int PRESET_NAME = 131075;		///<RESERVED PARAMETER ID VALUE
const unsigned int WRITE_PRESET_FILE = 131076;	///<RESERVED PARAMETER ID VALUE
const unsigned int SCALE_GUI_SIZE = 131077;		///<RESERVED PARAMETER ID VALUE
// --- 131078 -through- 131999 are RESERVED		///<RESERVED PARAMETER ID VALUE

// --- custom views may be added using the base here: e.g.
//     const unsigned int CUSTOM_SPECTRUM_VIEW = CUSTOM_VIEW_BASE + 1;
const unsigned int CUSTOM_VIEW_BASE = 132000;	///<ID values for Custom Views (not necessarily required)

// --- enum for the GUI object's message processing
enum { tinyGUI, verySmallGUI, smallGUI, normalGUI, largeGUI, veryLargeGUI };	///< GUI scaling constants

/**
@isReservedTag
\ingroup ASPiK-GUI

@brief check to see if a tag is reserved: ASPiK defines several reserved control ID values.

\param tag - the tag to check
\return true if is reserved, false otherwise
*/
inline bool isReservedTag(int tag)
{
	// --- these are reserved
	if (tag >= 131072 && tag <= 131999)
		return true;
	return false;
}

/**
@isBonusParameter
\ingroup ASPiK-GUI

@brief check to see if a tag is Bonus Parameter: these should NOT be added to API parameter lists

NOTE: PLUGIN_SIDE_BYPASS is not a bonus; it is used in VST3 as the bypass flag
\param tag - the tag to check
\return true if is Bonus Parameter, false otherwise
*/
inline bool isBonusParameter(int tag)
{
	// --- these are reserved
	if (tag == XY_TRACKPAD || 
		tag == VECTOR_JOYSTICK ||
		tag == PRESET_NAME || 
		tag == WRITE_PRESET_FILE || 
		tag == SCALE_GUI_SIZE)
		return true;

	return false;
}

// --- typed enumeration helpers
/**
@enumToInt
\ingroup ASPiK-GUI
\def enumToInt
@brief macro helper to cast a typed enum to an int

\param ENUM - the typed enum to convert
\return the enum properly cast as an int (the true underlying datatype for enum class)
*/
#define enumToInt(ENUM) static_cast<int>(ENUM)

/**
@compareEnumToInt
\ingroup ASPiK-GUI
\def compareEnumToInt
@brief compare a typed enum value to an int

\param ENUM - the typed enum to compare with
\param INT - the int to compare with
\return true if equal false otherwise
*/
#define compareEnumToInt(ENUM,INT) (static_cast<int>(ENUM) == (INT))

/**
@compareIntToEnum
\ingroup ASPiK-GUI
\def compareIntToEnum

@brief compare a typed enum value to an int

\param INT - the int to compare with
\param ENUM - the typed enum to compare with
\return true if equal false otherwise
*/
#define compareIntToEnum(INT,ENUM) ((INT) == static_cast<int>(ENUM))

/**
@convertIntToEnum
\ingroup ASPiK-GUI
\def convertIntToEnum

@brief convert an int to an enum, e.g. for passing to functions

\param INT - the int to compare with
\param ENUM - the typed enum to compare with
\return the int value properly cast as the enum type
*/
#define convertIntToEnum(INT,ENUM) static_cast<ENUM>(INT)

/**
@kCTCoefficient
\ingroup Constants-Enums
@brief concave and/or convex transform correction factor
*/
const double kCTCoefficient = 5.0 / 12.0;

/**
@kCTCorrFactorZero
\ingroup Constants-Enums
@brief concave/convex transform correction factor at x = 0
*/
const double kCTCorrFactorZero = pow(10.0, (-1.0/kCTCoefficient));

/**
@kCTCorrFactorAnitZero
\ingroup Constants-Enums
@brief inverse concave/convex transform factor at x = 0
*/
const double kCTCorrFactorAnitZero = 1.0 / (1.0 - kCTCorrFactorZero);

/**
@kCTCorrFactorUnity
\ingroup Constants-Enums
@brief concave/convex transform correction factor at x = 1
*/
const double kCTCorrFactorUnity = 1.0 / (1.0 + kCTCoefficient*log10(1.0 + kCTCorrFactorZero));

/**
@kCTCorrFactorAntiUnity
\ingroup Constants-Enums
@brief inverse concave/convex transform correction factor at x = 1
*/
const double kCTCorrFactorAntiUnity = 1.0 / (1.0 + (-pow(10.0, (-1.0/kCTCoefficient))));

/**
@kCTCorrFactorAntiLog
\ingroup Constants-Enums
@brief concave/convex transform correction factor
*/
const double kCTCorrFactorAntiLog = kCTCoefficient*log10(1.0 + kCTCorrFactorZero);

/**
@kCTCorrFactorAntiLogScale
\ingroup Constants-Enums
@brief concave/convex transform scaling factor
*/
const double kCTCorrFactorAntiLogScale = 1.0 / (-kCTCoefficient*log10(kCTCorrFactorZero) + kCTCorrFactorAntiLog);

/**
@kPi
\ingroup Constants-Enums
@brief pi to 80 decimal places
*/
const double kPi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;

/**
@kTwoPi
\ingroup Constants-Enums
@brief 2pi to 80 decimal places
*/
const double kTwoPi = 2.0*3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;

//-----------------------------------------------------------------------------
/// @name Audio Detector Constants
//-----------------------------------------------------------------------------
/** @AudioDetectorConstants
\ingroup Constants-Enums @{*/
// ---
const uint32_t ENVELOPE_DETECT_MODE_PEAK = 0;	///< |x|
const uint32_t ENVELOPE_DETECT_MODE_MS = 1;		///< (1/N)|x|^2
const uint32_t ENVELOPE_DETECT_MODE_RMS = 2;	///< SQRT((1/N)|x|^2)
const uint32_t ENVELOPE_DETECT_MODE_NONE = 3;	///< not used

const float ENVELOPE_DIGITAL_TC = -4.6051701859880913680359829093687;///< ln(1%)
const float ENVELOPE_ANALOG_TC = -1.0023934309275667804345424248947; ///< ln(36.7%)
/** @} */

/** @GUITiming
\ingroup Constants-Enums @{*/
// ---
const float GUI_METER_UPDATE_INTERVAL_MSEC = 50.f;	///< repaint interval; larger = slower
const float GUI_METER_MIN_DB = -60.f;				///< min GUI value in dB
/** @} */

/** \ingroup Constants-Enums */
#define FLT_EPSILON_PLUS      1.192092896e-07        ///< /* smallest such that 1.0+FLT_EPSILON != 1.0 */
/** \ingroup Constants-Enums */
#define FLT_EPSILON_MINUS    -1.192092896e-07        ///< /* smallest such that 1.0-FLT_EPSILON != 1.0 */
/** \ingroup Constants-Enums */
#define FLT_MIN_PLUS          1.175494351e-38        ///< /* min positive value */
/** \ingroup Constants-Enums */
#define FLT_MIN_MINUS        -1.175494351e-38        ///< /* min negative value */

// --- for math.h constants
#define _MATH_DEFINES_DEFINED

/**
\enum smoothingMethod
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the smoothing type

- enum class smoothingMethod { kLinearSmoother, kLPFSmoother };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class smoothingMethod { kLinearSmoother, kLPFSmoother };

/**
\enum taper
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the control taper

- enum class taper { kLinearTaper, kLogTaper, kAntiLogTaper, kVoltOctaveTaper };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class taper { kLinearTaper, kLogTaper, kAntiLogTaper, kVoltOctaveTaper };

/**
\enum meterCal
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set meter calibration.

- enum class meterCal { kLinearMeter, kLogMeter };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class meterCal { kLinearMeter, kLogMeter };

/**
\enum controlVariableType
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the control's behavior; this tells the PluginParameter object
how to interpret the control information (e.g. as float versus int).\n\n
Note that you can set a PluginParameter as kNonVariableBoundControl to indicate that it is not bound to any
variable.

- enum class controlVariableType { kFloat, kDouble, kInt, kTypedEnumStringList, kMeter, kNonVariableBoundControl };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class controlVariableType { kFloat, kDouble, kInt, kTypedEnumStringList, kMeter, kNonVariableBoundControl };


/**
\enum boundVariableType
\ingroup Constants-Enums
\brief
Use this strongly typed enum to easily set the control's linked variable datatype (for automatic variable binding).

- enum boundVariableType { kFloat, kDouble, kInt, kUInt };

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum class boundVariableType { kFloat, kDouble, kInt, kUInt };

/**
\class ParamSmoother
\ingroup ASPiK-GUI
\ingroup ASPiK-Core
\brief
The ParamSmoother object performs parameter smoothing on GUI control information. You can choose linear or exponential smoothing.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
template <class T>
class ParamSmoother
{
public:
	ParamSmoother() { a = 0.0; b = 0.0; z = 0.0; z2 = 0.0; }

	/** set a new sample rate; this recalculates internal coefficients
	\param samplingRate the new sampling rate
	*/
	void setSampleRate(T samplingRate)
	{
		sampleRate = samplingRate;

		// --- for LPF smoother
		a = exp(-kTwoPi / (smoothingTimeInMSec * 0.001 * sampleRate));
		b = 1.0 - a;

		// --- for linear smoother
		linInc = (maxVal - minVal) / (smoothingTimeInMSec * 0.001 * sampleRate);
	}

	/** initialize the smoother; this recalculates internal coefficients
	\param smoothingTimeInMs the smoothing time in mSec to move from the two control extrema (min and max values)
	\param samplingRate the new sampling rate
	\param initValue initial (pre-smoothed) value
	\param minControlValue minimum numerical value control takes
	\param maxControlValue maximum numerical value control takes
	\param smoother type of smoothing
	*/
	void initParamSmoother(T smoothingTimeInMs,
		T samplingRate,
		T initValue,
		T minControlValue,
		T maxControlValue,
		smoothingMethod smoother = smoothingMethod::kLPFSmoother)
	{
		minVal = minControlValue;
		maxVal = maxControlValue;
		sampleRate = samplingRate;
		smoothingTimeInMSec = smoothingTimeInMs;

		setSampleRate(samplingRate);

		// --- storage
		z = initValue;
		z2 = initValue;
	}

	/**perform smoothing operation
	\param in input sample
	\param out smoothed value
	\return true if smoothing occurred, false otherwise (e.g. once control has assumed final value, smoothing is turned off)
	*/
	inline bool smoothParameter(T in, T& out)
	{
		if (smootherType == smoothingMethod::kLPFSmoother)
		{
			z = (in * b) + (z * a);
			if (z == z2)
			{
				out = in;
				return false;
			}
			z2 = z;
			out = z2;
			return true;
		}
		else // if (smootherType == smoothingMethod::kLinearSmoother)
		{
			if (in == z)
			{
				out = in;
				return false;
			}
			if (in > z)
			{
				z += linInc;
				if (z > in) z = in;
			}
			else if (in < z)
			{
				z -= linInc;
				if (z < in) z = in;
			}
			out = z;
			return true;
		}
	}

private:
	T a = 0.0;		///< a coefficient for smoothing
	T b = 0.0;		///< b coefficient for smoothing
	T z = 0.0;		///< storage register
	T z2 = 0.0;		///< storage register

	T linInc = 0.0;	///< linear stepping value

	T minVal = 0.0;	///< min extrema
	T maxVal = 1.0;	///< max exrema

	T sampleRate = 44100;	///< fs
	T smoothingTimeInMSec = 100.0; ///< tafget min-max smoothing time

	smoothingMethod smootherType = smoothingMethod::kLPFSmoother; ///< smoothing type
};


#endif
