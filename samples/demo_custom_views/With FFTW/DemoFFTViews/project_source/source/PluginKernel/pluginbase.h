// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  pluginbase.h
//
/**
    \file   pluginbase.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class interface file for ASPiK pluginbase object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
/** \defgroup ASPiK-Core

The PluginCore:
\brief
- handles the audio signal-processing and implements the DSP functionality
- is straight C++ and does not contain any API-specific code, does not require any API-specific files or SDK components
- does not link to any pre-compiled libraries
- defines and maintains a set of PluginParameter objects, each of which corresponds to a GUI control or other plugin parameter that may be stored and loaded with DAW sessions and presets
- defines factory presets for AU, VST and RAFX2 plugins (AAX factory presets are done in an entirely different manner and are discussed in Chapter 5)
- exists independently from the PluginGUI and does not know, or need to know, of the PluginGUIs existence
- does not create the PluginGUI object
- does not hold a pointer to the PluginGUI object or share any resources with it

The PluginParameter:
\brief
- stores plugin parameters as atomic variables for thread-safe operation
- encapsulates each parameter specific to the plugin as a C++ object
- can store all types of input parameters; ints, floats, doubles, string-lists, and custom user types
- can implement audio meters, complete with meter ballistics (attack and release times) and various envelope detection schemes: peak, MS, RMS and in linear or log format
- implements optional automatic variable-binding to connect GUI parameter changes to plugin variables in a completely thread-safe manner across all APIs
- has an optional auxiliary storage system to maintain other information along with any of the plugin parameters allowing you to easily customize and extend the object
- implements optional parameter smoothing for glitch-free GUI controls with two types of smoothing available: linear and exponential
- automatically implements four types of control tapering: linear, log, anti-log, and volt/octave
- implements optional sample-accurate automation for VST3 plugins (VST3 is the only API that has a specification for sample-accurate automation)

**/

/** \defgroup ASPiK-GUI

The VSTGUI::PluginGUI:
\brief
- handles all of the GUI functionality including the GUI designer
- is built using the VSTGUI4 library
- is platform independent
- does not link to any pre-compiled libraries
- encodes the entire GUI in a single XML file, including the graphics file data; this one file may be moved or copied from one project to another, allowing the whole GUI to be easily moved or reused in other projects; advanced users may define multiple GUIs in multiple XML files which may be swapped in and out
- contains API-specific code in the few places where it is absolutely needed (namely for the AU event-listener system)
- supports the VTSGUI4 Custom View and Sub-Controller paradigms to extend its functionality
- exists independently from the PluginCore and does not know, or need to know, of the PluginCore’s existence
- does not hold a pointer to the PluginCore object or share any resources with it
- NOTE: the PluginGUI object is defined within the VSTGUI namespace and is grouped with more VSTGUI objects that you may use

The PluginParameter:
\brief
- stores plugin parameters as atomic variables for thread-safe operation
- encapsulates each parameter specific to the plugin as a C++ object
- can store all types of input parameters; ints, floats, doubles, string-lists, and custom user types
- can implement audio meters, complete with meter ballistics (attack and release times) and various envelope detection schemes: peak, MS, RMS and in linear or log format
- implements optional automatic variable-binding to connect GUI parameter changes to plugin variables in a completely thread-safe manner across all APIs
- has an optional auxiliary storage system to maintain other information along with any of the plugin parameters allowing you to easily customize and extend the object
- implements optional parameter smoothing for glitch-free GUI controls with two types of smoothing available: linear and exponential
- automatically implements four types of control tapering: linear, log, anti-log, and volt/octave
- implements optional sample-accurate automation for VST3 plugins (VST3 is the only API that has a specification for sample-accurate automation)

**/

/** \defgroup Interfaces

Interfaces are used in the FX Objects as a common way of programming.

\brief

**/


/** \defgroup Structures


**/

/** \defgroup Constants-Enums


**/


/** \defgroup Custom-Controls

The ASPiK CustomControls folder contains C++ files that implement custom controls, views, and sub-controllers.
\brief

**/

/** \defgroup Custom-Views

The ASPiK CustomControls folder contains C++ files that implement custom controls, views, and sub-controllers.
\brief

**/

/** \defgroup Custom-SubControllers

The ASPiK CustomControls folder contains C++ files that implement custom controls, views, and sub-controllers.
\brief

**/

/** \defgroup FX-Objects
\brief
The FX-Objects module contains specialized audio DSP C++ objects that are inlcuded with and documented in Will Pirkle's new plugin programming book. These objects will be available in May 2019 when the 2nd Edition of Designing Audio Effects Plugins in C++ is published and will be free to use in your projects, commercial or personal.

\brief

**/

