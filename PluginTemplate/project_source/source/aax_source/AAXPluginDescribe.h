// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxplugindescribe.h
//
/**
    \file   aaxplugindescribe.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  static description file for the monolithic plugin paradigm; this
    		code is based heavily on the monolithic plugin example code in the
    		SDK.

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "AAX.h"
#include "AAXPluginParameters.h"

/**
 @AAX_CTypeID constants
 \ingroup AAX-Shell

 @brief AAX_CTypeID constants for channel enumerations

 NOTES:
 - currently support one mixed I/O combo: mono-in, stereo-out
 - if you need to add some strange I/O combo, make your own ID and add here
 - these channel fomrats are all selected as the closest matching/overlapping of the same
   variables and values for AU and VST; you can add AAX specific IDs here too
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters

*/
const AAX_CTypeID kPluginCore_PlugInID_Native_Mono        			= 'PCMR'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_MonoIn_StereoOut      = 'PCMS'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_Stereo      			= 'PCST'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_LCR         			= 'PCcR'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_QUAD       		 	= 'PCQa'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_LCRS        			= 'PCRs'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_5_0         			= 'PC50'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_5_1         			= 'PC51'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_6_0         			= 'PC60'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_6_1         			= 'PC61'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_7_0_SDDS    			= 'PC0R'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_7_1_SDDS    			= 'PC1R'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_7_0_DTS     			= 'PC0r'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_7_1_DTS     			= 'PC1r'; ///< AAX_CTypeID channel I/O constant
const AAX_CTypeID kPluginCore_PlugInID_Native_7_1_2       			= 'PC12'; ///< AAX_CTypeID channel I/O constant



class AAX_ICollection;
class AAX_IEffectDescriptor;
class AAX_IComponentDescriptor;

/**
 @DescribeAlgComponent
 \ingroup AAX-Shell

 @brief one of two parts of the static AAX Parameters declaration; this is detailed in the book source below.

*/
void DescribeAlgComponent(AAX_EStemFormat inStemFormat, AAX_IComponentDescriptor& outDesc, AAX_EStemFormat outStemFormat, PluginCore* plugin);

/**
 @GetASPiKPlugInDescription
 \ingroup AAX-Shell

 @brief setup channel I/O algorithms and other core-specific stuff

*/
AAX_Result GetASPiKPlugInDescription(AAX_IEffectDescriptor & outDescriptor, PluginCore* plugin);

/**
 @GetASPiKPlugInDescription
 \ingroup AAX-Shell

 @brief one of two parts of the static AAX Parameters declaration; this is detailed in the book source below.

*/
AAX_Result GetEffectDescriptions(AAX_ICollection * outDescriptions);

/**
 @getAAXStemFormatForChannelFormat
 \ingroup AAX-Shell

 @brief convert an ASPiK channel format enumeration into an AAX_EStemFormat version

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
inline AAX_EStemFormat getAAXStemFormatForChannelFormat(uint32_t format)
{
    switch(format)
    {
        case kCFNone: {
            return AAX_eStemFormat_None; }

        case kCFMono: {
            return AAX_eStemFormat_Mono; }

        case kCFStereo: {
            return AAX_eStemFormat_Stereo; }

        case kCFLCR: {
            return AAX_eStemFormat_LCR; }

        case kCFLCRS: {
            return AAX_eStemFormat_LCRS; }

        case kCFQuad: {
            return AAX_eStemFormat_Quad; }

        case kCF5p0: {
            return AAX_eStemFormat_5_0; }

        case kCF5p1: {
            return AAX_eStemFormat_5_1; }

        case kCF6p0: {
            return AAX_eStemFormat_6_0; }

        case kCF6p1: {
            return AAX_eStemFormat_6_1; }

        case kCF7p0Sony: {
            return AAX_eStemFormat_7_0_SDDS; }

        case kCF7p0DTS: {
            return AAX_eStemFormat_7_0_DTS; }

        case kCF7p1Sony: {
            return AAX_eStemFormat_7_1_SDDS; }

        case kCF7p1DTS: {
            return AAX_eStemFormat_7_1_DTS; }

        case kCF7p1Proximity: {
            return AAX_eStemFormat_7_1_2; }

        default: {
            return AAX_eStemFormat_None; }
    }
    return AAX_eStemFormat_None;
}
