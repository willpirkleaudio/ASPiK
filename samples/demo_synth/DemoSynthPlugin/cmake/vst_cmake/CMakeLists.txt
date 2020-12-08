	# ---------------------------------------------------------------------------------
#
# --- VST3 CMakeLists.txt
# --- Created by RackAFX(TM), see VST3SDK
# --- www.willpirkle.com
#
# ---------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.4.3)

# --- default project folder location
set(PROJECT_FOLDER "../../")

message(STATUS "------> Starting VST Project Build:")
message(STATUS "        VST SDK Location: ${SDK_ROOT}")
message(STATUS "")

# ---------------------------------------------------------------------------------
#
# ---  Basic Setup
#
# ---------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${SDK_ROOT}/cmake/modules")

# --- SMTG_ prefix added with VSTSDK 3.7.1
include(SMTG_Global)
include(SMTG_AddVST3Library)
include(SMTG_Bundle)
include(SMTG_ExportedSymbols)
include(SMTG_PrefixHeader)
include(SMTG_PlatformIOS)
include(SMTG_PlatformToolset)
include(SMTG_CoreAudioSupport)
include(SMTG_AAXSupport)
include(SMTG_VstGuiSupport)
include(SMTG_UniversalBinary)
include(SMTG_AddVST3Options)

# --- Setup for proper location: this is because we are inside of the SDK to be congruent with all of the other APIs
# setupPlatformToolset()
if(MAC)
	set(SMTG_MAC TRUE)
	if(${SMTG_VST3_TARGET_PATH})
	  file(MAKE_DIRECTORY ${SMTG_VST3_TARGET_PATH})
	  if(EXISTS ${SMTG_VST3_TARGET_PATH})
	      message(STATUS "SMTG_VST3_TARGET_PATH is set to : " ${SMTG_VST3_TARGET_PATH})
	  else()
	      message(STATUS "SMTG_VST3_TARGET_PATH is not set!")
	  endif()
	endif()

else()
	set(SMTG_WIN TRUE)
	set(DEF_OPT_LINK OFF) 		 #--- be sure to start visual with admin right when enabling this
	set(SMTG_CREATE_BUNDLE_FOR_WINDOWS TRUE)
	set(SMTG_CREATE_VST3_LINK OFF) 	 #--- causes too many problems with admin rights and VS (especially on Parallels where it won't work)
	set(SMTG_CREATE_PLUGIN_LINK OFF) #--- causes too many problems with admin rights and VS (especially on Parallels where it won't work)
endif()

# --- default path stuff
smtg_get_default_vst3_path(DEFAULT_VST3_FOLDER)
set(SMTG_VST3_TARGET_PATH "${DEFAULT_VST3_FOLDER}")

# --- use by default SMTG_ as prefix for ASSERT,...
option(SMTG_RENAME_ASSERT "Rename ASSERT to SMTG_ASSERT" ON)

# --- internal variable
set(VST_SDK TRUE)

# ---------------------------------------------------------------------------------
#
# --- Compiler Project
#
# ---------------------------------------------------------------------------------
project(${EXP_SUBPROJECT_NAME_VST})

# --- change from ALL_BUILD as there is only one project
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT ${EXP_SUBPROJECT_NAME_VST})

if (SMTG_RENAME_ASSERT)
	add_compile_options(-DSMTG_RENAME_ASSERT=1)
endif()

if (LINUX)
	# Enable Sample audioHost (based on Jack Audio)
	option(SMTG_ENABLE_USE_OF_JACK "Enable Use of Jack" ON)

    option(SMTG_ADD_ADDRESS_SANITIZER_CONFIG "Add AddressSanitizer Config (Linux only)" OFF)
if(SMTG_ADD_ADDRESS_SANITIZER_CONFIG)
		set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};ASan")
		add_compile_options($<$<CONFIG:ASan>:-DDEVELOPMENT=1>)
		add_compile_options($<$<CONFIG:ASan>:-fsanitize=address>)
		add_compile_options($<$<CONFIG:ASan>:-DVSTGUI_LIVE_EDITING=1>)
		add_compile_options($<$<CONFIG:ASan>:-g>)
		add_compile_options($<$<CONFIG:ASan>:-O0>)
		set(ASAN_LIBRARY asan)
		link_libraries($<$<CONFIG:ASan>:${ASAN_LIBRARY}>)
	endif()
else()
	# Disable Sample audioHost (based on Jack Audio)
	# not yet tested on Windows and Mac
	option(SMTG_ENABLE_USE_OF_JACK "Enable Use of Jack" OFF)
