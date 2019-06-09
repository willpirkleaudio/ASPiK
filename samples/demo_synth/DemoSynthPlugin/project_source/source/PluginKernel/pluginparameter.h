// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  pluginparameter.h
//
/**
    \file   pluginparameter.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class interface file for ASPiK pluginparameter object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef _PluginParameter_H_
#define _PluginParameter_H_

#include <string>
#include <vector>
#include <sstream>
#include <atomic>
#include <map>
#include <iomanip>
#include <iostream>

#include <math.h>
#include "pluginstructures.h"
#include "guiconstants.h"


/**
\class PluginParameter
\ingroup ASPiK-Core
\ingroup ASPiK-GUI
\brief
The PluginParameter object stores all of the data needed for any type of plugin parameter.
It is a large object, but it is not complex as it really just stores LOTS of information about
plugin parameters.

PluginParameter Operations:
- store attributes of plugin parameters (numerous)
- store the actual parameter value as an atomic double
- provide access to the atomic double value as needed (and safely)
- hold the parameter smoother object
- store infinite amount of auxilliary data in numerous formats (you can easily add your own)
- consists mainly of attributes and get/set functions for each

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginParameter
{
public:
    /** constructor for continuous controls */
    PluginParameter(int _controlID, const char* _controlName, const char* _controlUnits,
                    controlVariableType _controlType, double _minValue, double _maxValue, double _defaultValue,
                    taper _controlTaper = taper::kLinearTaper, uint32_t _displayPrecision = 2);

	/** constructor 1 for string-list controls */
    PluginParameter(int _controlID, const char* _controlName, std::vector<std::string> _stringList, std::string _defaultString);

    /** constructor 2 for string-list controls */
    PluginParameter(int _controlID, const char* _controlName, const char* _commaSeparatedList, std::string _defaultString);

    /** constructor for meter controls */
    PluginParameter(int _controlID, const char* _controlName, double _meterAttack_ms, double _meterRelease_ms,
                    uint32_t _detectorMode, meterCal _meterCal = meterCal::kLinearMeter);

    /** constructor for NonVariableBoundControl */
    PluginParameter(int _controlID, const char* _controlName = "", controlVariableType _controlType = controlVariableType::kNonVariableBoundControl);

    /** empty constructor */
    PluginParameter();

   /** copy constructor */
    PluginParameter(const PluginParameter& initGuiControl);

	/** D-TOR */
    virtual ~PluginParameter();

    uint32_t getControlID() { return controlID; }			///< get ID value
    void setControlID(uint32_t cid) { controlID = cid; }	///< set ID value

    const char* getControlName() { return controlName.c_str(); }			///< get name as const char*
    void setControlName(const char* name) { controlName.assign(name); }		///< set name as const char*

    const char* getControlUnits() { return controlUnits.c_str(); }			///< get units as const char*
    void setControlUnits(const char* units) { controlUnits.assign(units); }	///< set units as const char*

    controlVariableType getControlVariableType() { return controlType; }						///< get variable type associated with parameter
    void setControlVariableType(controlVariableType ctrlVarType) { controlType = ctrlVarType; }	///< set variable type associated with parameter

    double getMinValue() { return minValue; }				///< get minimum value
    void setMinValue(double value) { minValue = value; }	///< set minimum value

    double getMaxValue() { return maxValue; }				///< get maximum value
    void setMaxValue(double value) { maxValue = value; }	///< set maximum value

    double getDefaultValue() { return defaultValue; }				///< get default value
    void setDefaultValue(double value) { defaultValue = value; }	///< set default value

	bool getIsDiscreteSwitch() { return isDiscreteSwitch; }										///< set is switch (not used)
	void setIsDiscreteSwitch(bool _isDiscreteSwitch) { isDiscreteSwitch = _isDiscreteSwitch; }	///< get is switch (not used)

	taper getControlTaper() { return controlTaper; }					///< get taper
	void setControlTaper(taper ctrlTaper) { controlTaper = ctrlTaper; }	///< set taper

    // -- taper getters
    bool isLinearTaper() { return controlTaper == taper::kLinearTaper ? true : false; }		///< query: linear taper
    bool isLogTaper() { return controlTaper == taper::kLogTaper ? true : false; }			///< query: log taper
    bool isAntiLogTaper() { return controlTaper == taper::kAntiLogTaper ? true : false; }		///< query: antilog taper
    bool isVoltOctaveTaper() { return controlTaper == taper::kVoltOctaveTaper ? true : false; }	///< query: volt/octave taper

    // --- control type getters
    bool isMeterParam() { return controlType == controlVariableType::kMeter ? true : false; }					///< query: meter param?
    bool isStringListParam() { return controlType == controlVariableType::kTypedEnumStringList ? true : false; }///< query: string list para,?
    bool isFloatParam() { return controlType == controlVariableType::kFloat ? true : false; }					///< query: float param?
    bool isDoubleParam() { return controlType == controlVariableType::kDouble ? true : false; }					///< query: double param?
    bool isIntParam() { return controlType == controlVariableType::kInt ? true : false; }						///< query: int param?
    bool isNonVariableBoundParam() { return controlType == controlVariableType::kNonVariableBoundControl ? true : false; }///< query: non-bound param?

    uint32_t getDisplayPrecision() { return displayPrecision; }						///< get sig digits
    void setDisplayPrecision(uint32_t precision) { displayPrecision = precision; }	///< set sig digits

    double getMeterAttack_ms() { return meterAttack_ms;}				///< get meter attack time (ballistics)
    void setMeterAttack_ms(double value) { meterAttack_ms = value; }	///< set meter attack time (ballistics)

    double getMeterRelease_ms() { return meterRelease_ms; }				///< get meter release time (ballistics)
    void setMeterRelease_ms(double value) { meterRelease_ms = value; }	///< set meter release time (ballistics)

    uint32_t getDetectorMode() { return detectorMode; }					///< get meter detect mode
    void setMeterDetectorMode(uint32_t value) { detectorMode = value; }	///< set meter detect mode

	bool getLogMeter() { return logMeter; }				///< query log meter flag
	void setLogMeter(bool value) { logMeter = value; }	///< set log meter flag

	bool getInvertedMeter() { return invertedMeter; }				///< query inverted meter flag
	void setInvertedMeter(bool value) { invertedMeter = value; }	///< set inverted meter flag

	bool isProtoolsGRMeter() { return protoolsGRMeter; }				///< query pro tools GR meter flag
	void setIsProtoolsGRMeter(bool value) { protoolsGRMeter = value; }	///< set inverted meter flag

	bool getParameterSmoothing() { return useParameterSmoothing; }				///< query parameter smoothing flag
    void setParameterSmoothing(bool value) { useParameterSmoothing = value; }	///< set inverted meter flag

    double getSmoothingTimeMsec() { return smoothingTimeMsec;}					///< query smoothing time
    void setSmoothingTimeMsec(double value) { smoothingTimeMsec = value; }		///< set inverted meter flag

    smoothingMethod getSmoothingMethod() { return smoothingType; }									///< query smoothing method
    void setSmoothingMethod(smoothingMethod smoothingMethod) { smoothingType = smoothingMethod; }	///< set smoothing method

    bool getIsWritable() { return isWritable; }				///< query writable control (meter)
    void setIsWritable(bool value) { isWritable = value; }	///< set writable control (meter)

    bool getEnableVSTSampleAccurateAutomation() { return enableVSTSampleAccurateAutomation; }			///< query VST3 sample accurate automation
    void setEnableVSTSampleAccurateAutomation(bool value) { enableVSTSampleAccurateAutomation = value; }///< set VST3 sample accurate automation

	// --- for aux attributes
	AuxParameterAttribute* getAuxAttribute(uint32_t attributeID);										///< get aux data
	uint32_t setAuxAttribute(uint32_t attributeID, const AuxParameterAttribute& auxParameterAtribute);	///< set aux data

	/**
	\brief the main function to access the underlying atomic double value

	\return the value as a regular double
	*/
    inline double getControlValue() { return getAtomicControlValueDouble(); }

	/**
	\brief the main function to set the underlying atomic double value

	\param actualParamValue parameter value as a regular double
	\param ignoreSmoothing flag to ignore smoothing operation and write directly to atomic double
	*/
	inline void setControlValue(double actualParamValue, bool ignoreSmoothing = false)
	{
		if (controlType == controlVariableType::kDouble ||
			controlType == controlVariableType::kFloat)
		{
			if (useParameterSmoothing && !ignoreSmoothing)
				setSmoothedTargetValue(actualParamValue);
			else
				setAtomicControlValueDouble(actualParamValue);
		}
		else
			setAtomicControlValueDouble(actualParamValue);
	}

	/**
	\brief the main function to set the underlying atomic double value using a normalized value; this is the operation in VST3 and RAFX2

	\param normalizedValue parameter value as a regular double
	\param applyTaper add the control taper during the operation
	\param ignoreParameterSmoothing flag to ignore smoothing operation and write directly to atomic double
	*/
	inline double setControlValueNormalized(double normalizedValue, bool applyTaper = true, bool ignoreParameterSmoothing = false)
	{
		// --- set according to smoothing option
		double actualParamValue = getControlValueWithNormalizedValue(normalizedValue, applyTaper);

		if (controlType == controlVariableType::kDouble ||
			controlType == controlVariableType::kFloat)
		{
			if (useParameterSmoothing && !ignoreParameterSmoothing)
				setSmoothedTargetValue(actualParamValue);
			else
				setAtomicControlValueDouble(actualParamValue);
		}
		else
			setAtomicControlValueDouble(actualParamValue);

		return actualParamValue;
	}

	/**
	\brief the main function to access the underlying atomic double value as a string

	\return the value as a std::string
	*/
	std::string getControlValueAsString();

	/**
	\brief get the number of individual strings in a string-list control

	\return number of strings
	*/
	size_t getStringCount(){return stringList.size();}

	/**
	\brief get the strings in a string-list control as a comma separated list

	\return comma separated list as a const char*
	*/
	const char* getCommaSeparatedStringList() {return commaSeparatedStringList.c_str();}

	/**
	\brief convert the string-list into a comma-separated list (during construction)
	*/
	void setCommaSeparatedStringList();

	/**
	\brief set the string-list using a vector of strings
	*/
	void setStringList(std::vector<std::string> _stringList) {stringList = _stringList;}

	/** get a string-list string using the index */
	std::string getStringByIndex(uint32_t index);

	/**
	\brief get default value as normalied value
	\return default value in normalized form
	*/
	inline double getDefaultValueNormalized()
    {
        // --- apply taper as needed
        switch (controlTaper)
        {
            case taper::kLinearTaper:
                return getNormalizedDefaultValue();

            case taper::kLogTaper:
                return getNormalizedLogDefaultValue();

            case taper::kAntiLogTaper:
                return getNormalizedAntiLogDefaultValue();

            case taper::kVoltOctaveTaper:
                return getNormalizedVoltOctaveDefaultValue();

            default:
                return 0.0;
        }
    }

	/**
	\brief get control value as normalied value
	\return control value in normalized form
	*/
	inline double getControlValueNormalized()
    {
        // --- apply taper as needed
        switch (controlTaper)
        {
            case taper::kLinearTaper:
                return getNormalizedControlValue();

            case taper::kLogTaper:
                return getNormalizedLogControlValue();

            case taper::kAntiLogTaper:
                return getNormalizedAntiLogControlValue();

            case taper::kVoltOctaveTaper:
                return getNormalizedVoltOctaveControlValue();

            default:
                return 0.0;
        }
    }

	/**
	\brief get the new control value *as if* it were set with a normalized value
	\return control value *as if* it were set with a normalized value
	*/
    inline double getControlValueWithNormalizedValue(double normalizedValue, bool applyTaper = true)
    {
        if(!applyTaper)
            return getControlValueFromNormalizedValue(normalizedValue);

        double newValue = 0;

        // --- apply taper as needed
        switch (controlTaper)
        {
            case taper::kLinearTaper:
                newValue = getControlValueFromNormalizedValue(normalizedValue);
                break;

            case taper::kLogTaper:
                newValue = getControlValueFromNormalizedValue(normToLogNorm(normalizedValue));
                break;

            case taper::kAntiLogTaper:
                newValue = getControlValueFromNormalizedValue(normToAntiLogNorm(normalizedValue));
                break;

            case taper::kVoltOctaveTaper:
                newValue = getVoltOctaveControlValueFromNormValue(normalizedValue);
                break;

            default:
                break; // --- no update
        }

        return newValue;
    }

	/**
	\brief get the new normalized control value *as if* it were set with an actual value
	\return control value *as if* it were set with a normalized value
	*/
	inline double getNormalizedControlValueWithActualValue(double actualValue) { return getNormalizedControlValueWithActual(actualValue); }

	/** get the minimum GUI value - this is ALWAYS 0.0 for VSTGUI4*/
	double getGUIMin() { return 0.0; }

	/** get the maximum GUI value for string-list params */
	double getGUIMax()
    {
        if(controlType == controlVariableType::kTypedEnumStringList)
            return (double)getStringCount() - 1;

        return 1.0;
    }

	/**
	\brief find a string in the list of a string-list parameter

	\return index of string in list
	*/
	int findStringIndex(std::string searchString)
    {
        auto it = std::find(stringList.begin (), stringList.end (), searchString);

        if (it == stringList.end())
        {
            return -1;
        }
        else
        {
            return (int)std::distance(stringList.begin(), it);
        }

        return -1;
    }

    /** normalized to Log-normalized version (convex transform) */
    inline double normToLogNorm(double normalizedValue)
    {
        return 1.0 + kCTCoefficient*log10(normalizedValue);
    }

	/** Log-normalized to normalized version (reverse-convex transform) */
    inline double logNormToNorm(double logNormalizedValue)
    {
        return pow(10.0, (logNormalizedValue - 1.0) / kCTCoefficient);
    }

     /** normalized to AntiLog-normalized version */
    inline double normToAntiLogNorm(double normalizedValue)
    {
		if (normalizedValue == 1.0)
			return 1.0;

		double aln = -kCTCoefficient*log10(1.0 - normalizedValue);
		aln = fmin(1.0, aln);
		return aln;
	}

     /** AntiLog-normalized to normalized version */
    inline double antiLogNormToNorm(double aLogNormalizedValue)
    {
        return -pow(10.0, (-aLogNormalizedValue / kCTCoefficient)) + 1.0;
    }

	/**
	\brief initialize or reset the parameter smoother object

	\param sampleRate fs (needed for coefficient calc)
	*/
	void initParamSmoother(double sampleRate)
    {
        paramSmoother.initParamSmoother(smoothingTimeMsec,
                                        sampleRate,
                                        getAtomicControlValueDouble(),
                                        minValue,
                                        maxValue,
                                        smoothingType);
    }

	/**
	\brief change any sample-rate dependent members

	\param sampleRate fs (needed for coefficient calc)
	*/
	void updateSampleRate(double sampleRate)
    {
        paramSmoother.setSampleRate(sampleRate);
    }

	/**
	\brief perform smoothing operation on data

	\return true if data was actually smoothed, false otherwise (data that has reached its terminal value will not be smoothed any further)
	*/
	bool smoothParameterValue()
    {
        if(!useParameterSmoothing) return false;
        double smoothedValue = 0.0;
        bool smoothed = paramSmoother.smoothParameter(getSmoothedTargetValue(), smoothedValue);
        if(smoothed)
			setAtomicControlValueDouble(smoothedValue);
        return smoothed;
    }

	/**
	\brief save the variable for binding operation

	\param boundVariable naked pointer to bound variable
	\param dataType data type for casting
	*/
	void setBoundVariable(void* boundVariable, boundVariableType dataType)
    {
		boundVariableDataType = dataType;

        if(dataType == boundVariableType::kDouble)
            boundVariableDouble = (double*)boundVariable;
        else if(dataType == boundVariableType::kFloat)
            boundVariableFloat = (float*)boundVariable;
        else if(dataType == boundVariableType::kInt)
            boundVariableInt = (int*)boundVariable;
        else if(dataType == boundVariableType::kUInt)
            boundVariableUInt = (uint32_t*)boundVariableUInt;

		// --- initialize it
		updateInBoundVariable();
    }

	/**
	\brief get the datatype of the bound variable

	\return boundVariableType
	*/
	boundVariableType getBoundVariableType() { return boundVariableDataType; }

	/**
	\brief perform the variable binding update (change the value)

	\return boundVariableType
	*/
	bool updateInBoundVariable()
	{
		if (boundVariableUInt)
		{
			*boundVariableUInt = (uint32_t)getControlValue();
			return true;
		}
		else if (boundVariableInt)
		{
			*boundVariableInt = (int)getControlValue();
			return true;
		}
		else if (boundVariableFloat)
		{
			*boundVariableFloat = (float)getControlValue();
			return true;
		}
		else if (boundVariableDouble)
		{
			*boundVariableDouble = getControlValue();
			return true;
		}
		return false;
	}

	/**
	\brief perform the variable binding update on meter data

	\return true if variable was udpated, false otherwise
	*/
	bool updateOutBoundVariable()
	{
		if (boundVariableUInt)
		{
			setControlValue((double)*boundVariableUInt);
			return true;
		}
		else if (boundVariableInt)
		{
			setControlValue((double)*boundVariableInt);
			return true;
		}
		else if (boundVariableFloat)
		{
			setControlValue((double)*boundVariableFloat);
			return true;
		}
		else if (boundVariableDouble)
		{
			setControlValue(*boundVariableDouble);
			return true;
		}
		return false;
	}

	/**
	\brief stores the update queue for VST3 sample accuate automation; note this is only used during actual DAW runs with automation engaged

	\param _parameterUpdateQueue the update queue to store
	*/
    void setParameterUpdateQueue(IParameterUpdateQueue* _parameterUpdateQueue) { parameterUpdateQueue = _parameterUpdateQueue; }

	/**
	\brief retrieves the update queue for VST3 sample accuate automation; note this is only used during actual DAW runs with automation engaged

	\return IParameterUpdateQueue*
	*/
	IParameterUpdateQueue* getParameterUpdateQueue() { return parameterUpdateQueue; } // may be NULL - that is OK

	/** overloaded = operator (standard C++ fare) */
	PluginParameter& operator=(const PluginParameter& aPluginParameter)	// need this override for collections to work
	{
		if (this == &aPluginParameter)
			return *this;

		controlID = aPluginParameter.controlID;
		controlName = aPluginParameter.controlName;
		controlUnits = aPluginParameter.controlUnits;
		commaSeparatedStringList = aPluginParameter.commaSeparatedStringList;
		controlType = aPluginParameter.controlType;
		minValue = aPluginParameter.minValue;
		maxValue = aPluginParameter.maxValue;
		defaultValue = aPluginParameter.defaultValue;
		controlTaper = aPluginParameter.controlTaper;
		controlValueAtomic = aPluginParameter.getAtomicControlValueFloat();
		displayPrecision = aPluginParameter.displayPrecision;
		stringList = aPluginParameter.stringList;
		useParameterSmoothing = aPluginParameter.useParameterSmoothing;
		smoothingType = aPluginParameter.smoothingType;
		smoothingTimeMsec = aPluginParameter.smoothingTimeMsec;
		meterAttack_ms = aPluginParameter.meterAttack_ms;
		meterRelease_ms = aPluginParameter.meterRelease_ms;
		detectorMode = aPluginParameter.detectorMode;
		logMeter = aPluginParameter.logMeter;
		isWritable = aPluginParameter.isWritable;
		isDiscreteSwitch = aPluginParameter.isDiscreteSwitch;
		invertedMeter = aPluginParameter.invertedMeter;

		return *this;
	}

