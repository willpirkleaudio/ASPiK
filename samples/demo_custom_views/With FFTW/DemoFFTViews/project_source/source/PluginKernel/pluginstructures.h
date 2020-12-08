// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  pluginstructures.h
//
/**
    \file   pluginstructures.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  globally utilized structures and enumerations
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef _pluginstructures_h
#define _pluginstructures_h

// --- support multichannel operation up to 128 channels
#define MAX_CHANNEL_COUNT 128

#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>

#include "readerwriterqueue.h"
#include "atomicops.h"

class IGUIPluginConnector;
class IGUIWindowFrame;
class IGUIView;

#ifdef AUPLUGIN
#import <CoreFoundation/CoreFoundation.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

typedef struct
{
    void* pWindow;
    float width;
    float height;
    AudioUnit au;
    IGUIWindowFrame* pGUIFrame;
    IGUIView* pGUIView;
} VIEW_STRUCT;
#endif


/**
\enum pluginType
\ingroup Constants-Enums
\brief
Use this enum to identify the plugin category.

- kFXPlugin or kSynthPlugin

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum pluginType
{
    kFXPlugin,
    kSynthPlugin
};

/**
\enum aaxPlugInCategory
\ingroup Constants-Enums
\brief
Use this enum to identify the AAX plugin category.

- aaxPlugInCategory_None through aaxPlugInCategory_Effect
- 18 categories in total

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum aaxPlugInCategory
{
    aaxPlugInCategory_None              = 0x00000000,
    aaxPlugInCategory_EQ                = 0x00000001,	///<  Equalization
    aaxPlugInCategory_Dynamics          = 0x00000002,	///<  Compressor, expander, limiter, etc.
    aaxPlugInCategory_PitchShift		= 0x00000004,	///<  Pitch processing
    aaxPlugInCategory_Reverb			= 0x00000008,	///<  Reverberation and room/space simulation
    aaxPlugInCategory_Delay             = 0x00000010,	///<  Delay and echo
    aaxPlugInCategory_Modulation		= 0x00000020,	///<  Phasing, flanging, chorus, etc.
    aaxPlugInCategory_Harmonic          = 0x00000040,	///<  Distortion, saturation, and harmonic enhancement
    aaxPlugInCategory_NoiseReduction    = 0x00000080,	///<  Noise reduction
    aaxPlugInCategory_Dither			= 0x00000100,	///<  Dither, noise shaping, etc.
    aaxPlugInCategory_SoundField		= 0x00000200,  	///<  Pan, auto-pan, upmix and downmix, and surround handling
    aaxPlugInCategory_HWGenerators      = 0x00000400,	///<  Fixed hardware audio sources such as SampleCell
    aaxPlugInCategory_SWGenerators      = 0x00000800,	///<  Virtual instruments, metronomes, and other software audio sources
    aaxPlugInCategory_WrappedPlugin     = 0x00001000,	///<  Plug-ins wrapped by a thrid party wrapper except synth plug-ins which = AAX_PlugInCategory_SWGenerators
    aaxPlugInCategory_Effect			= 0x00002000,	///<  Special effects
};

/**
\enum channelFormat
\ingroup Constants-Enums
\brief
Use this enum to identify plugin channel formats. Steinberg calls these "speaker arrangements"

- 30 different formats
- you can add support for ambisonics and other API-specific formats by starting here

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum channelFormat
{
    kCFNone,
    kCFMono,
    kCFStereo,
    kCFLCR,
    kCFLCRS,
    kCFQuad,
    kCF5p0,
    kCF5p1,
    kCF6p0,
    kCF6p1,
    kCF7p0Sony,
    kCF7p0DTS,
    kCF7p1Sony,
    kCF7p1DTS,
    kCF7p1Proximity,

    /* the following are NOT directly suppported by AAX/AU */
    kCF8p1,
    kCF9p0,
    kCF9p1,
    kCF10p0,
    kCF10p1,
    kCF10p2,
    kCF11p0,
    kCF11p1,
    kCF12p2,
    kCF13p0,
    kCF13p1,
    kCF22p2,
};

/**
\enum auxGUIIdentifier
\ingroup Constants-Enums
\brief
Identifier enum for aux parameter information. Not used in ASPiK though is used for RAFX plugins (not-exported)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum auxGUIIdentifier
{
	GUIKnobGraphic,
	GUI2SSButtonStyle,
	EnableMIDIControl,
	MIDIControlChannel,
	MIDIControlIndex,
	midiControlData,
	guiControlData
};

/**
\struct ResetInfo
\ingroup Structures
\brief
Sample rate and bit-depth information that is passed during the reset( ) function.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ResetInfo
{
	ResetInfo()
		: sampleRate(44100)
		, bitDepth(16) {}

	ResetInfo(double _sampleRate,
		uint32_t _bitDepth)
		: sampleRate(_sampleRate)
		, bitDepth(_bitDepth) {}

	double sampleRate = 0.0;	///< sample rate
	uint32_t bitDepth = 0;		///< bit depth (not available in all APIs)
};

/**
\struct APISpecificInfo
\ingroup Structures
\brief
Identifiers, GUIDs and other strings and number id values, API specific.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct APISpecificInfo
{
    APISpecificInfo ()
    : aaxManufacturerID(0) /* change this in the plugin core constructor */
    , aaxProductID(0)      /* change this in the plugin core constructor */
    , aaxEffectID("")
    , aaxPluginCategoryCode(aaxPlugInCategory::aaxPlugInCategory_Effect)
    , fourCharCode(0)
    , vst3FUID("")
    , enableVST3SampleAccurateAutomation(0)
    , vst3SampleAccurateGranularity(1)
    , vst3BundleID("")
    , auBundleID("")
    {}

	APISpecificInfo& operator=(const APISpecificInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		aaxManufacturerID = data.aaxManufacturerID;
		aaxProductID = data.aaxProductID;
		aaxEffectID = data.aaxEffectID;
		aaxBundleID = data.aaxBundleID;
		aaxPluginCategoryCode = data.aaxPluginCategoryCode;

		fourCharCode = data.fourCharCode;
		vst3FUID = data.vst3FUID;

		enableVST3SampleAccurateAutomation = data.enableVST3SampleAccurateAutomation;
		vst3SampleAccurateGranularity = data.vst3SampleAccurateGranularity;
		vst3BundleID = data.vst3BundleID;
		auBundleID = data.auBundleID;
		auBundleName = data.auBundleName;

		return *this;
	}

    uint32_t aaxManufacturerID = 0;			///< aax manu ID
    uint32_t aaxProductID = 0;				///< aax ID
    std::string aaxEffectID;				///< aax Effect ID
    std::string aaxBundleID;				///< AAX bundle  /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    uint32_t aaxPluginCategoryCode = 0;		///< aax plugin category

    // --- common to AU and AAX
    int fourCharCode = 0;					///< the mystic and ancient 4-character code (oooh)

    // --- VST3
    std::string vst3FUID;								///< VST GUID
    bool enableVST3SampleAccurateAutomation = false;	///< flag for sample accurate automation
    uint32_t vst3SampleAccurateGranularity = 1;			///< sample accuracy granularity (update interval)
    std::string vst3BundleID;							///< VST bundle ID /* MacOS only: this MUST match the bundle identifier in your info.plist file */

    // --- AU
    std::string auBundleID;         ///< AU bundle ID /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    std::string auBundleName;		///< AU bundle name  /* MacOS only: this MUST match the bundle name which is the same as the project name */

 };

