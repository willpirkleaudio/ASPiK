// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_19EEE596C6303C69A1E04DD0515955B7
#define AU_COCOA_VIEW_NAME AUCocoaView_19EEE596C6303C69A1E04DD0515955B7

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.demovolumeplugin.bundleID";
const char* kAUBundleID = "developer.au.demovolumeplugin.bundleID";
const char* kVST3BundleID = "developer.vst3.demovolumeplugin.bundleID";

// --- Plugin Names 
const char* kPluginName = "DemoVolumePlugin";
const char* kShortPluginName = "DemoVolumePlugi";
const char* kAUBundleName = "DemoVolumePlugin";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{19eee596-c630-3c69-a1e0-4dd0515955b7}";

// --- 4-char codes 
const int32_t kFourCharCode = 'dvp1';
const int32_t kAAXProductID = 'dvp1';
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