protected:
    int controlID = -1;							///< the ID value for the parameter
    std::string controlName = "ControlName";	///< the name string for the parameter
    std::string controlUnits = "Units";			///< the units string for the parameter
    controlVariableType controlType = controlVariableType::kDouble; ///< the control type

    // --- min/max/def
    double minValue = 0.0;			///< the min for the parameter
    double maxValue = 1.0;			///< the max for the parameter
    double defaultValue = 0.0;		///< the default value for the parameter

    // --- *the* control value
    // --- atomic float as control value
    //     atomic double will not behave properly between 32/64 bit
    std::atomic<float> controlValueAtomic;		///< the underlying atomic variable

    float getAtomicControlValueFloat() const { return controlValueAtomic.load(std::memory_order_relaxed); }			///< set atomic variable with float
	void setAtomicControlValueFloat(float value) { controlValueAtomic.store(value, std::memory_order_relaxed); }	///< get atomic variable as float

    double getAtomicControlValueDouble() const { return (double)controlValueAtomic.load(std::memory_order_relaxed); }		///< set atomic variable with double
	void setAtomicControlValueDouble(double value) { controlValueAtomic.store((float)value, std::memory_order_relaxed); }	///< get atomic variable as double

    std::atomic<float> smoothedTargetValueAtomic;	///< the underlying atomic variable TARGET for smoothing
    void setSmoothedTargetValue(double value){ smoothedTargetValueAtomic.store((float)value); }	///< set atomic TARGET smoothing variable with double
    double getSmoothedTargetValue() const { return (double)smoothedTargetValueAtomic.load(); }	///< set atomic TARGET smoothing variable with double

    // --- control tweakers
    taper controlTaper = taper::kLinearTaper;	///< the taper
    uint32_t displayPrecision = 2;				///< sig digits for display

    // --- for enumerated string list
    std::vector<std::string> stringList;		///< string list
    std::string commaSeparatedStringList;		///< string list a somma separated string

    // --- gui specific
    bool appendUnits = true;					///< flag to append units in GUI controls (use with several built-in custom views)
    bool isWritable = false;					///< flag for meter variables
	bool isDiscreteSwitch = false;				///< flag for switches (not currently used in ASPiK)

    // --- for VU meters
    double meterAttack_ms = 10.0;				///< meter attack time in milliseconds
    double meterRelease_ms = 500.0;				///< meter release time in milliseconds
    uint32_t detectorMode = ENVELOPE_DETECT_MODE_RMS;///< meter detector mode
	bool logMeter = false;						///< meter is log
	bool invertedMeter = false;					///< meter is inverted
	bool protoolsGRMeter = false;				///< meter is a Pro Tools gain reduction meter

    // --- parameter smoothing
    bool useParameterSmoothing = false;			///< enable param smoothing
    smoothingMethod smoothingType = smoothingMethod::kLPFSmoother;	///< param smoothing type
    double smoothingTimeMsec = 100.0;			///< param smoothing time
    ParamSmoother<double> paramSmoother;		///< param smoothing object

	// --- variable binding
	boundVariableType boundVariableDataType = boundVariableType::kFloat;	///< bound data type

    // --- our sample accurate interface for VST3
    IParameterUpdateQueue* parameterUpdateQueue = nullptr;					///< interface for VST3 sample accurate updates

    // --- default is enabled; you can disable this for controls that have a long postUpdate cooking time
    bool enableVSTSampleAccurateAutomation = true;							///< VST3 sample accurate flag

    /**
	\brief get volt/octave control value from a normalized value

	\param normalizedValue the normalized version
	\return volt/octave version
	*/
    inline double getVoltOctaveControlValueFromNormValue(double normalizedValue)
    {
        double octaves = log2(maxValue / minValue);
        if (normalizedValue == 0)
            return minValue;

        return minValue*pow(2.0, normalizedValue*octaves);
    }

	/**
	\brief get get log control value in normalized form
	\return log control value in normalized form
	*/
    inline double getNormalizedLogControlValue()
    {
        return logNormToNorm(getNormalizedControlValue());
    }

	/**
	\brief get get anti-log control value in normalized form
	\return anti-log control value in normalized form
	*/
    inline double getNormalizedAntiLogControlValue()
    {
        return antiLogNormToNorm(getNormalizedControlValue());
    }

	/**
	\brief get get volt/octave control value in normalized form
	\return volt/octave control value in normalized form
	*/
    inline double getNormalizedVoltOctaveControlValue()
    {
        if (minValue == 0)
            return getAtomicControlValueDouble();

        return log2(getAtomicControlValueDouble() / minValue) / (log2(maxValue / minValue));
    }

	/**
	\brief convert actual control value into normalized value (helper)

	\param actualValue the actual value
	\return normalized version
	*/
	inline double getNormalizedControlValueWithActual(double actualValue)
    {
        // --- calculate normalized value from actual
		return (actualValue - minValue) / (maxValue - minValue);
	}

	/**
	\brief get control value as normalized value (helper)
	\return normalized version
	*/
	inline double getNormalizedControlValue()
    {
        // --- calculate normalized value from actual
		//double d = (getAtomicControlValueDouble() - minValue) / (maxValue - minValue);
		return (getAtomicControlValueDouble() - minValue) / (maxValue - minValue);
	}

	/**
	\brief get control value with a normalized value (helper)

	\param normalizedValue the normalized value
	\return actual version
	*/
    inline double getControlValueFromNormalizedValue(double normalizedValue)
    {
        // --- calculate the control Value using normalized input
		//double d = (maxValue - minValue)*normalizedValue + minValue;
		return (maxValue - minValue)*normalizedValue + minValue;
	}

	/**
	\brief get default value as a normalized value (helper)

	\return normalized version
	*/
	inline double getNormalizedDefaultValue()
    {
        // --- calculate normalized value from actual
		//double d = (defaultValue - minValue) / (maxValue - minValue);
		return (defaultValue - minValue) / (maxValue - minValue);
	}

	/**
	\brief get log default value in normalized form

	\return normalized version
	*/
	inline double getNormalizedLogDefaultValue()
    {
        return logNormToNorm(getNormalizedDefaultValue());
    }

	/**
	\brief get anti-log default value in normalized form

	\return normalized version
	*/
	inline double getNormalizedAntiLogDefaultValue()
    {
        return antiLogNormToNorm(getNormalizedDefaultValue());
    }

	/**
	\brief get volt/octave default value in normalized form

	\return normalized version
	*/
	inline double getNormalizedVoltOctaveDefaultValue()
    {
        if (minValue == 0)
            return defaultValue;

        return log2(defaultValue / minValue) / (log2(maxValue / minValue));
    }

private:
    // --- for variable-binding support
    uint32_t* boundVariableUInt = nullptr;			///< bound variable as UINT
    int* boundVariableInt = nullptr;				///< bound variable as int
    float* boundVariableFloat = nullptr;			///< bound variable as float
    double* boundVariableDouble = nullptr;			///< bound variable as double

	typedef std::map<uint32_t, AuxParameterAttribute*> auxParameterAttributeMap; ///< Aux attributes that can be stored on this object (similar to VSTGUI4) makes it easy to add extra data in the future
	auxParameterAttributeMap auxAttributeMap;		///< map of aux attributes

};

#endif

