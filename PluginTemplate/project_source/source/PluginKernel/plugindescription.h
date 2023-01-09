// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_803DF3590FDE3F2A9E6DEC0FFDBBC3FA
#define AU_COCOA_VIEW_NAME AUCocoaView_803DF3590FDE3F2A9E6DEC0FFDBBC3FA

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.testplutin.bundleID";
const char* kAUBundleID = "developer.au.testplutin.bundleID";
const char* kVST3BundleID = "developer.vst3.testplutin.bundleID";

// --- Plugin Names 
const char* kPluginName = "TEstPlutin";
const char* kShortPluginName = "TEstPlutin";
const char* kAUBundleName = "TEstPlutin_AU";
const char* kAAXBundleName = "TEstPlutin_AAX";
const char* kVSTBundleName = "TEstPlutin_VST";

// --- bundle name helper 
inline static const char* getPluginDescBundleName() 
{ 
#ifdef AUPLUGIN 
	return kAUBundleName; 
#endif 

#ifdef AAXPLUGIN 
	return kAAXBundleName; 
#endif 

#ifdef VSTPLUGIN 
	return kVSTBundleName; 
#endif 

	// --- should never get here 
	return kPluginName; 
} 

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{803df359-0fde-3f2a-9e6d-ec0ffdbbc3fa}";

// --- 4-char codes 
const int32_t kFourCharCode = 'PLUG';
const int32_t kAAXProductID = 'plUg';
const int32_t kManufacturerID = 'COMP';

// --- Vendor information 
const char* kVendorName = "ASPiK User";
const char* kVendorURL = "www.yourcompany.com";
const char* kVendorEmail = "help@yourcompany.com";

// --- Plugin Options 
const bool kProcessFrames = true;
const uint32_t kBlockSize = DEFAULT_AUDIO_BLOCK_SIZE;
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0.000000;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 0;
const uint32_t kAAXCategory = 0;

#endif