/** \defgroup FX-Functions

\brief
The FX-Functions module contains specialized audio DSP C++ functions that are inlcuded with and documented in Will Pirkle's new plugin programming book. These objects will be available in May 2019 when the 2nd Edition of Designing Audio Effects Plugins in C++ is published and will be free to use in your projects, commercial or personal.


\brief

**/

/** \defgroup WDF-Objects

\brief
The WDF-Objects module contains specialized audio DSP C++ objects that are inlcuded with and documented in Will Pirkle's new plugin programming book. These objects will be available in May 2019 when the 2nd Edition of Designing Audio Effects Plugins in C++ is published and will be free to use in your projects, commercial or personal.


\brief

**/

/** \defgroup FFTW-Objects
\brief
The FX-Objects module contains specialized audio DSP C++ objects that are inlcuded with and documented in Will Pirkle's new plugin programming book. These objects will be available in May 2019 when the 2nd Edition of Designing Audio Effects Plugins in C++ is published and will be free to use in your projects, commercial or personal.


**/


/** \defgroup Plugin-Shells


**/

/** @addtogroup AU-Shell
  * \ingroup Plugin-Shells

The AU plugin shell consists of the following files:
- aufxplugin.h
- aufxplugin.cpp
- aufxplugin.exp
- aucocoaviewfactory.mm
*/

/** @addtogroup VST-Shell
  * \ingroup Plugin-Shells

The VST plugin shell consists of the following files:
- channelformats.h
- factory.cpp
- vst3plugin.h
- vst3plugin.cpp
- customparameters.h
- customparameters.cpp
*/

/** @addtogroup AAX-Shell
  * \ingroup Plugin-Shells

The AAX plugin shell consists of the following files:
- channelformats.h (not used)
- antilogtaperdelegate.h
- logtaperdelegate.h
- voltoctavetaperdelegate.h
- AAXPluginDescribe.h
- AAXPluginDescribe.cpp
- AAXPluginGUI.h
- AAXPluginGUI.cpp
- AAXPluginParameters.h
- AAXPluginParameters.cpp
*/




#ifndef __PluginBase__
#define __PluginBase__

#include "pluginparameter.h"

#include <map>

