# ---------------------------------------------------------------------------------
#
# --- CMakeLists.txt
# --- ASPiK(TM) Plugin Development Framework
# --- http://www.aspikplugins.com
# --- http://www.willpirkle.com
# --- Author: Will Pirkle
# --- Date: 16 Sept 2018
#
# ---------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.4.3)

# ---------------------------------------------------------------------------------
# --- BEGIN USER INPUT
# ---------------------------------------------------------------------------------

# --- TRUE if merging new 1.1.0 SDK with the old 1.0 SDK
#
#     To Merge with existing AU_SDK folder for previous users:
#	  1. download the SDK from https://github.com/apple/AudioUnitSDK
#	  2. copy the AudioUnitSDK-main/include folder to the AU_SDK folder, keep the name "include"
#	  3. copy the AudioUnitSDK-main/src folder to the AU_SDK folder, keep the name "src"
#
#	  Alternatively, you can download the SDK from https://github.com/apple/AudioUnitSDK
#	  and install it in parallel with your other SDK folders (VST3 and AAX) keeping the
#	  folder named "AudioUnitSDK-main" and set this variable to FALSE
#	  YOU SHOULD ONLY DO THIS IF YOU ARE A SEASONED ASPIK USER
# ---------------------------------------------------------------------------------
set(MERGED_AU_SDKS TRUE) 	# <-- set TRUE or FALSE

# ---------------------------------------------------------------------------------
#
# --- UNIVERSAL SDK FOLDER NAMES
#
# ---------------------------------------------------------------------------------
#
# --- Here you can set the names of your UNIVERSAL SDK sub-USE_FOLDERS
#     NOTE: this is not needed when you export individual API projects
set(UNI_AAX_SDK_FOLDER 		AAX_SDK)
set(UNI_VST3_SDK_FOLDER 	VST_SDK/vst3sdk)

if(MERGED_AU_SDKS)
	set(UNI_AU_SDK_FOLDER 	AU_SDK)
else()
	set(UNI_AU_SDK_FOLDER 	AudioUnitSDK-main)
endif()
# ---------------------------------------------------------------------------------

# ---------------------------------------------------------------------------------
#
# --- BUILD FLAGS - set these to build individual projects or a universal version
#
# ---------------------------------------------------------------------------------

# --- Universal Build Flag - when using combined SDKs; can still build independently
#                            with individual flags below
set(UNIVERSAL_SDK_BUILD TRUE) 	# <-- set TRUE or FALSE

# --- Individual project builds
set(AAX_SDK_BUILD TRUE)# <-- set TRUE or FALSE
set(AU_SDK_BUILD TRUE)# <-- set TRUE or FALSE
set(VST_SDK_BUILD TRUE)# <-- set TRUE or FALSE

# ---------------------------------------------------------------------------------
#
# --- PROJECT NAMES/FLAGS/CODES
#
# ---------------------------------------------------------------------------------
# --- the name of the project must be valid CMAKE name: must not contain spaces or numbers
set(PLUGIN_PROJECT_NAME DemoCustomViews)	# <-- set PROJECT NAME with or without quotes

# --- the name of the PLUGIN as you want it to appear in DAW menus for the user
set(PLUGIN_NAME "DemoCustomViews")	# <-- set PLUGIN NAME with quotes; ok to have spaces or numbers if you like

# --- Synth Flag: FALSE = FX plugin, TRUE = Synth Plugin
set(IS_SYNTH_PLUGIN FALSE)		# <-- set Synth Flag to TRUE only for an output-only (synth) plugin

# --- AAX Sidechain Flag:
set(WANT_AAX_SIDECHAIN FALSE)	# <-- for AAX Sidechaining Only

# --- AAX ProTools Gain Reduction Meter Flag:
set(HAS_AAX_GR_METER FALSE)		# <-- for special Pro-Tools AAX Gain Reduction Meter Only

# --- VENDOR information (your company stuff)
set(VENDOR_NAME "ASPiK'")			# <-- required!
set(VENDOR_URL "www.myplugins.com")			# <-- OPTIONAL (ok if blank)
set(VENDOR_EMAIL "support@myplugins.com")		# <-- OPTIONAL (ok if blank)