/**
\struct VectorJoystickData
\ingroup Structures
\brief
Incoming data from a vector joystick.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct VectorJoystickData
{
	VectorJoystickData()
		: vectorA(0.0)
		, vectorB(0.0)
		, vectorC(0.0)
		, vectorD(0.0)
		, vectorACMix(0.0)
		, vectorBDMix(0.0) {}

	VectorJoystickData(double _vectorA, double _vectorB, double _vectorC, double _vectorD, double _vectorACMix, double _vectorBDMix)
		: vectorA(_vectorA)
		, vectorB(_vectorB)
		, vectorC(_vectorC)
		, vectorD(_vectorD)
		, vectorACMix(_vectorACMix)
		, vectorBDMix(_vectorBDMix) {}

	VectorJoystickData& operator=(const VectorJoystickData& vsData)	// need this override for collections to work
	{
		if (this == &vsData)
			return *this;

		vectorA = vsData.vectorA;
		vectorB = vsData.vectorB;
		vectorC = vsData.vectorC;
		vectorD = vsData.vectorD;

		vectorACMix = vsData.vectorACMix;
		vectorBDMix = vsData.vectorBDMix;

		return *this;
	}

	double vectorA = 0.0;
	double vectorB = 0.0;
	double vectorC = 0.0;
	double vectorD = 0.0;

	double vectorACMix = 0.0;
	double vectorBDMix = 0.0;
};


/**
\struct GUIParameter
\ingroup Structures
\brief
Information that defines a single GUI parameter's possible values and ID.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct GUIParameter
{
	GUIParameter()
		: controlID(0)
		, actualValue(0.0)
		, useCustomData(0)
		, customData(0) {}

	GUIParameter& operator=(const GUIParameter& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		controlID = data.controlID;
		actualValue = data.actualValue;
		useCustomData = data.useCustomData;
		customData = data.customData;

		return *this;
	}

	uint32_t controlID = 0;		///< ID value
	double actualValue = 0.0;	///< actual value
	bool useCustomData = false;	///< custom data flag (reserved for future use)

	// --- for custom drawing, or other custom data
	void* customData = nullptr; ///< custom data (reserved for future use)
};

/**
\struct PresetParameter
\ingroup Structures
\brief
Information that defines a preset value as a control_ID::value data pair

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct PresetParameter
{
    PresetParameter ()
    : controlID(0)
    , actualValue(0.0){}

    PresetParameter (uint32_t _controlID, double _actualValue)
    : controlID(_controlID)
    , actualValue(_actualValue){}

	PresetParameter& operator=(const PresetParameter& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		controlID = data.controlID;
		actualValue = data.actualValue;
		return *this;
	}

	uint32_t controlID = 0;		///< ID
    double actualValue = 0.0;	///< value
};

/**
\struct PresetParameter
\ingroup Structures
\brief
Information about a preset

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct PresetInfo
{
    PresetInfo(uint32_t _presetIndex, const char* _name)
    : presetIndex(_presetIndex)
    , presetName(_name) {}

	PresetInfo& operator=(const PresetInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		presetIndex = data.presetIndex;
		presetName = data.presetName;
		presetParameters = data.presetParameters;

		return *this;
	}

    uint32_t presetIndex = 0;		///< preset index
    std::string presetName;			///< preset name

    std::vector<PresetParameter> presetParameters;	///< list of parameters for this preset
};

/**
\struct GUIUpdateData
\ingroup Structures
\brief
Information about a GUI update message; this is for sending GUI control information from the plugin core.
It is not the optimal way to intelligently link or combine controls - use a sub-controller for that.
This can be abused too, and create dangerous code. Make sure to see the example code before using this.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct GUIUpdateData
{
	GUIUpdateData& operator=(const GUIUpdateData& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		guiUpdateCode = data.guiUpdateCode;
		guiParameters = data.guiParameters;
		customData = data.customData;
		useCustomData = data.useCustomData;

		return *this;
	}

    uint32_t guiUpdateCode = -1; ///< unused

    // --- for control updates
    std::vector<GUIParameter> guiParameters; ///< list of updates

    // --- for custom draw updates (graphs, etc...)
    void* customData = nullptr;	///< unused

    // --- flag
    bool useCustomData = false; ///< unused
};

/**
\enum hostMessage
\ingroup Constants-Enums
\brief
Use this enum to identify a message to send to the plugin shell (host)

- sendGUIUpdate or sendRAFXStatusWndText

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum hostMessage { sendGUIUpdate, sendRAFXStatusWndText };

struct HostMessageInfo
{
	HostMessageInfo()
		: hostMessage(0){}

	HostMessageInfo& operator=(const HostMessageInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		hostMessage = data.hostMessage;
		guiParameter = data.guiParameter;
		guiUpdateData = data.guiUpdateData;
		rafxStatusWndText = data.rafxStatusWndText;

		return *this;
	}

    uint32_t hostMessage = 0;
	GUIParameter guiParameter;		/* for single param updates */

    // --- for GUI messages
    GUIUpdateData guiUpdateData;	/* for multiple param updates */
	std::string rafxStatusWndText;
};