/**
\class PluginBase
\ingroup ASPiK-Core
\brief
The PluginBase object is the base class for the Plugin Core object.

PluginBase Operations:
- maintains the PluginParameter list in multiple formats (map, vector, old-fashioned C array) that are tailored to specific lookup duties.
- supplies low level functions for getting plugin atrributes (e.g. the VST3 GUID or the AAX ID number)
- maintains three plugin descriptor arrays that store attribute information; part of the "Description" process from the book; these are:
	- PluginDescriptor pluginDescriptor - describes the basic plugin strings (name, manufacturer, etc...)
	- AudioProcDescriptor audioProcDescriptor - describes the currently loaded DAW session's audio (WAV) file param; is always available to any plugin function
	- APISpecificInfo apiSpecificInfo - contains api-specific description strings, code numbers, and other details.
- implements the buffer processing function; this method breaks the incoming and outgoing buffers into frames, and then calls your plugin core frame processing function.
- you should not need to edit this object - all work should be done in the PluginCore object


\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginBase
{
public:
    PluginBase();
    virtual ~PluginBase();

	// --- PURE VIRTUAL FUNCTIONS ---------------------------------------------------------------------------------------------------- //
    /** pure virtual function: process one frame of audio */
	virtual bool processAudioFrame(ProcessFrameInfo& processFrameInfo) = 0;

	/** pure virtual function: update the plugin parameter using GUI control value (thread-safe) */
	virtual bool updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo) = 0;

	/** pure virtual function: update the plugin parameter using normalized GUI control value (thread-safe) */
	virtual bool updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo) = 0;

	/** pure virtual function: do any post-update processing (cooking) of data */
	virtual bool postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo) = 0;

	/** pure virtual function: do any last preparation for arrival of fresh audio buffers*/
	virtual bool preProcessAudioBuffers(ProcessBufferInfo& processInfo) = 0;

	/** pure virtual function: do any extra work that requires the buffer to be completely processed first */
	virtual bool postProcessAudioBuffers(ProcessBufferInfo& processInfo) = 0;
	// ----------------------------------------------------------------------------------------------------------------------------- //

	/** rest the object to it's preferred initial state */
	virtual bool reset(ResetInfo& resetInfo);

	/** one-time post creation init function; pluginInfo contains path to this plugin */
	virtual bool initialize(PluginInfo& _pluginInfo);

	/** Buffer Proc Cycle: I connects GUI control changes to bound variables (part of ASPiK input variable binding option) */
	void syncInBoundVariables();

	/** Buffer Proc Cycle: II PluginCore overrides this method to process frames */
	virtual bool processAudioBuffers(ProcessBufferInfo& processInfo);

	/** Buffer Proc Cycle: III connects meter variables to outbound GUI control changes (part of ASPiK output variable binding option) */
	bool updateOutBoundVariables();

	/** notification that a GUI parameter changed. NOT for updating internal states or variables; unused at base class level */
	virtual bool guiParameterChanged(int32_t controlID, double actualValue) { return true; }

	/** ASPiK messaging system: base class implementation is empty */
	virtual bool processMessage(MessageInfo& messageInfo) { return true; }

	/** ASPiK midi event system: base class implementation is empty */
	virtual bool processMIDIEvent(midiEvent& event) { return true; }

	/** perform parameter smoothing or VST3 sample accurate upates */
	void doSampleAccurateParameterUpdates();

	/** only for a vector joystick control from DAW that implements it (reserved for future use): base class implementation is empty */
	virtual bool setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData) { return true; }

	/** helper function to add a new plugin parameter to the variety of lists */
	int32_t addPluginParameter(PluginParameter* piParam, double sampleRate = 44100);

	/** helper function to add auxilliary data to a PluginParameter (SUPER useful way to stash all kinds of data on the parameter object!) */
	void setParamAuxAttribute(uint32_t controlID, const AuxParameterAttribute& auxAttribute);

	/** returns number of parameters in our list */
	size_t getPluginParameterCount(){ return pluginParameters.size(); }

	/**
	\brief get a parameter by index location in vector or array

	\param index the index in the array

	\return a naked pointer to the PluginParameter object
	*/
	PluginParameter* getPluginParameterByIndex(int32_t index) { return pluginParameters[index]; }

	/**
	\brief get a parameter by control ID - uses map (slowest)

	\param controlID the control ID of the parameter

	\return a naked pointer to the PluginParameter object
	*/
	PluginParameter* getPluginParameterByControlID(int32_t controlID) { return pluginParameterMap[controlID]; }

	/** get a parameter by type - used to find all meter variables for writing to GUI */
	PluginParameter* getNextParameterOfType(int32_t& startIndex, controlVariableType controlType);

	/** get a parameter's variable as double */
	double getPIParamValueDouble(int32_t controlID);

	/** get a parameter's variable as float */
	float getPIParamValueFloat(int32_t controlID);

	/** get a parameter's variable as int */
	int getPIParamValueInt(int32_t controlID);

	/** get a parameter's variable as UINT */
	uint32_t getPIParamValueUInt(int32_t controlID);

	/** query for Pro Tools meter */
	bool hasProToolsGRMeters();

	/** query for Pro Tools meter value*/
	double getProToolsGRValue();

	/** helper to easily add new I/O combination */
	uint32_t addSupportedIOCombination(ChannelIOConfig ioConfig);

	/** helper to easily add new Aux (sidechain) I/O combination */
	uint32_t addSupportedAuxIOCombination(ChannelIOConfig ioConfig);

	/**
	\brief get I/O channel pair count
	\return the count of I/O combinations
	*/
	uint32_t getNumSupportedIOCombinations() {return pluginDescriptor.numSupportedIOCombinations;}

	/** query input format */
	bool hasSupportedInputChannelFormat(uint32_t channelFormat);

	/** query output format */
	bool hasSupportedOutputChannelFormat(uint32_t channelFormat);

	/**
	\brief get input cj
	\return the count of I/O combinations
	*/
    int32_t getChannelInputFormat(uint32_t ioConfigIndex) {return pluginDescriptor.supportedIOCombinations[ioConfigIndex].inputChannelFormat;}

	/**
	\brief get I/O channel pair count
	\return the count of I/O combinations
	*/
	int32_t getChannelOutputFormat(uint32_t ioConfigIndex) {return pluginDescriptor.supportedIOCombinations[ioConfigIndex].outputChannelFormat;}

	/** query input channels */
	uint32_t getInputChannelCount(uint32_t ioConfigIndex);

	/** query output channels */
	uint32_t getOutputChannelCount(uint32_t ioConfigIndex);

	/**
	\brief get the configuration (e.g. kCFStereo) for a given channel count; mainly for AU that does not discriminate between formats with same channel counts (e.g. Sony 7.1 vs DTS 7.1 which have same channel count)

	\param channelCount number of channels (e.g. 2)

	\returns the plugin's preferred configuration (e.g. kCFStereo) for channel count
	*/
	uint32_t getDefaultChannelIOConfigForChannelCount(uint32_t channelCount) { return pluginDescriptor.getDefaultChannelIOConfigForChannelCount(channelCount); }

	/** set value  */
	void setPIParamValue(uint32_t _controlID, double _controlValue);

	/** sest normalized */
	double setPIParamValueNormalized(uint32_t _controlID, double _normalizedValue, bool applyTaper = true);

	/** perform variable binding at parameter level */
	bool updatePIParamBoundValue(uint32_t _controlID);

	/** clears the current cache of GUI parameters for updating (this is part of the GUI update messaging and NOT part of variable binding) */
	void clearUpdateGUIParameters(std::vector<GUIParameter*>& guiParameters);

	/** copies the current plugin list into another list */
    std::vector<PluginParameter*>* makePluginParameterVectorCopy(bool disableSmoothing = true);

	/** setup the preset list */
	bool initPresetParameters(std::vector<PresetParameter>& presetParameters, bool disableSmoothing = true);

	/** set an individual preset */
	bool setPresetParameter(std::vector<PresetParameter>& presetParameters, uint32_t _controlID, double _controlValue);

	/**
	\brief store the plugin host interface pointer: this pointer will never go out of scope or be invalid once stored!

	\param _pluginHostConnector the interface pointer
	*/
    void setPluginHostConnector(IPluginHostConnector* _pluginHostConnector){pluginHostConnector = _pluginHostConnector;}

	/**
	\brief get number of stored presets

	\return preset count
	*/
	size_t getPresetCount(){return presets.size();}

	/** get preset name	*/
	const char* getPresetName(uint32_t index);

	/** add preset	*/
	size_t addPreset(PresetInfo* preset);

	/** remove preset	*/
	void removePreset(uint32_t index);

	/** remove all presets	*/
	void removeAllPresets();

	/** get a preset pointner	*/
	PresetInfo* getPreset(uint32_t index);

	/** prepare all parameter lists	*/
	void initPluginParameterArray();

	/** compare param value as string*/
	bool compareSelectedString(int32_t controlID, const char* compareString);

	/**
	\brief Description query: sidechain

	\return true if plugin wants a sidechain input
	*/
	bool hasSidechain() { return pluginDescriptor.hasSidechain; }

	/**
	\brief Description query: MIDI

	\return true if plugin wants MIDI input (and output if available)
	*/
	bool wantsMIDI() { return pluginDescriptor.wantsMIDI; }

	/**
	\brief Description query: has GUI

	\return true if plugin has a custom GUI
	*/
	bool hasCustomGUI() { return pluginDescriptor.hasCustomGUI; }

	/**
	\brief Description query: latency

	\return latency in samples (as double)
	*/
	double getLatencyInSamples() { return pluginDescriptor.latencyInSamples; }

	/**
	\brief Description query: tail time

	\return tail time in milliseconds (as double)
	*/
	double getTailTimeInMSec() { return pluginDescriptor.tailTimeInMSec; }

	/**
	\brief Description query: infinite tail (VST3 only)

	\return true if plugin wants infinite tail
	*/
	bool wantsInfiniteTailVST3() { return pluginDescriptor.infiniteTailVST3; }

	/**
	\brief Description query: name

	\return name as const char*
	*/
	const char* getPluginName() { return pluginDescriptor.pluginName.c_str(); }

	/**
	\brief Description query: short name (AAX)

	\return short name as const char*
	*/
	const char* getShortPluginName() { return pluginDescriptor.shortPluginName.c_str(); }

	/**
	\brief Description query:vendor  name

	\return vendor name as const char*
	*/
	const char* getVendorName() { return pluginDescriptor.vendorName.c_str(); }

	/**
	\brief Description query: plugin type

	\return enum type (kFXPlugin)
	*/
	uint32_t getPluginType() { return pluginDescriptor.pluginTypeCode; }

	/**
	\brief Description query: sample rate

	\return sample rate
	*/
	double getSampleRate() { return audioProcDescriptor.sampleRate; }

	/**
	\brief Description query: 4-char code

	\return four-char code as an int
	*/
	int getFourCharCode() { return apiSpecificInfo.fourCharCode; }

	/**
	\brief Description query: AAX Man ID

	\return AAX manufacturer ID as const char*
	*/
	const uint32_t getAAXManufacturerID() { return apiSpecificInfo.aaxManufacturerID; }

	/**
	\brief Description query: AAX Prod ID

	\return AAX product ID as const char*
	*/
	const uint32_t getAAXProductID() { return apiSpecificInfo.aaxProductID; }

	/**
	\brief Description query: AAX Bundle ID

	\return AAX Bundle ID as const char*
	*/
	const char* getAAXBundleID() { return apiSpecificInfo.aaxBundleID.c_str(); }

	/**
	\brief Description query: AAX Effect ID

	\return AAX Effect ID as const char*
	*/
	const char* getAAXEffectID() { return apiSpecificInfo.aaxEffectID.c_str(); }

	/**
	\brief Description query: AAX Category

	\return AAX category code (e.g. aaxPlugInCategory::aaxPlugInCategory_Effect)
	*/
	uint32_t getAAXPluginCategory() { return apiSpecificInfo.aaxPluginCategoryCode; }

	/**
	\brief Description query: VST3 FUID

	\return VST3 FUID as const char*
	*/
	const char* getVST3_FUID() { return apiSpecificInfo.vst3FUID.c_str(); }

	/**
	\brief Description query: VST Sample Accurate Automation

	\return true if plugin wants sample accurate automation
	*/
	bool wantsVST3SampleAccurateAutomation() { return apiSpecificInfo.enableVST3SampleAccurateAutomation; }

	/**
	\brief Description query: VST Sample Accurate Automation granularity

	\return VST Sample Accurate Automation granularity in samples
	*/
	uint32_t getVST3SampleAccuracyGranularity() { return apiSpecificInfo.vst3SampleAccurateGranularity; }

	/**
	\brief Description query: VST3 Bundle ID

	\return VST3 Bundle ID as const char*
	*/
	const char* getVST3BundleID() { return apiSpecificInfo.vst3BundleID.c_str(); }

	/**
	\brief Description query: AU Bundle ID

	\return AU Bundle ID as const char*
	*/
	const char* getAUBundleID() { return apiSpecificInfo.auBundleID.c_str(); }

	/**
	\brief Description query: AU Bundle Name

	\return AU Bundle Name as const char*
	*/
	const char* getAUBundleName() { return apiSpecificInfo.auBundleName.c_str(); }

