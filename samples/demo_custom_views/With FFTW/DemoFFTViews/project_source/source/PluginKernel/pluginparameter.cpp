// -----------------------------------------------------------------------------
//   ASPiK Plugin Kernel File:  pluginparameter.cpp
//
/**
    \file   pluginparameter.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class implementation file for ASPiK pluginparameter object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "pluginparameter.h"

/**
\brief constructor for most knobs and sliders

\param _controlID numerical control identifier -- MUST BE UNIQUE among all parameter ID values
\param _controlName name string
\param _controlUnits units string
\param _controlType type
\param _minValue min limit
\param _maxValue max limit
\param _defaultValue default value at startup
\param _controlTaper type of gtaper
\param _displayPrecision sig digits of precision on GUI display
*/
PluginParameter::PluginParameter(int _controlID, const char* _controlName, const char* _controlUnits,
                                 controlVariableType _controlType, double _minValue, double _maxValue, double _defaultValue,
                                 taper _controlTaper, uint32_t _displayPrecision)
: controlID(_controlID)
, controlName(_controlName)
, controlUnits(_controlUnits)
, controlType(_controlType)
, minValue(_minValue)
, maxValue(_maxValue)
, defaultValue(_defaultValue)
, controlTaper(_controlTaper)
, displayPrecision(_displayPrecision)
{
    setControlValue(_defaultValue);
    setSmoothedTargetValue(_defaultValue);
    useParameterSmoothing = false;
    setIsWritable(false);
}

/**
\brief one method of constructing a string-list parameter

\param _controlID numerical control identifier -- MUST BE UNIQUE among all parameter ID values
\param _controlName name string
\param _stringList string list in std::vector format
\param _defaultString default string value as std::string
*/
PluginParameter::PluginParameter(int _controlID, const char* _controlName, std::vector<std::string> _stringList, std::string _defaultString)
: controlID(_controlID)
, controlName(_controlName)
, stringList(_stringList)
{
    setControlValue(0.0);
    setSmoothedTargetValue(0.0);
    setControlVariableType(controlVariableType::kTypedEnumStringList);
    setMaxValue((double)stringList.size()-1);

    int defaultStringIndex = findStringIndex(_defaultString);
    if(defaultStringIndex >= 0)
    {
        setDefaultValue((double)defaultStringIndex);
        setControlValue((double)defaultStringIndex);
    }
    useParameterSmoothing = false;
    setIsWritable(false);
    setCommaSeparatedStringList();
}

/**
\brief second method of constructing a string-list parameter

\param _controlID numerical control identifier -- MUST BE UNIQUE among all parameter ID values
\param _controlName name string
\param _commaSeparatedList simple character string of comma-separated items
\param _defaultString default string value as std::string
*/
PluginParameter::PluginParameter(int _controlID, const char* _controlName, const char* _commaSeparatedList, std::string _defaultString)
: controlID(_controlID)
, controlName(_controlName)
{
    setControlValue(0.0);
    setSmoothedTargetValue(0.0);

    std::stringstream ss(_commaSeparatedList);
    while(ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        stringList.push_back(substr);
    }

	// --- create csvlist
	setCommaSeparatedStringList();

    setControlVariableType(controlVariableType::kTypedEnumStringList);
    setMaxValue((double)stringList.size()-1);

    int defaultStringIndex = findStringIndex(_defaultString);
    if(defaultStringIndex >= 0)
    {
        setDefaultValue((double)defaultStringIndex);
        setControlValue((double)defaultStringIndex);
    }
    useParameterSmoothing = false;
    setIsWritable(false);
    setCommaSeparatedStringList();
}


/**
\brief method of constructing a meter parameter

\param _controlID numerical control identifier -- MUST BE UNIQUE among all parameter ID values
\param _controlName name string
\param _meterAttack_ms attack time for meter (ballistics)
\param _meterRelease_ms release time for meter (ballistics)
\param _detectorMode peak, ms, rms
\param _meterCal linear or log calibration
*/
PluginParameter::PluginParameter(int _controlID, const char* _controlName, double _meterAttack_ms, double _meterRelease_ms, uint32_t _detectorMode, meterCal _meterCal)
: controlID(_controlID)
, controlName(_controlName)
, meterAttack_ms(_meterAttack_ms)
, meterRelease_ms(_meterRelease_ms)
, detectorMode(_detectorMode)
{
	if (_meterCal == meterCal::kLinearMeter)
		setLogMeter(false);
	else
		setLogMeter(true);

    setControlValue(0.0);
    setSmoothedTargetValue(0.0);

    setControlVariableType(controlVariableType::kMeter);
    useParameterSmoothing = false;
    setIsWritable(true);
}

/**
\brief method of constructing a non-bound-variable parameter

\param _controlID numerical control identifier -- MUST BE UNIQUE among all parameter ID values
\param _controlName name string
\param _controlType type of control
*/
PluginParameter::PluginParameter(int _controlID, const char* _controlName, controlVariableType _controlType)
: controlID(_controlID)
, controlName(_controlName)
, controlType(_controlType)
{
    setControlValue(0.0);
    setSmoothedTargetValue(0.0);

    useParameterSmoothing = false;
    setIsWritable(false);
}