/**
\struct ChannelIOConfig
\ingroup Structures
\brief
Structure of a pair of channel format enumerators that set an input/output channel I/O capability.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ChannelIOConfig
{
    ChannelIOConfig ()
    : inputChannelFormat(kCFStereo)
    , outputChannelFormat(kCFStereo) {}

    ChannelIOConfig (uint32_t _inputChannelFormat,
                     uint32_t _outputChannelFormat)
    : inputChannelFormat(_inputChannelFormat)
    , outputChannelFormat(_outputChannelFormat){}

	ChannelIOConfig& operator=(const ChannelIOConfig& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		inputChannelFormat = data.inputChannelFormat;
		outputChannelFormat = data.outputChannelFormat;

		return *this;
	}

    uint32_t inputChannelFormat = kCFStereo;	///< input format for this I/O pair
    uint32_t outputChannelFormat = kCFStereo;	///< output format for this I/O pair

};

/**
\struct midiEvent
\ingroup Structures
\brief
Information about a MIDI event.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct midiEvent
{
	midiEvent(uint32_t _midiMessage, uint32_t _midiChannel, uint32_t _midiData1, uint32_t _midiData2, uint32_t _midiSampleOffset)
		: midiMessage(_midiMessage)
		, midiChannel(_midiChannel)
		, midiData1(_midiData1)
		, midiData2(_midiData2)
		, midiSampleOffset(_midiSampleOffset)
	{
		midiPitchBendValue = 0;
		midiNormalizedPitchBendValue = 0.f;
		midiIsDirty = false;
		auxUintData1 = 0;
		auxUintData2 = 0;
		auxIntData1 = -1;
		auxIntData2 = -1;
		auxDoubleData1 = 0.0;
		auxDoubleData2 = 0.0;
	}

	midiEvent(uint32_t _midiMessage, uint32_t _midiChannel, uint32_t _midiData1, uint32_t _midiData2, uint32_t _midiSampleOffset, double _audioTimeStamp)
		: midiMessage(_midiMessage)
		, midiChannel(_midiChannel)
		, midiData1(_midiData1)
		, midiData2(_midiData2)
		, midiSampleOffset(_midiSampleOffset)
		, audioTimeStamp(_audioTimeStamp)
	{
		midiPitchBendValue = 0;
		midiNormalizedPitchBendValue = 0.f;
		midiIsDirty = false;
		auxUintData1 = 0;
		auxUintData2 = 0;
		auxIntData1 = -1;
		auxIntData2 = -1;
		auxDoubleData1 = 0.0;
		auxDoubleData2 = 0.0;
	}

	midiEvent ()
    : midiMessage(0)
    , midiChannel(0)
    , midiData1(0)
    , midiData2(0)
    , midiSampleOffset(0)
	, auxUintData1(0)
	, auxUintData2(0)
	, auxIntData1(-1)
	, auxIntData2(-1)
	, auxDoubleData1(0.0)
	, auxDoubleData2(0.0)
    , midiPitchBendValue(0)
    , midiNormalizedPitchBendValue(0)
    , midiIsDirty(0)
	, audioTimeStamp(0.0){}

	midiEvent& operator=(const midiEvent& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		midiMessage = data.midiMessage;
		midiChannel = data.midiChannel;
		midiData1 = data.midiData1;
		midiData2 = data.midiData2;
        midiSampleOffset = data.midiSampleOffset;
        auxUintData1 = data.auxUintData1;
		auxUintData2 = data.auxUintData2;
		auxIntData1 = data.auxIntData1;
		auxIntData2 = data.auxIntData2;
		auxDoubleData1 = data.auxDoubleData1;
		auxDoubleData2 = data.auxDoubleData2;
        midiPitchBendValue = data.midiPitchBendValue;
		midiNormalizedPitchBendValue = data.midiNormalizedPitchBendValue;
		midiIsDirty = data.midiIsDirty;
		audioTimeStamp = data.audioTimeStamp;

		return *this;
	}

	uint32_t    midiMessage = 0;			///< BYTE message as UINT
	uint32_t    midiChannel = 0;			///< BYTE channel as UINT
	uint32_t    midiData1 = 0;				///< BYTE data 1 as UINT
	uint32_t    midiData2 = 0;				///< BYTE data 2 as UINT
	uint32_t    midiSampleOffset = 0;		///< sample offset of midi event within audio buffer
	uint32_t    auxUintData1 = 0;			///< aux data (UINT)
	uint32_t    auxUintData2 = 0;			///< aux data (UINT)
	int32_t     auxIntData1 = 0;			///< aux data (INT)
	int32_t     auxIntData2 = 0;			///< aux data (INT)
	double		auxDoubleData1 = 0.0;		///< aux data (double)
	double		auxDoubleData2 = 0.0;		///< aux data (double)
	int         midiPitchBendValue = 0;		///< midi pitch bend value (14-bit)
	float       midiNormalizedPitchBendValue = 0.0;///< normalized bitch bend value
	bool        midiIsDirty = false;		///< dirty flag
	double		audioTimeStamp = 0.0;		///< time stamp (not all APIs)
};


/**
\enum messageType
\ingroup Constants-Enums
\brief
Message identifier for ASPiK Core messaging system.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum messageType {
	PLUGINGUI_DIDOPEN,						 /* called after successful population of GUI frame, NOT called with GUI_USER_CUSTOMOPEN*/
	PLUGINGUI_WILLCLOSE,					 /* called before window is destroyed, NOT called with GUI_USER_CUSTOM_CLOSE */
	PLUGINGUI_TIMERPING,					/* timer ping for custom views */
	PLUGINGUI_REGISTER_CUSTOMVIEW,			/* register a custom view */
	PLUGINGUI_DE_REGISTER_CUSTOMVIEW,		/* un-register a custom view */
	PLUGINGUI_REGISTER_SUBCONTROLLER,		/* register a subcontroller */
	PLUGINGUI_DE_REGISTER_SUBCONTROLLER,	/* un-register a subcontroller */
	PLUGINGUI_QUERY_HASUSERCUSTOM,			/* CUSTOM GUI - reply in bHasUserCustomView */
	PLUGINGUI_USER_CUSTOMOPEN,				/* CUSTOM GUI - create your custom GUI, you must supply the code */
	PLUGINGUI_USER_CUSTOMCLOSE,				/* CUSTOM GUI - destroy your custom GUI, you must supply the code */
	PLUGINGUI_USER_CUSTOMSYNC,				/* CUSTOM GUI - re-sync the GUI */
	PLUGINGUI_EXTERNAL_SET_NORMVALUE,		// for VST3??
	PLUGINGUI_EXTERNAL_SET_ACTUALVALUE,
	PLUGINGUI_EXTERNAL_GET_NORMVALUE,		/* currently not used */
	PLUGINGUI_EXTERNAL_GET_ACTUALVALUE,
	PLUGINGUI_PARAMETER_CHANGED,			/* for pluginCore->guiParameterChanged(nControlIndex, fValue); */
	PLUGIN_QUERY_DESCRIPTION,				/* fill in a Rafx2PluginDescriptor for host */
	PLUGIN_QUERY_PARAMETER,					/* fill in a Rafx2PluginParameter for host inMessageData = index of parameter*/
	PLUGIN_QUERY_TRACKPAD_X,
	PLUGIN_QUERY_TRACKPAD_Y,
};