# --- 4-Character Codes (for AAX and VST3)
set(4CHAR_CODE "dcv2")				# MUST be unique for each plugin your company creates/sells
set(4CHAR_MANUFACTURER_ID "ASPK")			# your company CODE (does NOT need to be unique for each plugin; can be re-used)
set(4CHAR_AAX_PRODUCT_ID "dcv2")			# MUST be unique for each plugin your company creates/sells

# --- Plugin Options ---
set(INCLUDE_FX_OBJECTS FALSE)		# <-- set TRUE or FALSE
set(LINK_FFTW  FALSE)						# <-- set TRUE or FALSE
set(EXPOSE_SIDECHAIN FALSE) 		# <-- set TRUE or FALSE
set(LATENCY_IN_SAMPLES 0) 			# <-- numerical, in samples
set(TAIL_TIME_MSEC 0)				# <-- numerical, in mSec
set(PROCESS_FRAMES TRUE)		# <-- set TRUE or FALSE
set(PROCESS_BUFFERS FALSE)		# <-- set TRUE or FALSE
set(PROCESS_BLOCKS FALSE)		# <-- set TRUE or FALSE
set(BLOCK_SIZE 64) # <-- numerical, in samples (per channel) if using blocks

# --- VST3 Only ---
set(VST3_INFINITE_TAIL FALSE)
set(VST3_SAMPLE_ACCURATE_AUTOMATION FALSE)
set(VST3_SAMPLE_ACCURATE_GRANULARITY 1)

# --- AAX Only ---
set(AAX_CATEGORY 2)

# ---------------------------------------------------------------------------------
#
# --- OPTIONAL SUB-PROJECT NAMES
#     - change only if you need to; does NOT need to be changed with each project
#
# ---------------------------------------------------------------------------------
# --- this creates sub-project names for universal projects
if(UNIVERSAL_SDK_BUILD)
	# --- names of sub-projects for universal builds; you can change them here:
	set(EXP_PROJECT_NAME ${PLUGIN_PROJECT_NAME}_UNIVERSAL)
	set(EXP_SUBPROJECT_NAME_AAX ${PLUGIN_PROJECT_NAME}_AAX)
	set(EXP_SUBPROJECT_NAME_AU ${PLUGIN_PROJECT_NAME}_AU)
	set(EXP_SUBPROJECT_NAME_VST ${PLUGIN_PROJECT_NAME}_VST)
