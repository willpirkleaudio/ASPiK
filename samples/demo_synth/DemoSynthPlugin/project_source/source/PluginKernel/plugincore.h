// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugincore.h
//
/**
    \file   plugincore.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class interface file for ASPiK plugincore object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef __pluginCore_h__
#define __pluginCore_h__

#include "pluginbase.h"

// **--0x7F1F--**
enum controlID{
    volume_dB
};

// **--0x0F1F--**

/**
\class PluginCore
\ingroup ASPiK-Core
\brief
The PluginCore object is the default PluginBase derived object for ASPiK projects.
Note that you are fre to change the name of this object (as long as you change it in the compiler settings, etc...)


PluginCore Operations:
- overrides the main processing functions from the base class
- performs reset operation on sub-modules
- processes audio
- processes messages for custom views
- performs pre and post processing functions on parameters and audio (if needed)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginCore : public PluginBase
{
public:
    PluginCore();

	/** Destructor: empty in default version */
    virtual ~PluginCore(){}

	// --- PluginBase Overrides ---
	//
	/** this is the creation function for all plugin parameters */
	bool initPluginParameters();

	/** called when plugin is loaded, a new audio file is playing or sample rate changes */
	virtual bool reset(ResetInfo& resetInfo);

	/** one-time post creation init function; pluginInfo contains path to this plugin */
	virtual bool initialize(PluginInfo& _pluginInfo);

	// --- preProcess: sync GUI parameters here; override if you don't want to use automatic variable-binding
	virtual bool preProcessAudioBuffers(ProcessBufferInfo& processInfo);

	/** process frames of data */
	virtual bool processAudioFrame(ProcessFrameInfo& processFrameInfo);

	// --- uncomment and override this for buffer processing; see base class implementation for
	//     help on breaking up buffers and getting info from processBufferInfo
	//virtual bool processAudioBuffers(ProcessBufferInfo& processBufferInfo);

	/** preProcess: do any post-buffer processing required; default operation is to send metering data to GUI  */
	virtual bool postProcessAudioBuffers(ProcessBufferInfo& processInfo);

	/** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
	virtual bool updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

	/** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
	virtual bool updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo);

	/** this can be called: 1) after bound variable has been updated or 2) after smoothing occurs  */
	virtual bool postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

	/** this is ony called when the user makes a GUI control change */
	virtual bool guiParameterChanged(int32_t controlID, double actualValue);

	/** processMessage: messaging system; currently used for custom/special GUI operations */
	virtual bool processMessage(MessageInfo& messageInfo);

	/** processMIDIEvent: MIDI event processing */
	virtual bool processMIDIEvent(midiEvent& event);

	/** specialized joystick servicing (currently not used) */
	virtual bool setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData);

	/** create the presets */
	bool initPluginPresets();

	// --- BEGIN USER VARIABLES AND FUNCTIONS -------------------------------------- //
	//	   Add your variables and methods here
    double phaseInc = 0.0;
    double moduloCounter = 0.0;
    bool noteOn = false;


	// --- END USER VARIABLES AND FUNCTIONS -------------------------------------- //

private:
	//  **--0x07FD--**
    double volume_dB = 0.000000;

	// **--0x1A7F--**
    // --- end member variables

public:
    /** static description: bundle folder name

	\return bundle folder name as a const char*
	*/
    static const char* getPluginBundleName();

    /** static description: name

	\return name as a const char*
	*/
    static const char* getPluginName();

	/** static description: short name

	\return short name as a const char*
	*/
	static const char* getShortPluginName();

	/** static description: vendor name

	\return vendor name as a const char*
	*/
	static const char* getVendorName();

	/** static description: URL

	\return URL as a const char*
	*/
	static const char* getVendorURL();

	/** static description: email

	\return email address as a const char*
	*/
	static const char* getVendorEmail();

	/** static description: Cocoa View Factory Name

	\return Cocoa View Factory Name as a const char*
	*/
	static const char* getAUCocoaViewFactoryName();

	/** static description: plugin type

	\return type (FX or Synth)
	*/
	static pluginType getPluginType();

	/** static description: VST3 GUID

	\return VST3 GUID as a const char*
	*/
	static const char* getVSTFUID();

	/** static description: 4-char code

	\return 4-char code as int
	*/
	static int32_t getFourCharCode();

	/** initalizer */
	bool initPluginDescriptors();

};



//----------------------------------------------------------------
// --- MIDI Constants
//----------------------------------------------------------------
//
// --- CHANNEL VOICE MESSAGES
const unsigned char NOTE_OFF = 0x80;
const unsigned char NOTE_ON = 0x90;
const unsigned char POLY_PRESSURE = 0xA0;
const unsigned char CONTROL_CHANGE = 0xB0;
const unsigned char PROGRAM_CHANGE = 0xC0;
const unsigned char CHANNEL_PRESSURE = 0xD0;
const unsigned char PITCH_BEND = 0xE0;

// --- CONTINUOUS CONTROLLERS
const unsigned char MOD_WHEEL = 0x01;
const unsigned char VOLUME_CC07 = 0x07;
const unsigned char PAN_CC10 = 0x0A;
const unsigned char EXPRESSION_CC11 = 0x0B;
const unsigned char JOYSTICK_X = 0x10;
const unsigned char JOYSTICK_Y = 0x11;
const unsigned char SUSTAIN_PEDAL = 0x40;
const unsigned char RESET_ALL_CONTROLLERS = 0x79;
const unsigned char ALL_NOTES_OFF = 0x7B;