/**
\struct MessageInfo
\ingroup Structures
\brief
Information that includes the message code as well as the message data.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct MessageInfo
{
    MessageInfo ()
    : message(0)
    , inMessageData(0)
    , outMessageData(0){}

    MessageInfo (uint32_t _message)
    : message(_message)
     , inMessageData(0)
    , outMessageData(0)
    {}

	MessageInfo& operator=(const MessageInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		message = data.message;
		inMessageData = data.inMessageData;
		outMessageData = data.outMessageData;
		inMessageString = data.inMessageString;
		outMessageString = data.outMessageString;

		return *this;
	}

    uint32_t message = 0;			///< message code
    void* inMessageData = nullptr;	///< incoming message data (interpretation depends on message)
    void* outMessageData = nullptr;	///< outgoing message data (interpretation depends on message)

	std::string inMessageString;	///< incoming message data as a std::string (interpretation depends on message)
	std::string outMessageString;	///< outgoing message data as a std::string (interpretation depends on message)
};

/**
\struct PluginInfo
\ingroup Structures
\brief
Structure that is used during the base class initilize( ) funciton call, after object instantiation is complete.
This structure contains the path to the DLL itself which can be used to open/save files including pre-installed
WAV files for sample based synths.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct PluginInfo
{
	PluginInfo() {}

	PluginInfo& operator=(const PluginInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		pathToDLL = data.pathToDLL; ///< note copies pointer - somewhat useless

		return *this;
	}

	const char* pathToDLL;	///< complete path to the DLL (component) without trailing backslash
};


/**
\struct CreateGUIInfo
\ingroup Structures
\brief
Structure that is used during GUI creation to safely pass information about the GUI size and interfaces.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct CreateGUIInfo
{
	CreateGUIInfo()
		: window(0)
		, guiPluginConnector(0)
		, guiWindowFrame(0)
		, width(0.0)
		, height(0.0)
	{ }

	CreateGUIInfo(void* _window, IGUIPluginConnector* _guiPluginConnector, IGUIWindowFrame* _guiWindowFrame)
		: window(_window)
		, guiPluginConnector(_guiPluginConnector)
		, guiWindowFrame(_guiWindowFrame)
		, width(0.0)
		, height(0.0)
	{ }

	CreateGUIInfo& operator=(const CreateGUIInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		window = data.window;
		guiPluginConnector = data.guiPluginConnector;
		guiWindowFrame = data.guiWindowFrame;
		width = data.width;
		height = data.height;

		return *this;
	}

	void* window = nullptr;								///< window handle or NSView*
	IGUIPluginConnector* guiPluginConnector = nullptr;	///< GUI-to-plugin-shell interface
	IGUIWindowFrame* guiWindowFrame;					///< GUI-to-frame interface (resizing)

	// --- returned
	double width = 0.0;		///< GUI width in pixels
	double height = 0.0;	///< GUI height in pixels
};

/**
\struct ParameterUpdateInfo
\ingroup Structures
\brief
Information about a paraemeter being updated. Used when bound variables are updated. Multiple advanced uses.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ParameterUpdateInfo
{
	ParameterUpdateInfo()
		: isSmoothing(0)
		, isVSTSampleAccurateUpdate(0)
		, loadingPreset(0)
		, boundVariableUpdate(0)
		, bufferProcUpdate(0)
		, applyTaper(1){}

	ParameterUpdateInfo(bool _isSmoothing, bool _isVSTSampleAccurateUpdate)
		: isSmoothing(_isSmoothing)
		, isVSTSampleAccurateUpdate(_isVSTSampleAccurateUpdate) {
		loadingPreset = false;
		boundVariableUpdate = false;
		bufferProcUpdate = false;
		applyTaper = true;
	}

	ParameterUpdateInfo& operator=(const ParameterUpdateInfo& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		isSmoothing = data.isSmoothing;
		isVSTSampleAccurateUpdate = data.isVSTSampleAccurateUpdate;
		loadingPreset = data.loadingPreset;
		boundVariableUpdate = data.boundVariableUpdate;
		bufferProcUpdate = data.bufferProcUpdate;
		applyTaper = data.applyTaper;

		return *this;
	}

	bool isSmoothing = false;               ///< param is being (bulk) smoothed
	bool isVSTSampleAccurateUpdate = false; ///< param updated with VST sample accurate automation
	bool loadingPreset = false;				///< a preset is being loaded
	bool boundVariableUpdate = false;		///< bound variable is being udpated
	bool bufferProcUpdate = false;			///< update at top of buffer process
	bool applyTaper = true;					///< add tapering to udpate
};

/**
\enum attributeType
\ingroup Constants-Enums
\brief
AttributeType identifier for ASPiK PluginParameter auxilliary storage system. You are free to implement your own mechanism; this is an example.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
enum attributeType { isFloatAttribute, isDoubleAttribute, isIntAttribute, isUintAttribute, isBoolAttribute, isVoidPtrAttribute, isStringAttribute };

/**
\union attributeValue
\ingroup Constants-Enums
\brief
Attribute value smashed down into a union.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
union attributeValue
{
	float f;
	double d;
	int n;
	unsigned int u;
	bool b;
	void* vp;
};

/**
\struct AuxParameterAttribute
\ingroup Structures
\brief
Information about auxilliary parameter details - purely customizeable. This uses the attributeValue
union to set the attribute in multiple fashions without resorting to declaring multiple datatypes.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct AuxParameterAttribute
{
	AuxParameterAttribute()
	: attributeID(0)
	{ memset(&value, 0, sizeof(attributeValue)); }

	AuxParameterAttribute(uint32_t _attributeID)
		: attributeID(_attributeID) { }

	AuxParameterAttribute& operator=(const AuxParameterAttribute& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		value = data.value;
		attributeID = data.attributeID;
		return *this;
	}

	void reset(uint32_t _attributeID) { memset(&value, 0, sizeof(attributeValue));  attributeID = _attributeID; }

	void setFloatAttribute(float f) { value.f = f; }
	void setDoubleAttribute(double d) { value.d = d; }
	void setIntAttribute(int n) { value.n = n; }
	void setUintAttribute(unsigned int u) { value.u = u; }
	void setBoolAttribute(bool b) { value.b = b; }
	void setVoidPtrAttribute(void* vp) { value.vp = vp; }

	float getFloatAttribute( ) { return value.f; }
	double getDoubleAttribute( ) { return value.d; }
	int getIntAttribute( ) { return value.n; }
	unsigned int getUintAttribute( ) { return  value.u; }
	bool getBoolAttribute( ) { return value.b; }
	void* getVoidPtrAttribute( ) { return value.vp; }

	attributeValue value;	///< value in union form
	uint32_t attributeID = 0;	///< attribute ID
};

/**
\struct HostInfo
\ingroup Structures
\brief
Information from the host that is updated on each buffer process cycle; includes BPM, time signature, SMPTE and other data.
The values in the stock structure are consistent across most APIs, however others may be added (commnted out here)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct HostInfo
{
    // --- common to all APIs
    unsigned long long uAbsoluteFrameBufferIndex = 0;	///< the sample index at top of buffer
    double dAbsoluteFrameBufferTime = 0.0;				///< the time in seconds of the sample index at top of buffer
    double dBPM = 0.0;									///< beats per minute, aka "tempo"
    float fTimeSigNumerator = 0.f;						///< time signature numerator
    uint32_t uTimeSigDenomintor = 0;					///< time signature denominator

    // --- VST3 Specific: note these use same variable names as VST3::struct ProcessContext
    //     see ..\VST3 SDK\pluginterfaces\vst\ivstprocesscontext.h for information on decoding these
    //
    uint32_t state = 0;					///< a combination of the values from \ref StatesAndFlags; use to decode validity of other VST3 items in this struct
    long long systemTime = 0;			///< system time in nanoseconds (optional)
    double continousTimeSamples = 0.0;	///< project time, without loop (optional)
    double projectTimeMusic = 0.0;		///< musical position in quarter notes (1.0 equals 1 quarter note)
    double barPositionMusic = 0.0;		///< last bar start position, in quarter notes
    double cycleStartMusic = 0.0;		///<- cycle start in quarter notes
    double cycleEndMusic = 0.0;			///< cycle end in quarter notes
    uint32_t samplesToNextClock = 0;	///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest)
	bool enableVSTSampleAccurateAutomation = false;
	/*
     IF you need SMPTE information, you need to get the information yourself at the start of the process( ) function
     where the above values are filled out. See the variables here in VST3 SDK\pluginterfaces\vst\ivstprocesscontext.h:

     int32 smpteOffsetSubframes = 0;	// --- SMPTE (sync) offset in subframes (1/80 of frame)
     FrameRate frameRate;				// --- frame rate
     */

    // --- AU Specific
    //     see AUBase.h for definitions and information on decoding these
    //
    double dCurrentBeat = 0.0;					///< current DAW beat value
	bool bIsPlayingAU = false;					///< notorously incorrect in Logic - once set to true, stays stuck there
    bool bTransportStateChanged = false;		///< only notifies a change, but not what was changed to...
    uint32_t nDeltaSampleOffsetToNextBeat = 0;	///< samples to next beat
    double dCurrentMeasureDownBeat = 0.0;		///< current downbeat
    bool bIsCycling = false;					///< looping
    double dCycleStartBeat = 0.0;				///< loop start
    double dCycleEndBeat = 0.0;					///< loop end

    // --- AAX Specific
    //     see AAX_ITransport.h for definitions and information on decoding these
    bool bIsPlayingAAX = false;					///< flag if playing
    long long nTickPosition = 0;				///< "Tick" is represented here as 1/960000 of a quarter note
    bool bLooping = false;						///< looping flag
    long long nLoopStartTick = 0;				///< start tick for loop
    long long nLoopEndTick = 0;					///< end tick for loop
    /*
     NOTE: there are two optional functions that cause a performance hit in AAX; these are commented outs;
	 if you decide to use them, you should re-locate them to a non-realtime thread. Use at your own risk!

     int32_t nBars = 0;
     int32_t nBeats = 0;
     int64_t nDisplayTicks = 0;
     int64_t nCustomTickPosition = 0;

     // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
     midiTransport->GetBarBeatPosition(&nBars, &nBeats, &nDisplayTicks, nAbsoluteSampleLocation);

     // --- There is a minor performance cost associated with using this API in Pro Tools. It should NOT be used excessively without need
     midiTransport->GetCustomTickPosition(&nCustomTickPosition, nAbsoluteSampleLocation);

     NOTE: if you need SMPTE or metronome information, you need to get the information yourself at the start of the ProcessAudio( ) function
			  see AAX_ITransport.h for definitions and information on decoding these
     virtual	AAX_Result GetTimeCodeInfo(AAX_EFrameRate* oFrameRate, int32_t* oOffset) const = 0;
     virtual	AAX_Result GetFeetFramesInfo(AAX_EFeetFramesRate* oFeetFramesRate, int64_t* oOffset) const = 0;
     virtual AAX_Result IsMetronomeEnabled(int32_t* isEnabled) const = 0;
     */
};