else()
	# --- names of sub-projects for non-universal builds are same as normal builds; you can change them here:
	set(EXP_PROJECT_NAME ${PLUGIN_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_AAX ${PLUGIN_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_AU ${PLUGIN_PROJECT_NAME})
	set(EXP_SUBPROJECT_NAME_VST ${PLUGIN_PROJECT_NAME})
endif()

# ---------------------------------------------------------------------------------
#
# --- SDK ROOTS (OPTIONAL - only change these if you know what you are doing!)
#
# ---------------------------------------------------------------------------------
# --- PARENT: this is the UNIVERSAL project folder
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# --- GRANDPARENT: this is the outer "myprojects" folder
get_filename_component(GRANDPARENT_DIR ${PARENT_DIR} DIRECTORY)

# --- GR_GRANDPARENT: this is the outer "ALL_SDK" folder, or whatever the user calls it
get_filename_component(GR_GRANDPARENT_DIR ${GRANDPARENT_DIR} DIRECTORY)

# ---------------------------------------------------------------------------------
#
#	  NOTE: the SDK path defaults to the location included in the "readme.txt" file
#			in the Project folder. However, you can customize it here
#			by either replacing the code with your own CMake script, or by
#			setting the ROOT variable to an absolute path, e.g.
#			MacOS:
#				set(ROOT "/Users/<your user name>/Documents/AU_SDK")
#
#			Windows:
#				set(ROOT "C:/AU_SDK")
#
# ---------------------------------------------------------------------------------
if(UNIVERSAL_SDK_BUILD)
	# --- AAX:
	set(AAX_SDK_ROOT ${GRANDPARENT_DIR}/${UNI_AAX_SDK_FOLDER})

	# --- AU:
	set(AU_SDK_ROOT ${GRANDPARENT_DIR}/${UNI_AU_SDK_FOLDER})

	# --- VST3:
	set(SDK_ROOT ${GRANDPARENT_DIR}/${UNI_VST3_SDK_FOLDER})
else()
	# --- AAX:
	set(AAX_SDK_ROOT ${GRANDPARENT_DIR})

	# --- AU:
	set(AU_SDK_ROOT ${GRANDPARENT_DIR})

	# --- VST3:
	set(SDK_ROOT ${GRANDPARENT_DIR})

endif()

# --- new in 3.6.11/12 (VST ONLY)
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)

# ---------------------------------------------------------------------------------
# --- END USER INPUT (you don't need to modify anything past this point!)
# ---------------------------------------------------------------------------------
set(WINBUILD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/win_build)
set(MACBUILD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/mac_build)

# --- VSTGUI4: NOTE: we use our own VSTGUI4 folder - portable to new SDKs where you
#              may not want to update VSTGUI just yet and won't interfere
#		       		 with VST3 SDK whatsoever. You do NOT need to customize this path!
set(VSTGUI_ROOT ${PARENT_DIR})

# ---------------------------------------------------------------------------------
#
# ---  Basic Setup
#
# ---------------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR OR EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
	message(SEND_ERROR "In-source builds are not allowed. Please create a separate build directory and run 'cmake /path/to/folder [options]' there.")
	message(FATAL_ERROR "You can remove the file \"CMakeCache.txt\" and directory \"CMakeFiles\" in ${CMAKE_SOURCE_DIR}.")
endif()

if(POLICY CMP0054)
	cmake_policy(SET CMP0054 NEW)
endif()

if(APPLE)
  set(MAC TRUE)
elseif(WIN32)
	set(WIN TRUE)
elseif(UNIX)
	set(LINUX TRUE)
endif()

# ---------------------------------------------------------------------------------
#
# --- Setup Compiler Project
#
# ---------------------------------------------------------------------------------
project(${EXP_PROJECT_NAME})

if(UNIX)
	if(XCODE)
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14") # --- NOTE: VST3 requires c++14!
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	elseif(APPLE)
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
		link_libraries(c++)
	else()
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-multichar")
		link_libraries(stdc++fs pthread dl)
	endif()
elseif(WIN)
	# add_definitions(-D_UNICODE)
	add_compile_options(/fp:fast)
	add_compile_options($<$<CONFIG:Release>:/Oi>)	# Enable Intrinsic Functions (Yes)
	add_compile_options($<$<CONFIG:Release>:/Ot>)	# Favor Size Or Speed (Favor fast code)
	add_compile_options($<$<CONFIG:Release>:/GF>)	# Enable String Pooling
	add_compile_options($<$<CONFIG:Release>:/EHa>)	# Enable C++ Exceptions
	add_compile_options($<$<CONFIG:Release>:/Oy>)	# Omit Frame Pointers
	#add_compile_options($<$<CONFIG:Release>:/Ox>)	# Optimization (/O2: Maximise Speed /0x: Full Optimization)
endif()


# --- add various API projects
set(AAX_CMAKE_FOLDER cmake/aax_cmake)
set(AU_CMAKE_FOLDER cmake/au_cmake)
set(VST_CMAKE_FOLDER cmake/vst_cmake)

# ---------------------------------------------------------------------------------
#
# --- BUNDLE IDs and other variables for <plugindescription.h>
#
# ---------------------------------------------------------------------------------
string(REPLACE "_" "" CLEAN_PROJECT_NAME ${PLUGIN_PROJECT_NAME})
string(TOLOWER ${CLEAN_PROJECT_NAME} CLEAN_PROJECT_NAME_LC)

# --- BUNDLE ID Values: one for each API
string(CONCAT AAX_BUNDLE_ID "developer.aax." ${CLEAN_PROJECT_NAME_LC} ".bundleID")
string(CONCAT AU_BUNDLE_ID "developer.au." ${CLEAN_PROJECT_NAME_LC} ".bundleID")
string(CONCAT VST_BUNDLE_ID "developer.vst3." ${CLEAN_PROJECT_NAME_LC} ".bundleID")

string(CONCAT AAX_BUNDLE_ID_ASVAR "const char* kAAXBundleID = \"" ${AAX_BUNDLE_ID} "\"")
string(CONCAT AU_BUNDLE_ID_ASVAR "const char* kAUBundleID = \"" ${AU_BUNDLE_ID} "\"")
string(CONCAT VST_BUNDLE_ID_ASVAR "const char* kVST3BundleID = \"" ${VST_BUNDLE_ID} "\"")

# --- added v1.6.4 for correct bundle naming of Cocoa component in AU universal builds
if(UNIVERSAL_SDK_BUILD)
	string(CONCAT AU_BUNDLE_NAME_ASVAR_P1 "const char* kAUBundleName = \"" ${PLUGIN_PROJECT_NAME})
	string(CONCAT AU_BUNDLE_NAME_ASVAR ${AU_BUNDLE_NAME_ASVAR_P1} "_AU\"")
	
	string(CONCAT AAX_BUNDLE_NAME_ASVAR_P1 "const char* kAAXBundleName = \"" ${PLUGIN_PROJECT_NAME})
	string(CONCAT AAX_BUNDLE_NAME_ASVAR ${AAX_BUNDLE_NAME_ASVAR_P1} "_AAX\"")
	
	string(CONCAT VST_BUNDLE_NAME_ASVAR_P1 "const char* kVSTBundleName = \"" ${PLUGIN_PROJECT_NAME})
	string(CONCAT VST_BUNDLE_NAME_ASVAR ${VST_BUNDLE_NAME_ASVAR_P1} "_VST\"")
else()
	string(CONCAT AU_BUNDLE_NAME_ASVAR "const char* kAUBundleName = \"" ${PLUGIN_PROJECT_NAME} "\"")
	string(CONCAT AAX_BUNDLE_NAME_ASVAR "const char* kAAXBundleName = \"" ${PLUGIN_PROJECT_NAME} "\"")
	string(CONCAT VST_BUNDLE_NAME_ASVAR "const char* kVSTBundleName = \"" ${PLUGIN_PROJECT_NAME} "\"")
endif()

# --- short AAX plugin name
string(SUBSTRING ${PLUGIN_NAME} 0 15 PLUGIN_NAME_SHORT)

string(CONCAT PLUGIN_NAME_ASVAR "const char* kPluginName = \"" ${PLUGIN_NAME} "\"")
string(CONCAT SHORT_PLUGIN_NAME_ASVAR "const char* kShortPluginName = \"" ${PLUGIN_NAME_SHORT} "\"")

# --- GUID for VST3 and for creating unique name for Cocoa GUI (AU only)
set(UUID_SEED 6ba7b810-9dad-11d1-80b4-00c04fd430c8)
string(TIMESTAMP CONFIG_DATE)
string(RANDOM a)
string(CONCAT UUID_DIRT ${CONFIG_DATE} ${a})
string(
    UUID NEW_VST_UUID
    NAMESPACE ${UUID_SEED}
    NAME ${UUID_DIRT}
    TYPE MD5
)

string(REPLACE "-" "" CLEAN_UUID ${NEW_VST_UUID})
string(TOUPPER ${CLEAN_UUID} CLEAN_UUID_UC)

string(CONCAT AU_COCOA_VIEWFACTORY_NAME "AUCocoaViewFactory_" ${CLEAN_UUID_UC})
string(CONCAT AU_COCOA_VIEWFACTORY_DEFINE "#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_" ${CLEAN_UUID_UC})
string(CONCAT AU_COCOA_VIEW_DEFINE "#define AU_COCOA_VIEW_NAME AUCocoaView_" ${CLEAN_UUID_UC})

string(CONCAT VST3_UUID_ASVAR "const char* kVSTFUID = \"{" ${NEW_VST_UUID} "}\"")

if(IS_SYNTH_PLUGIN)
	set(PLUGIN_TYPE_ASVAR "const pluginType kPluginType = pluginType::kSynthPlugin")
	set(INFOPLIST_AU_TYPE_CODE "aumu")
else()
	set(PLUGIN_TYPE_ASVAR "const pluginType kPluginType = pluginType::kFXPlugin")
	set(INFOPLIST_AU_TYPE_CODE "aufx") # --- NOTE: using aufx here rather than aumf, even though we use the aumf base to expose sidechain channels (aumf does not expose sidechain!)
endif()

string(CONCAT VENDOR_NAME_ASVAR "const char* kVendorName = \"" ${VENDOR_NAME} "\"")
string(CONCAT VENDOR_URL_ASVAR "const char* kVendorURL = \"" ${VENDOR_URL} "\"")
string(CONCAT VENDOR_EMAIL_ASVAR "const char* kVendorEmail = \"" ${VENDOR_EMAIL} "\"")

string(SUBSTRING ${4CHAR_CODE} 0 4 4CHAR_CODE_PROPER)
string(SUBSTRING ${4CHAR_AAX_PRODUCT_ID} 0 4 4CHAR_AAX_PRODUCT_ID_PROPER)
string(SUBSTRING ${4CHAR_MANUFACTURER_ID} 0 4 4CHAR_AAX_MANUFACTURER_ID_PROPER)
string(CONCAT INFOPLIST_PI_VENDOR_COLON_NAME_CODE ${VENDOR_NAME} ": " ${PLUGIN_NAME})

string(CONCAT 4CHAR_CODE_ASVAR "const int32_t kFourCharCode = \'" ${4CHAR_CODE_PROPER} "\'")
string(CONCAT 4CHAR_AAX_PRODUCT_ID_ASVAR "const int32_t kAAXProductID = \'" ${4CHAR_AAX_PRODUCT_ID_PROPER} "\'")
string(CONCAT 4CHAR_AAX_MANUFACTURER_ID_ASVAR "const int32_t kManufacturerID = \'" ${4CHAR_AAX_MANUFACTURER_ID_PROPER} "\'")

# --- PLUGIN OPTIONS
if(EXPOSE_SIDECHAIN)
	set(EXPOSE_SIDECHAIN_ASVAR "const bool kWantSidechain = true")
else()
	set(EXPOSE_SIDECHAIN_ASVAR "const bool kWantSidechain = false")
endif()

string(CONCAT LATENCY_IN_SAMPLES_ASVAR "const uint32_t kLatencyInSamples = " ${LATENCY_IN_SAMPLES})
string(CONCAT TAIL_TIME_MSEC_ASVAR "const double kTailTimeMsec = " ${TAIL_TIME_MSEC})

if(VST3_INFINITE_TAIL)
	set(VST3_INFINITE_TAIL_ASVAR "const bool kVSTInfiniteTail = true")
else()
	set(VST3_INFINITE_TAIL_ASVAR "const bool kVSTInfiniteTail = false")
endif()

if(VST3_SAMPLE_ACCURATE_AUTOMATION)
	set(VST3_SAMPLE_ACCURATE_AUTOMATION_ASVAR "const bool kVSTSAA = true")
else()
	set(VST3_SAMPLE_ACCURATE_AUTOMATION_ASVAR "const bool kVSTSAA = false")
endif()

string(CONCAT VST3_SAMPLE_ACCURATE_GRANULARITY_ASVAR "const uint32_t kVST3SAAGranularity = " ${VST3_SAMPLE_ACCURATE_GRANULARITY})
string(CONCAT AAX_CAT_ASVAR "const uint32_t kAAXCategory = " ${AAX_CATEGORY})


if(PROCESS_FRAMES)
	set(PROCESS_FRAMES_ASVAR "const bool kProcessFrames = true")
	set(BLOCK_SIZE_ASVAR "const uint32_t kBlockSize = DEFAULT_AUDIO_BLOCK_SIZE")
else()
	set(PROCESS_FRAMES_ASVAR "const bool kProcessFrames = false")
	set(BLOCK_SIZE_ASVAR "const uint32_t kBlockSize = DEFAULT_AUDIO_BLOCK_SIZE")
endif()

if(PROCESS_BUFFERS)
	set(BLOCK_SIZE_ASVAR "const uint32_t kBlockSize = 0")
endif()

if(PROCESS_BLOCKS)
	string(CONCAT BLOCK_SIZE_ASVAR "const uint32_t kBlockSize = " ${BLOCK_SIZE})
endif()


# --- the plugindescription.h file - this is edited to contain your string settings for the project!
set(PI_DESCRIPTION_H_FILE project_source/source/PluginKernel/plugindescription.h)
file(WRITE ${PI_DESCRIPTION_H_FILE} "")

# --- header part
file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- CMAKE generated variables for your plugin\n\n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#include \"pluginstructures.h\"\n\n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#ifndef _plugindescription_h\n#define _plugindescription_h\n\n")

file(APPEND ${PI_DESCRIPTION_H_FILE} "#define QUOTE(name) #name\n#define STR(macro) QUOTE(macro)\n#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)\n#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)\n\n")

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- AU Plugin Cocoa View Names (flat namespace) \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AU_COCOA_VIEWFACTORY_DEFINE}\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AU_COCOA_VIEW_DEFINE}\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- BUNDLE IDs (MacOS Only) \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AAX_BUNDLE_ID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AU_BUNDLE_ID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST_BUNDLE_ID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- Plugin Names \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${PLUGIN_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${SHORT_PLUGIN_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AU_BUNDLE_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AAX_BUNDLE_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST_BUNDLE_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- bundle name helper \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "inline static const char* getPluginDescBundleName() \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "{ \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#ifdef AUPLUGIN \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "\treturn kAUBundleName; \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#endif \n\n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#ifdef AAXPLUGIN \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "\treturn kAAXBundleName; \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#endif \n\n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#ifdef VSTPLUGIN \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "\treturn kVSTBundleName; \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "#endif \n\n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "\t// --- should never get here \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "\treturn kPluginName; \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} "} \n\n")

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- Plugin Type \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${PLUGIN_TYPE_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- VST3 UUID \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST3_UUID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- 4-char codes \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${4CHAR_CODE_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${4CHAR_AAX_PRODUCT_ID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${4CHAR_AAX_MANUFACTURER_ID_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- Vendor information \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VENDOR_NAME_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VENDOR_URL_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VENDOR_EMAIL_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

file(APPEND ${PI_DESCRIPTION_H_FILE} "// --- Plugin Options \n")
file(APPEND ${PI_DESCRIPTION_H_FILE} ${PROCESS_FRAMES_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${BLOCK_SIZE_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${EXPOSE_SIDECHAIN_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${LATENCY_IN_SAMPLES_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${TAIL_TIME_MSEC_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST3_INFINITE_TAIL_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST3_SAMPLE_ACCURATE_AUTOMATION_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${VST3_SAMPLE_ACCURATE_GRANULARITY_ASVAR}\;\n)
file(APPEND ${PI_DESCRIPTION_H_FILE} ${AAX_CAT_ASVAR}\;\n)

file(APPEND ${PI_DESCRIPTION_H_FILE} \n)

# --- the EOF
file(APPEND ${PI_DESCRIPTION_H_FILE} "#endif\n")
message(STATUS "--- Updated File: " ${PI_DESCRIPTION_H_FILE})
message(STATUS "")


# --- you can rename the plugin output folders; VST is set to VST3 (see below)
set(AAX_PLUGIN_FOLDER AAX)
set(AU_PLUGIN_FOLDER AU)
# --- VST plugin folder is set to "VST3" inside of one of the vst cmake modules, which we don't want to mess with
#     set(VST_PLUGIN_FOLDER VST3)

# --- add each project:
if(AAX_SDK_BUILD)
	add_subdirectory(${AAX_CMAKE_FOLDER})
endif()

if(MAC AND AU_SDK_BUILD)
	add_subdirectory(${AU_CMAKE_FOLDER})
endif()

if(VST_SDK_BUILD)
	add_subdirectory(${VST_CMAKE_FOLDER})
endif()