// --- SYSTEM MESSAGES
const unsigned char SYSTEM_EXCLUSIVE = 0xF0;
const unsigned char MIDI_TIME_CODE = 0xF1;
const unsigned char SONG_POSITION_POINTER = 0xF2;
const unsigned char SONG_SELECT = 0xF3;
const unsigned char TUNE_REQUEST = 0xF6;
const unsigned char END_OF_EXCLUSIVE = 0xF7;
const unsigned char TIMING_CLOCK = 0xF8;
const unsigned char START = 0xFA;
const unsigned char CONTINUE = 0xFB;
const unsigned char SToP = 0xFC;
const unsigned char ACTIVE_SENSING = 0xFE;
const unsigned char SYSTEM_RESET = 0xFF;

// --- FOR Synth Projects
enum midiChannels {
    MIDI_CH_1 = 0, MIDI_CH_2, MIDI_CH_3, MIDI_CH_4, MIDI_CH_5, MIDI_CH_6, MIDI_CH_7,
    MIDI_CH_8, MIDI_CH_9, MIDI_CH_10, MIDI_CH_11, MIDI_CH_12, MIDI_CH_13,
    MIDI_CH_14, MIDI_CH_15, MIDI_CH_16, MIDI_CH_ALL
};

inline double unipolarToBipolar(double value)
{
    return 2.0*value - 1.0;
}

const float midiFreqTable[128] = {
    8.1757993698120117,
    8.6619567871093750,
    9.1770238876342773,
    9.7227182388305664,
    10.3008613586425780,
    10.9133825302124020,
    11.5623254776000980,
    12.2498569488525390,
    12.9782714843750000,
    13.7500000000000000,
    14.5676174163818360,
    15.4338531494140630,
    16.3515987396240230,
    17.3239135742187500,
    18.3540477752685550,
    19.4454364776611330,
    20.6017227172851560,
    21.8267650604248050,
    23.1246509552001950,
    24.4997138977050780,
    25.9565429687500000,
    27.5000000000000000,
    29.1352348327636720,
    30.8677062988281250,
    32.7031974792480470,
    34.6478271484375000,
    36.7080955505371090,
    38.8908729553222660,
    41.2034454345703130,
    43.6535301208496090,
    46.2493019104003910,
    48.9994277954101560,
    51.9130859375000000,
    55.0000000000000000,
    58.2704696655273440,
    61.7354125976562500,
    65.4063949584960940,
    69.2956542968750000,
    73.4161911010742190,
    77.7817459106445310,
    82.4068908691406250,
    87.3070602416992190,
    92.4986038208007810,
    97.9988555908203130,
    103.8261718750000000,
    110.0000000000000000,
    116.5409393310546900,
    123.4708251953125000,
    130.8127899169921900,
    138.5913085937500000,
    146.8323822021484400,
    155.5634918212890600,
    164.8137817382812500,
    174.6141204833984400,
    184.9972076416015600,
    195.9977111816406200,
    207.6523437500000000,
    220.0000000000000000,
    233.0818786621093700,
    246.9416503906250000,
    261.6255798339843700,
    277.1826171875000000,
    293.6647644042968700,
    311.1269836425781200,
    329.6275634765625000,
    349.2282409667968700,
    369.9944152832031200,
    391.9954223632812500,
    415.3046875000000000,
    440.0000000000000000,
    466.1637573242187500,
    493.8833007812500000,
    523.2511596679687500,
    554.3652343750000000,
    587.3295288085937500,
    622.2539672851562500,
    659.2551269531250000,
    698.4564819335937500,
    739.9888305664062500,
    783.9908447265625000,
    830.6093750000000000,
    880.0000000000000000,
    932.3275146484375000,
    987.7666015625000000,
    1046.5023193359375000,
    1108.7304687500000000,
    1174.6590576171875000,
    1244.5079345703125000,
    1318.5102539062500000,
    1396.9129638671875000,
    1479.9776611328125000,
    1567.9816894531250000,
    1661.2187500000000000,
    1760.0000000000000000,
    1864.6550292968750000,
    1975.5332031250000000,
    2093.0046386718750000,
    2217.4609375000000000,
    2349.3181152343750000,
    2489.0158691406250000,
    2637.0205078125000000,
    2793.8259277343750000,
    2959.9553222656250000,
    3135.9633789062500000,
    3322.4375000000000000,
    3520.0000000000000000,
    3729.3100585937500000,
    3951.0664062500000000,
    4186.0092773437500000,
    4434.9218750000000000,
    4698.6362304687500000,
    4978.0317382812500000,
    5274.0410156250000000,
    5587.6518554687500000,
    5919.9106445312500000,
    6271.9267578125000000,
    6644.8750000000000000,
    7040.0000000000000000,
    7458.6201171875000000,
    7902.1328125000000000,
    8372.0185546875000000,
    8869.8437500000000000,
    9397.2724609375000000,
    9956.0634765625000000,
    10548.0820312500000000,
    11175.3037109375000000,
    11839.8212890625000000,
    12543.8535156250000000 };







#endif /* defined(__pluginCore_h__) */