class IMidiEventQueue;

/**
\struct ProcessBufferInfo
\ingroup Structures
\brief
Information package that arrives with each new audio buffer process cycle. Contains everything needed for one buffer's worth of data.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ProcessBufferInfo
{
    ProcessBufferInfo(){}

	/*
	AAX -- MUST be float
	AU --- Float32
	RAFX2 --- float
	VST3 --- float OR double
	 *  \brief Subscribes an audio input context field
	 *
	 *  Defines an audio in port for host-provided information in the algorithm's
	 *	context structure.
     *
     *  - Data type: float**
     *  - Data kind: An array of float arrays, one for each input channel
	 *
     */
	// --- audio inputs and outputs (arrays of channel-array pointers)
	float** inputs = nullptr;		///< audio input buffers
	float** outputs = nullptr;		///< audio output buffers
	float** auxInputs = nullptr;	///< aux (sidechain) input buffers
	float** auxOutputs = nullptr;	///< aux outputs - for future use
	uint32_t numAudioInChannels = 0;		///< audio input channel count
	uint32_t numAudioOutChannels = 0;		///< audio output channel count
	uint32_t numAuxAudioInChannels = 0;		///< aux input channel count
	uint32_t numAuxAudioOutChannels = 0;	///< aux output channel count (not used)

	uint32_t numFramesToProcess = 0;		///< frame count in this buffer
    ChannelIOConfig channelIOConfig;		///< input/output channel I/O configuration pair
    ChannelIOConfig auxChannelIOConfig;		///< aux input/output channel I/O configuration pair

	// --- for future use, VCVRack
	float* controlSignalInputs = nullptr;	///< control signals in (reserved for future use)
	float* controlSignalOutputs = nullptr;	///< control signals out (reserved for future use)
	uint32_t numControlSignalInputs = 0;	///< num control signals in (reserved for future use)
	uint32_t numControlSignalOutputs = 0;	///< num control signals out (reserved for future use)

	// --- should make these const?
    HostInfo* hostInfo = nullptr;			///< pointer to host data for this buffer
    IMidiEventQueue* midiEventQueue = nullptr;	///< MIDI event queue
};

