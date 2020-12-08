// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_363A85B567D539FA9B46FF5307B16F80
#define AU_COCOA_VIEW_NAME AUCocoaView_363A85B567D539FA9B46FF5307B16F80

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.plugintemplate.bundleID";
const char* kAUBundleID = "developer.au.plugintemplate.bundleID";
const char* kVST3BundleID = "developer.vst3.plugintemplate.bundleID";

// --- Plugin Names 
const char* kPluginName = "Plugin Template";
const char* kShortPluginName = "Plugin Template";
const char* kAUBundleName = "PluginTemplate_AU";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{363a85b5-67d5-39fa-9b46-ff5307b16f80}";

// --- 4-char codes 
const int32_t kFourCharCode = 'PLUG';
const int32_t kAAXProductID = 'plUg';
const int32_t kManufacturerID = 'COMP';

// --- Vendor information 
const char* kVendorName = "ASPiK User";
const char* kVendorURL = "www.yourcompany.com";
const char* kVendorEmail = "help@yourcompany.com";

// --- Plugin Options 
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0.000000;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 0;
const uint32_t kAAXCategory = 0;

#endif
