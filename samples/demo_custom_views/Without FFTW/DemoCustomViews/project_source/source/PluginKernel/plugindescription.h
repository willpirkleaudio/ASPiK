// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_50D69A4137F23D25B04E78214F00B5CB
#define AU_COCOA_VIEW_NAME AUCocoaView_50D69A4137F23D25B04E78214F00B5CB

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.democustomviews.bundleID";
const char* kAUBundleID = "developer.au.democustomviews.bundleID";
const char* kVST3BundleID = "developer.vst3.democustomviews.bundleID";

// --- Plugin Names 
const char* kPluginName = "DemoCustomViews";
const char* kShortPluginName = "DemoCustomViews";
const char* kAUBundleName = "DemoCustomViews";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{50d69a41-37f2-3d25-b04e-78214f00b5cb}";

// --- 4-char codes 
const int32_t kFourCharCode = 'dcv2';
const int32_t kAAXProductID = 'dcv2';
const int32_t kManufacturerID = 'ASPK';

// --- Vendor information 
const char* kVendorName = "ASPiK'";
const char* kVendorURL = "www.myplugins.com";
const char* kVendorEmail = "support@myplugins.com";

// --- Plugin Options 
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 1;
const uint32_t kAAXCategory = 2;

#endif
