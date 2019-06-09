// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_2847825C431C36EE82F8848C2E6123DF
#define AU_COCOA_VIEW_NAME AUCocoaView_2847825C431C36EE82F8848C2E6123DF

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.demofftviews.bundleID";
const char* kAUBundleID = "developer.au.demofftviews.bundleID";
const char* kVST3BundleID = "developer.vst3.demofftviews.bundleID";

// --- Plugin Names 
const char* kPluginName = "DemoFFTViews";
const char* kShortPluginName = "DemoFFTViews";
const char* kAUBundleName = "DemoFFTViews";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{2847825c-431c-36ee-82f8-848c2e6123df}";

// --- 4-char codes 
const int32_t kFourCharCode = 'dfv2';
const int32_t kAAXProductID = 'dfv2';
const int32_t kManufacturerID = 'ASPK';

// --- Vendor information 
const char* kVendorName = "ASPiK";
const char* kVendorURL = "www.aspikplugins.com";
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