endif()

if(UNIX)
	if(XCODE)
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	elseif(APPLE)
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
		link_libraries(c++)
	else()
		set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar")
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

# --- VST SDK 3.7.1 addition for vstgui_support target
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEVELOPMENT")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

# --- parse sdk subfolders
include_directories(${ROOT} ${SDK_ROOT})

set(SDK_IDE_LIBS_FOLDER FOLDER "Libraries")
set(SDK_IDE_HOSTING_EXAMPLES_FOLDER FOLDER Validator)
#set(SDK_IDE_HOSTING_EXAMPLES_FOLDER FOLDER "HostingExamples")

set(SMTG_ADD_VST3_HOSTING_SAMPLES OFF)

# 3.6.13
add_subdirectory(${SDK_ROOT}/pluginterfaces pluginterfaces)

add_subdirectory(${SDK_ROOT}/base base)
add_subdirectory(${SDK_ROOT}/public.sdk public.sdk)
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst-hosting public.sdk/samples/vst-hosting)

# --- add RackAFX ported project(s)
#
message(STATUS "     -> Adding VST ported project...")
message(STATUS "        VST Target Name: ${EXP_SUBPROJECT_NAME_VST}")
message(STATUS "")

# --- add the project
add_subdirectory(${PROJECT_FOLDER}project_source/${VST_CMAKE_FOLDER} ${PROJECT_FOLDER}project_source/${VST_CMAKE_FOLDER})

#-------------------------------------------------------------------------------
# VSTGUI Support Library
#-------------------------------------------------------------------------------
#option(VSTGUI_STANDALONE "VSTGUI Standalone library" OFF)
set(VSTGUI_STANDALONE OFF)
set(VSTGUI_STANDALONE_EXAMPLES OFF)
set(VSTGUI_DISABLE_UNITTESTS ON)

add_subdirectory(${VSTGUI_ROOT}/vstgui4/vstgui vstgui4/vstgui)

# --- WAS done in AddVST3Library module from Steinberg; as of SDK3.6.14 this seems to be removed
#     but it's cool because ASPiK had been doing this before 
target_compile_definitions(${EXP_SUBPROJECT_NAME_VST} PUBLIC $<$<CONFIG:Debug>:VSTGUI_LIVE_EDITING=1>)

# --- set our compiler flag for VST (individual builds)
# target_compile_definitions(${EXP_SUBPROJECT_NAME_VST} PUBLIC $<$<CONFIG:Debug>:VSTPLUGIN=1>)
# target_compile_definitions(${EXP_SUBPROJECT_NAME_VST} PUBLIC $<$<CONFIG:Release>:VSTPLUGIN=1>)

# --- set our compiler flag for VST (all builds)
target_compile_definitions(${EXP_SUBPROJECT_NAME_VST} PUBLIC VSTPLUGIN=1)

# --- for windows/VS only
add_definitions(-DVSTGUI_DIRECT2D_SUPPORT=1)

set(VST3_VSTGUI_SOURCES
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3groupcontroller.cpp
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3groupcontroller.h
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3padcontroller.cpp
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3padcontroller.h
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.cpp
	${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.h
	${SDK_ROOT}/public.sdk/source/vst/vstguieditor.cpp
)
add_library(vstgui_support STATIC ${VST3_VSTGUI_SOURCES})
target_include_directories(vstgui_support PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_link_libraries(vstgui_support PRIVATE vstgui_uidescription)
if(MAC)
	if(XCODE)
		target_link_libraries(vstgui_support PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework Accelerate" "-framework QuartzCore" "-framework Carbon")
	else()
		find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
		find_library(COCOA_FRAMEWORK Cocoa)
		find_library(OPENGL_FRAMEWORK OpenGL)
		find_library(ACCELERATE_FRAMEWORK Accelerate)
		find_library(QUARTZCORE_FRAMEWORK QuartzCore)
		find_library(CARBON_FRAMEWORK Carbon)
		target_link_libraries(vstgui_support PRIVATE ${COREFOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${ACCELERATE_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} ${CARBON_FRAMEWORK})
	endif()
endif()

#-------------------------------------------------------------------------------
# IDE sorting
#-------------------------------------------------------------------------------
set_target_properties(vstgui_support PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(sdk PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(base PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(vstgui PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(vstgui_uidescription PROPERTIES ${SDK_IDE_LIBS_FOLDER})
#set_target_properties(vstgui_standalone PROPERTIES ${SDK_IDE_LIBS_FOLDER})