/**
\struct ProcessFrameInfo
\ingroup Structures
\brief
Information package that arrives with each new audio frame; called internally from the buffer process function.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct ProcessFrameInfo
{
    ProcessFrameInfo(){ }

	float* audioInputFrame = nullptr;		///< audio input frame (array)
	float* audioOutputFrame = nullptr;		///< audio output frame (array)
	float* auxAudioInputFrame = nullptr;	///< aux input frame (array)
	float* auxAudioOutputFrame = nullptr;	///< aux output frame (array) for future use

	uint32_t numAudioInChannels = 0;		///< audio input channel count
	uint32_t numAudioOutChannels = 0;		///< audio input channel count
	uint32_t numAuxAudioInChannels = 0;		///< audio input channel count
	uint32_t numAuxAudioOutChannels = 0;	///< audio input channel count

    ChannelIOConfig channelIOConfig;	///< input/output channel I/O configuration pair
    ChannelIOConfig auxChannelIOConfig;	///< aux input/output channel I/O configuration pair
	uint32_t currentFrame = 0;			///< index of this frame

	// --- for future use, VCVRack
	float* controlSignalInputs = nullptr;	///< control signals in (reserved for future use)
	float* controlSignalOutputs = nullptr;	///< control signals out (reserved for future use)
	uint32_t numControlSignalInputs = 0;	///< num control signals in (reserved for future use)
	uint32_t numControlSignalOutputs = 0;	///< num control signals out (reserved for future use)

	// --- should make these const?
	HostInfo* hostInfo = nullptr;			///< pointer to host data for this buffer
	IMidiEventQueue* midiEventQueue = nullptr;	///< MIDI event queue
};

/**
\struct AudioProcDescriptor
\ingroup Structures
\brief
Information package about the current DAW session. Sample rate and bit-depth of audio.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct AudioProcDescriptor
{
    AudioProcDescriptor ()
    : sampleRate(44100)
    , bitDepth(16){}

    AudioProcDescriptor (double _sampleRate,
                         uint32_t _bitDepth)
    : sampleRate(_sampleRate)
    , bitDepth(_bitDepth){}

	AudioProcDescriptor& operator=(const AudioProcDescriptor& data)	// need this override for collections to work
	{
		if (this == &data)
			return *this;

		sampleRate = data.sampleRate;
		bitDepth = data.bitDepth;
		return *this;
	}

    double sampleRate = 44100.0;	///< sample rate
    uint32_t bitDepth = 16;			///< wav file bit depth (not supported in all APIs)
};


/**
\struct PluginDescriptor
\ingroup Structures
\brief
Information package about the plugin itself, consisting mainly of simple strings and ID values.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct PluginDescriptor
{
    PluginDescriptor ()
    : pluginName("Long Plugin Name") // max 31 chars
    , shortPluginName("ShortPIName") // max 15 chars
    , vendorName("Plugin Developer")
    , pluginTypeCode(pluginType::kFXPlugin) // FX or synth
    , hasSidechain(0)
    , processFrames(1)                  /* default operation */
    , wantsMIDI(1)                      /* default operation */
    , hasCustomGUI(1)
    , latencyInSamples(0)
    , tailTimeInMSec(0)
    , infiniteTailVST3(0)
    , numSupportedIOCombinations(0)
    , supportedIOCombinations(0)
    , numSupportedAuxIOCombinations(0)
    , supportedAuxIOCombinations(0)
    {}

    // --- string descriptors
    std::string pluginName;			///< name (up to 31 chars)
    std::string shortPluginName;	///< name (up to 15 chars)
    std::string vendorName;			///< manufacturer name
    uint32_t pluginTypeCode = 0;	///< FX or synth

    bool hasSidechain = false;		///< sidechain flag
    bool processFrames = true;		///< want frames (default)
    bool wantsMIDI = true;			///< want MIDI (don't need to actually use it)
    bool hasCustomGUI = true;		///< default on
    uint32_t latencyInSamples = 0;	///< latency
    double tailTimeInMSec = 0.0;	///< tail time
    bool infiniteTailVST3 = false;	///< VST3 infinite tail flag

    uint32_t numSupportedIOCombinations = 0;	///< should support at least main 3 combos
    ChannelIOConfig* supportedIOCombinations;

    uint32_t numSupportedAuxIOCombinations = 0;///< should support at least main 3 combos
    ChannelIOConfig* supportedAuxIOCombinations;

    /** for AU plugins, returns default setting */
    uint32_t getDefaultChannelIOConfigForChannelCount(uint32_t channelCount)
    {
        switch(channelCount)
        {
            case 0:
                return kCFNone;
            case 1:
                return kCFMono;
            case 2:
                return kCFStereo;
            case 3:
                return kCFLCR;
            case 4:
                return kCFQuad; // or kCFLCR
            case 5:
                return kCF5p0;
            case 6:
                return kCF5p1; // or kCF6p0
            case 7:
                return kCF6p1; // or kCF7p0Sony kCF7p0DTS
            case 8:
                return kCF7p1DTS; // or kCF7p1Sony or kCF7p1Proximity
            case 9:
                return kCF8p1; // or kCF9p0
            case 10:
                return kCF9p1; // or kCF10p0
            case 11:
                return kCF10p1;
            case 12:
                return kCF11p1; // or kCF10p2
            case 13:
                return kCF13p0; // or kCF12p2
            case 14:
                return kCF13p1;
            case 24:
                return kCF22p2;

            default:
                return 0;
        }
    }
	/** for AU plugins, this converts channel I/0 confirguration into an integer channel count value */
    uint32_t getChannelCountForChannelIOConfig(uint32_t format)
    {
        switch(format)
        {
            case kCFNone:
                return 0;

            case kCFMono:
                return 1;
            case kCFStereo:
                return 2;
            case kCFLCR:
                return 3;

            case kCFQuad:
            case kCFLCRS:
                return 4;

            case kCF5p0:
                return 5;

            case kCF5p1:
            case kCF6p0:
                return 6;

            case kCF6p1:
            case kCF7p0Sony:
            case kCF7p0DTS:
                return 7;

            case kCF7p1Sony:
            case kCF7p1DTS:
            case kCF7p1Proximity:
                return 8;

            case kCF8p1:
            case kCF9p0:
                return 9;

            case kCF9p1:
            case kCF10p0:
                return 10;

            case kCF10p1:
                return 11;

            case kCF10p2:
            case kCF11p1:
                return 12;

            case kCF13p0:
            case kCF12p2:
                return 13;

            case kCF13p1:
                return 14;

            case kCF22p2:
                return 24;

            default:
                return 0;
        }
        return 0;
    }
};

