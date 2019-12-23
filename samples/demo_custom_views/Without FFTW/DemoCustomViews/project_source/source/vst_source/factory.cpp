// -----------------------------------------------------------------------------
//    ASPiK VST Shell File:  factory.cpp
//
/**
    \file   factory.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  class factory definition file; based on numerous examples in VST SDK

			- included in ASPiK(TM) VST Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and a
    		  VST Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "public.sdk/source/main/pluginfactory.h"
#include "vst3plugin.h"
/**
@DEF_CLASS2
\ingroup VST-Shell

@brief VST macro for defining a VST plugin.

- see VST3 SDK Documentation for more details
*/
BEGIN_FACTORY_DEF (Steinberg::Vst::ASPiK::VST3Plugin::getVendorName(),
				   Steinberg::Vst::ASPiK::VST3Plugin::getVendorURL(),
				   Steinberg::Vst::ASPiK::VST3Plugin::getVendorEmail())

	DEF_CLASS2 (INLINE_UID_FROM_FUID((*(Steinberg::Vst::ASPiK::VST3Plugin::getFUID()))),
				PClassInfo::kManyInstances,                             // cardinality
				kVstAudioEffectClass,                                   // the component category
				Steinberg::Vst::ASPiK::VST3Plugin::getPluginName(),		// Plug-in name
				0,                                                      // single component effects can not be destributed across CPUs so this is zero
                Steinberg::Vst::ASPiK::VST3Plugin::getPluginType(),		// Subcategory for this Plug-in
				"1.0.0.001",                                            // Plug-in version
				kVstVersionString,                                      // the VST 3 SDK version
				Steinberg::Vst::ASPiK::VST3Plugin::createInstance)		// function pointer called when this component should be instanciated
END_FACTORY

/**
@InitModule
\ingroup VST-Shell

@brief VST function called after module is loaded

- see VST3 SDK Documentation for more details
- not used in ASPiK
*/
bool InitModule(){
	return true;
}

/**
@DeinitModule
\ingroup VST-Shell

@brief VST function called before module is un-loaded

- see VST3 SDK Documentation for more details
- not used in ASPiK
*/
bool DeinitModule(){
	return true;
}