/**
\brief simple constructor - you can always use this and then use the massive number of get/set functions to customize in any manner
*/
PluginParameter::PluginParameter()
{
    setControlValue(0.0);
    setSmoothedTargetValue(0.0);

    useParameterSmoothing = false;
    setIsWritable(false);
}


/**
\brief copy constructor
*/
PluginParameter::PluginParameter(const PluginParameter& initGuiControl)
{
    controlID = initGuiControl.controlID;
    controlName = initGuiControl.controlName;
	controlUnits = initGuiControl.controlUnits;
	commaSeparatedStringList = initGuiControl.commaSeparatedStringList;
	controlType = initGuiControl.controlType;
    minValue = initGuiControl.minValue;
    maxValue = initGuiControl.maxValue;
    defaultValue = initGuiControl.defaultValue;
    controlValueAtomic = initGuiControl.getAtomicControlValueFloat();
    controlTaper = initGuiControl.controlTaper;
    displayPrecision = initGuiControl.displayPrecision;
    stringList = initGuiControl.stringList;
    useParameterSmoothing = initGuiControl.useParameterSmoothing;
    smoothingType = initGuiControl.smoothingType;
    smoothingTimeMsec = initGuiControl.smoothingTimeMsec;
    meterAttack_ms = initGuiControl.meterAttack_ms;
    meterRelease_ms = initGuiControl.meterRelease_ms;
	detectorMode = initGuiControl.detectorMode;
    logMeter = initGuiControl.logMeter;
    isWritable = initGuiControl.isWritable;
	isDiscreteSwitch = initGuiControl.isDiscreteSwitch;
	invertedMeter = initGuiControl.invertedMeter;
}

/**
\brief only need to clean out the aux parameters - everything else is self deleting
*/
PluginParameter::~PluginParameter()
{
	// --- clear aux attributes
	for (auxParameterAttributeMap::const_iterator it = auxAttributeMap.begin(), end = auxAttributeMap.end(); it != end; ++it)
	{
		delete it->second;
	}

	auxAttributeMap.clear();
}

/**
\brief get control value: string

\return std::string version of control value
*/
std::string PluginParameter::getControlValueAsString()
{
	std::string empty;
	if (controlType == controlVariableType::kTypedEnumStringList)
	{
		if ((uint32_t)getAtomicControlValueFloat() >= stringList.size())
			return empty;

		return stringList[(uint32_t)getAtomicControlValueFloat()];
	}

	std::ostringstream ss;
	ss.setf(std::ios_base::fixed);
	float value = getAtomicControlValueFloat();
	uint32_t precision = getDisplayPrecision();
	double zeroLimit = pow(10.0, -1.0*precision);

	if (fabs(value) < zeroLimit)
		value = 0.0;

	ss << value;
	std::string numString(ss.str());

	if (numString == "-inf" || numString == "inf")
		return numString;

	size_t pos = numString.find('.');
	if (pos == std::string::npos)
	{
		numString += ".00000000";
		pos = numString.find('.');
	}

	numString += "00000000000000000000000000000000";

	if (controlType != controlVariableType::kInt)
		pos += displayPrecision + 1;

	std::string formattedString = numString.substr(0, pos);
	if (appendUnits)
	{
		formattedString += " ";
		formattedString += controlUnits;
	}

	return formattedString;
}

/**
\brief get string-list string by index

\param index of string in list
\return std::string version of single string
*/
std::string PluginParameter::getStringByIndex(uint32_t index)
{
	std::string empty;
	if (index >= stringList.size())
		return empty;

	return stringList[index];
}

/**
\brief set comma-separated version of string list (internally called)
*/
void PluginParameter::setCommaSeparatedStringList()
{
    commaSeparatedStringList.clear();

    for(std::vector<std::string>::iterator it = stringList.begin(); it != stringList.end(); ++it)
    {
        std::string subStr = *it;
        if(commaSeparatedStringList.size() > 0)
            commaSeparatedStringList.append(",");
        commaSeparatedStringList.append(subStr);
    }
}

/**
\brief set an aux attribute

\param attributeID unique identifier of attribute
\param auxParameterAtribute information about the aux attribute

\return the index of the new attribute
*/
uint32_t PluginParameter::setAuxAttribute(uint32_t attributeID, const AuxParameterAttribute& auxParameterAtribute)
{
	AuxParameterAttribute* newAttribute = new AuxParameterAttribute(auxParameterAtribute);
	if (!newAttribute) return 0;

	auxAttributeMap.insert(std::make_pair(attributeID, newAttribute));

	return (uint32_t)auxAttributeMap.size();
}

/**
\brief get an aux attribute

\param attributeID unique identifier of attribute

\return a naked pointer to the attribute
*/
AuxParameterAttribute* PluginParameter::getAuxAttribute(uint32_t attributeID)
{
	if (auxAttributeMap.find(attributeID) == auxAttributeMap.end()) {
		return nullptr;
	}

	return  auxAttributeMap[attributeID];
}