/**
\struct JSControl
\ingroup Structures
\brief
Information package a joystick or trackpad GUI interaction.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
struct JSControl
{
	JSControl() {}
	JSControl& operator=(const JSControl& aControl)
	{
		if (this == &aControl)
			return *this;

		trackpadIndex = aControl.trackpadIndex;
		midiControl = aControl.midiControl;
		midiControlCommand = aControl.midiControlCommand;
		midiControlName = aControl.midiControlName;
		midiControlChannel = aControl.midiControlChannel;
		joystickValue = aControl.joystickValue;
		korgVectorJoystickOrientation = aControl.korgVectorJoystickOrientation;
		enableParamSmoothing = aControl.enableParamSmoothing;
		smoothingTimeInMs = aControl.smoothingTimeInMs;
		return *this;
	}

	int32_t trackpadIndex = -1;			///< trackpad or joystick index
	bool midiControl = false;			///< MIDI enabled
	uint32_t midiControlCommand = 0;	///< MIDI CC type
	uint32_t midiControlName = 0;		///< MIDI CC
	uint32_t midiControlChannel = 0;	///< MIDI CC Channel
	double joystickValue = 0.0;			///< joystick value as a double
	bool korgVectorJoystickOrientation = false; ///< vector joystick orientation
	bool enableParamSmoothing = false;	///< param smoothing on joystick (can be CPU abusive)
	double smoothingTimeInMs = 0.0;		///< JS smoothing time
};


// --------------------------------------------------------------------------------------------------------------------------- //
// --- INTERFACES
// --------------------------------------------------------------------------------------------------------------------------- //

/**
\class ICustomView
\ingroup Interfaces
\brief
Custom View interface to allow plugin core to create safe communication channels with GUI custom view objects. MANY uses!

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class ICustomView
{
public:
	/** function to tell the view to redraw after pushing data to it, or manipulating it */
	virtual void updateView() = 0;

	/**    push a new data value into the view; call this repeatedly to push multiple values\n
	//     The derived class should implement a lock-free ring buffer to store the data.\n
	//     If you need to move giant amounts of data, you might want to alter this interface\n
	//     and add a mechanism to copy a pointer to a lock-free buffer or some other\n
	//     thread-safe mechanism that you design */
	virtual void pushDataValue(double data) { }

	/**    send a message into the view
	//     The derived class should implement a lock-free ring buffer to store the message.\n
	//     and handle all messaging in a thread-safe manner\n
	//     argument: void* data\n
	//               The argument is any struct, object, or variable pointer cloaked as a void*\n
	//               Note that since the argument is variable and the programmer sets it\n
	//               you can send information back to the plugin via this call as well\n
	//               !!! --- BE CAREFUL and don't do anything unsafe with this messaging system --- !!! */
	virtual void sendMessage(void* data) { }
};

/**
\class IGUIWindowFrame
\ingroup Interfaces
\brief
Custom interface to allow resizing of GUI window; this is mainly used for the GUI designer.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IGUIWindowFrame
{
public:
	/** set the frame size */
	virtual bool setWindowFrameSize(double left = 0, double top = 0, double right = 0, double bottom = 0) = 0;

	/** get the frame size */
	virtual bool getWindowFrameSize(double& left, double&  top, double&  right, double&  bottom) = 0;

	/** enable GUI designer (not used) */
	virtual void enableGUIDesigner(bool enable) { }
};