protected:
    PluginDescriptor pluginDescriptor;			///< description strings
    APISpecificInfo apiSpecificInfo;			///< description strings, API specific
	AudioProcDescriptor audioProcDescriptor;	///< current audio processing description
	PluginInfo pluginInfo;						///< info about the DLL (component) itself, includes path to DLL

    // --- arrays for frame processing
    float inputFrame[MAX_CHANNEL_COUNT];		///< input array for frame processing
    float outputFrame[MAX_CHANNEL_COUNT];		///< output array for frame processing
    float auxInputFrame[MAX_CHANNEL_COUNT];		///< aux input array for frame processing
    float auxOutputFrame[MAX_CHANNEL_COUNT];	///< aux output array for frame processing

	// --- ultra-fast access for real-time audio processing
	PluginParameter** pluginParameterArray = nullptr;			///< old-fashioned C-arrays of pointers for ultra-fast access for real-time audio processing
	uint32_t numPluginParameters = 0;							///< total number of parameters
	PluginParameter** smoothablePluginParameters = nullptr;		///< old-fashioned C-arrays of pointers for smoothable parameters
	uint32_t numSmoothablePluginParameters = 0;					///< number of smoothable parameters only
	PluginParameter** outboundPluginParameters = nullptr;		///< old-fashioned C-arrays of pointers for outbound (meter) parameters
	uint32_t numOutboundPluginParameters = 0;					///< total number of outbound (meter) parameters

    // --- vectorized version of pluginParameterMap for fast iteration when key not needed
    std::vector<PluginParameter*> pluginParameters;				///< vector version of parameter list

    // --- map<controlID , PluginParameter*>
    typedef std::map<uint32_t, PluginParameter*> pluginParameterControlIDMap;	///< map version of parameter list
    pluginParameterControlIDMap pluginParameterMap;								///< member map of parameter list

    // --- plugin core -> host (wrap) connector
    IPluginHostConnector* pluginHostConnector = nullptr;						///< created and destroyed on host

    // --- PRESETS
    std::vector<PresetInfo*> presets;	///< preset list
};

#endif /* defined(__PluginBase__) */
