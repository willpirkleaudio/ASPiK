// -----------------------------------------------------------------------------
//    ASPiK VST Shell File:  channelformats.h
//
/**
    \file   channelformats.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  steinberg VST speaker arrangements and conversion routine to
    		identify as ASPiK Channel Format specifier

			- included in ASPiK(TM) VST Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and a
    		  VST Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef VTestA_channelformats_h
#define VTestA_channelformats_h

#include "pluginterfaces/vst/vsttypes.h"

/**
@NUM_SUPPORTED_CHANNEL_FORMATS
\ingroup VST-Shell

@brief number of supported channel I/O combinations
*/
const unsigned int NUM_SUPPORTED_CHANNEL_FORMATS = 15;

namespace Steinberg {
namespace Vst {
namespace ASPiK {

/**
	@speakerArrangements
	\ingroup VST-Shell

	@brief fixed array of supported VST3 speaker arrangements you can add more here if you want to support more channel arrangements;
	NOTE: these were chosen to match with AU and AAX channal I/O support.
*/
SpeakerArrangement speakerArrangements[NUM_SUPPORTED_CHANNEL_FORMATS] =
{SpeakerArr::kEmpty, SpeakerArr::kMono, SpeakerArr::kStereo, SpeakerArr::k30Cine,
    SpeakerArr::k31Cine, SpeakerArr::k40Music, SpeakerArr::k50, SpeakerArr::k51,
    SpeakerArr::k60Music, SpeakerArr::k61Music, SpeakerArr::k70Cine, SpeakerArr::k70Music,
    SpeakerArr::k71Cine, SpeakerArr::k71Music, SpeakerArr::k71Proximity};

/**
@getNumSupportedChannelFormats
\ingroup VST-Shell

@brief returns number of supported channel I/O combinations
*/
uint32_t getNumSupportedChannelFormats(){ return NUM_SUPPORTED_CHANNEL_FORMATS; }

/**
@getSupportedSpeakerArrangement
\ingroup FX-Functions

@brief implements n-order Lagrange Interpolation

\param index index of channel I/O struct

\return the VST SpeakerArrangement version of ASPiK channel I/O combi
*/
SpeakerArrangement getSupportedSpeakerArrangement(uint32_t index)
{
    if(index >= NUM_SUPPORTED_CHANNEL_FORMATS)
        return speakerArrangements[0];

    return speakerArrangements[index];
}

/**
@getChannelFormatForSpkrArrangement
\ingroup FX-Functions

@brief implements n-order Lagrange Interpolation

\param SpeakerArrangement the VST speaker arrangement

\return the ASPiK channel I/O combination enum
*/
uint32_t getChannelFormatForSpkrArrangement(SpeakerArrangement arr)
{
    switch(arr)
    {
        case SpeakerArr::kEmpty: {
            return kCFNone; }

        case SpeakerArr::kMono: {
            return kCFMono; }

        case SpeakerArr::kStereo: {
            return kCFStereo; }

        case SpeakerArr::k30Cine: {
            return kCFLCR; }

        case SpeakerArr::k31Cine: {
            return kCFLCRS; }

        case SpeakerArr::k40Music: {
            return kCFQuad; }

        case SpeakerArr::k50: {
            return kCF5p0; }

        case SpeakerArr::k51: {
            return kCF5p1; }

        case SpeakerArr::k60Music: {
            return kCF6p0; }

        case SpeakerArr::k61Music: {
            return kCF6p1; }

        case SpeakerArr::k70Cine: {
            return kCF7p0Sony; }

        case SpeakerArr::k70Music: {
            return kCF7p0DTS; }

        case SpeakerArr::k71Cine: {
            return kCF7p1Sony; }

        case SpeakerArr::k71Music: {
            return kCF7p1DTS; }

        case SpeakerArr::k71Proximity: {
            return kCF7p1Proximity; }

        default: {
            return kCFNone; }
    }
}



}
}
}

#endif