/**
\class IGUIView
\ingroup Interfaces
\brief
Custom interface to allow resizing of GUI window; this is mainly used for the GUI designer.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IGUIView
{
public:
	/** set the interface pointer */
	virtual void setGUIWindowFrame(IGUIWindowFrame* frame) = 0;
};


/**
\class IGUIPluginConnector
\ingroup Interfaces
\brief
Custom interface so that GUI can pass information to plugin shell in a thread-safe manner.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IGUIPluginConnector
{
public:
	/** GUI registers ICustomView* for Custom View Objects (see www.willpirkle.com) */
	virtual bool registerCustomView(std::string customViewName, ICustomView* customViewConnector) = 0;

	/** GUI de-registers ICustomView* for Custom View Objects (see www.willpirkle.com) */
	virtual bool deRegisterCustomView(ICustomView* customViewConnector) = 0;

	/** called AFTER GUI did open but before it is shown */
	virtual bool guiDidOpen() = 0;

	/** called before GUI closes */
	virtual bool guiWillClose() = 0;

	/** called once per timer ping on the GUI thread */
	virtual bool guiTimerPing() = 0;

	/** GUI registers ICustomView* for sub-controllers (see www.willpirkle.com) */
	virtual bool registerSubcontroller(std::string subcontrollerName, ICustomView* customViewConnector) { return false; }

	/** GUI de-registers ICustomView* for sub-controllers (see www.willpirkle.com) */
	virtual bool deRregisterSubcontroller(ICustomView* customViewConnector) { return false; }

	/** non-bound variable count*/
	virtual uint32_t getNonBoundVariableCount() { return 0; }

	/** get tag */
	virtual uint32_t getNextNonBoundVariableTag(int startTag) { return -1; }

	/** non-bound variable count check value changed*/
	virtual bool checkNonBoundValueChange(int tag, float normalizedValue) { return false; }

	/** for sending GUI udates only, does not change underlying control variable! */
	virtual void checkSendUpdateGUI(int tag, float actualValue, bool loadingPreset, void* data1 = 0, void* data2 = 0) {}

	/**  parameter has changed, derived object handles this in a thread-safe manner */
	virtual void parameterChanged(int32_t controlID, double actualValue, double normalizedValue) {}

	/**  get plugin parameter as normalize value */
	virtual double getNormalizedPluginParameter(int32_t controlID) { return 0.0; }

	/**  set plugin parameter with normalize value */
	virtual void setNormalizedPluginParameter(int32_t controlID, double value) { }

	/**  get plugin parameter as actual value */
	virtual double getActualPluginParameter(int32_t controlID) { return 0.0; }

	/**  set plugin parameter with actual value */
	virtual void setActualPluginParameter(int32_t controlID, double value) { }

	/**   AAX automation touch */
    virtual void beginParameterChangeGesture(int controlTag){ }

	/**   AAX automation release */
	virtual void endParameterChangeGesture(int controlTag){ }
};


/**
\class IPluginHostConnector
\ingroup Interfaces
\brief
Custom interface to send the plugin shell a message from plugin core.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IPluginHostConnector
{
public:
	/** send a message to the host */
	virtual void sendHostMessage(const HostMessageInfo& hostMessageInfo) = 0;
};

/**
\class IMidiEventQueue
\ingroup Interfaces
\brief
Double buffered queue for MIDI messages.

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IMidiEventQueue
{
public:
	/** Get the event count (extra, does not really need to be used) */
	virtual uint32_t getEventCount() = 0;

	/** Fire off the next <IDI event */
	virtual bool fireMidiEvents(uint32_t uSampleOffset) = 0;
};


/**
\class IParameterUpdateQueue
\ingroup Interfaces
\brief
Interface for VST3 parameter value update queue (sample accurate automation)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class IParameterUpdateQueue
{
public:
	/** Get the index number associated with the parameter */
	virtual uint32_t getParameterIndex() = 0;

	/**    Get the sample-accurate value of the parameter at the given sample offset. Pass in the last known normalized value.
	//     Returns true if dPreviousValue != dNextValue */
	virtual bool getValueAtOffset(long int _sampleOffset, double _previousValue, double& _nextValue) = 0;

	/**    Get the sample-accurate value of the parameter at the next sample offset, determined by an internal counter
	//     Returns true if dNextValue is different than the previous value */
	virtual bool getNextValue(double& _nextValue) = 0;
};

// --------------------------------------------------------------------------------------------------------------------------- //
// --- HELPER FUNCTIONS
// --------------------------------------------------------------------------------------------------------------------------- //
/**
@numberToString
\ingroup ASPiK-GUI

@brief converts unsigned int value to std::string

\param number - unsigned int to convert
\return number as a std::string
*/
inline std::string numberToString(unsigned int number)
{
    std::ostringstream strm;
    strm << number;
    std::string str = strm.str();
    return str;
}

/**
@numberToString
\ingroup ASPiK-GUI

@brief converts int value to std::string

\param number - int value to convert
\return number as a std::string
*/
inline std::string numberToString(int number)
{
    std::ostringstream strm;
    strm << number;
    std::string str = strm.str();
    return str;
}

/**
@numberToString
\ingroup ASPiK-GUI

@brief converts float value to std::string

\param number - float value to convert
\return number as a std::string
*/
inline std::string numberToString(float number)
{
    std::ostringstream strm;
    strm << number;
    std::string str = strm.str();
    return str;
}

/**
@numberToString
\ingroup ASPiK-GUI

@brief converts double value to std::string

\param number - double value to convert
\return number as a std::string
*/
inline std::string numberToString(double number)
{
    std::ostringstream strm;
    strm << number;
    std::string str = strm.str();
    return str;
}

/**
@boolToStdString
\ingroup ASPiK-GUI

@brief converts bool value to std::string

\param value - bool to convert
\return number as a std::string
*/
inline std::string boolToStdString(bool value)
{
	std::string returnString;
	if (value) returnString.assign("true");
	else returnString.assign("false");
	return returnString;
}


#endif //_pluginstructures_h
