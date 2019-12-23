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
set(SDK_ROOT ${AAX_SDK_ROOT})

set(WIN_ROOT "../../win/aax")
set(MAC_ROOT "../../mac/aax")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")

# --- local roots
set(KERNEL_SOURCE_ROOT "${SOURCE_ROOT}/PluginKernel")
set(OBJECTS_SOURCE_ROOT "${SOURCE_ROOT}/PluginObjects")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/CustomControls")
set(AAX_SOURCE_ROOT "${SOURCE_ROOT}/aax_source")
set(FFTW_SOURCE_ROOT "${SOURCE_ROOT}/FFTW")

# ---------------------------------------------------------------------------------
#
# ---  KERNEL plugin files
#
# ---------------------------------------------------------------------------------
set(kernel_sources
	${KERNEL_SOURCE_ROOT}/guiconstants.h
	${KERNEL_SOURCE_ROOT}/pluginbase.h
	${KERNEL_SOURCE_ROOT}/plugincore.h
	${KERNEL_SOURCE_ROOT}/plugindescription.h
	${KERNEL_SOURCE_ROOT}/plugingui.h
	${KERNEL_SOURCE_ROOT}/pluginparameter.h
	${KERNEL_SOURCE_ROOT}/pluginstructures.h
	${KERNEL_SOURCE_ROOT}/pluginbase.cpp
	${KERNEL_SOURCE_ROOT}/plugincore.cpp
	${KERNEL_SOURCE_ROOT}/plugingui.cpp
	${KERNEL_SOURCE_ROOT}/pluginparameter.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  Plugin Helper Object files
#
# ---------------------------------------------------------------------------------
set(plugin_object_sources
	${OBJECTS_SOURCE_ROOT}/fxobjects.h
	${OBJECTS_SOURCE_ROOT}/filters.h
	${OBJECTS_SOURCE_ROOT}/fxobjects.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  CustomControl files
#
# ---------------------------------------------------------------------------------
set(custom_vstgui_sources
	${VSTGUI_SOURCE_ROOT}/atomicops.h
	${VSTGUI_SOURCE_ROOT}/customcontrols.h
	${VSTGUI_SOURCE_ROOT}/readerwriterqueue.h
	${VSTGUI_SOURCE_ROOT}/customviews.h
	${VSTGUI_SOURCE_ROOT}/customcontrols.cpp
	${VSTGUI_SOURCE_ROOT}/customviews.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  AAX Core files
#
# ---------------------------------------------------------------------------------
set(aax_core_sources
	${AAX_SOURCE_ROOT}/AAXPluginDescribe.h
	${AAX_SOURCE_ROOT}/AAXPluginDescribe.cpp
	${AAX_SOURCE_ROOT}/AAXPluginGUI.h
	${AAX_SOURCE_ROOT}/AAXPluginGUI.cpp
	${AAX_SOURCE_ROOT}/AAXPluginParameters.h
	${AAX_SOURCE_ROOT}/AAXPluginParameters.cpp
	${AAX_SOURCE_ROOT}/AAXtoVSTGUIButtonState.h
	${AAX_SOURCE_ROOT}/AntiLogTaperDelegate.h
	${AAX_SOURCE_ROOT}/LogTaperDelegate.h
	${AAX_SOURCE_ROOT}/VoltOctaveTaperDelegate.h
	${AAX_SOURCE_ROOT}/channelformats.h
)

# ---------------------------------------------------------------------------------
#
# ---  AAX SDK files
#
# ---------------------------------------------------------------------------------
set(aax_lib_sources_mac
	${AAX_SDK_ROOT}/Interfaces/AAX_Exports.cpp
	${AAX_SDK_ROOT}/Libs/Debug/libAAXLibrary_libcpp.a
)

set(aax_lib_sources_win
	${AAX_SDK_ROOT}/Interfaces/AAX_Exports.cpp
	${AAX_SDK_ROOT}/Libs/Debug/AAXLibrary_x64_D.lib
	${AAX_SDK_ROOT}/Libs/Release/AAXLibrary_x64.lib
)

# ---------------------------------------------------------------------------------
#
# ---  VSTGUI files
#
# ---------------------------------------------------------------------------------
set(vstgui_sources_mac
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_mac.mm
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_mac.mm
)

set(vstgui_sources_win
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_win32.cpp
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_win32.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  setup IDE folders/filters
#
# ---------------------------------------------------------------------------------
source_group(PluginKernel FILES ${kernel_sources})
source_group(PluginObjects FILES ${plugin_object_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
source_group(AAXCore FILES ${aax_core_sources})
if(MAC)
	source_group(AvidLibraries FILES ${aax_lib_sources_mac})
	source_group(vstgui4 FILES ${vstgui_sources_mac})
elseif(WIN)
	source_group(AvidLibraries FILES ${aax_lib_sources_win})
	source_group(vstgui4 FILES ${vstgui_sources_win})
endif()


# ---------------------------------------------------------------------------------
#
# ---  AAX target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_AAX})

# --- note this is placed *before* add_library( ) on purpose!
if(LINK_FFTW)
	if(MAC)
		link_directories(/opt/local/lib)
	endif()
endif()

# ---  add the plugin source
if(MAC)
	if(INCLUDE_FX_OBJECTS)
		add_library(${target} MODULE ${kernel_sources} ${plugin_object_sources} ${aax_core_sources} ${aax_lib_sources_mac} ${vstgui_sources_mac} ${custom_vstgui_sources})
	else()
		add_library(${target} MODULE ${kernel_sources} ${aax_core_sources} ${aax_lib_sources_mac} ${vstgui_sources_mac} ${custom_vstgui_sources})
	endif()
elseif(WIN)
	if(INCLUDE_FX_OBJECTS)
		add_library(${target} MODULE ${kernel_sources} ${plugin_object_sources} ${aax_core_sources} ${aax_lib_sources_win} ${vstgui_sources_win} ${custom_vstgui_sources})
	else()
		add_library(${target} MODULE ${kernel_sources} ${aax_core_sources} ${aax_lib_sources_win} ${vstgui_sources_win} ${custom_vstgui_sources})
	endif()
endif()

# --- link to the AAX Library: NOTE: user must compile this first - there are no CMake files with AAX SDK!
if(MAC)
	add_library(aaxLibraryD STATIC IMPORTED)
	set_property(TARGET aaxLibraryD PROPERTY IMPORTED_LOCATION "${AAX_SDK_ROOT}/Libs/Debug/libAAXLibrary_libcpp.a")
	target_link_libraries(${target} PRIVATE debug aaxLibraryD)

	add_library(aaxLibraryR STATIC IMPORTED)
	set_property(TARGET aaxLibraryR PROPERTY IMPORTED_LOCATION ${AAX_SDK_ROOT}/Libs/Release/libAAXLibrary_libcpp.a)
	target_link_libraries(${target} PRIVATE optimized aaxLibraryR)
elseif(WIN)
	add_library(aaxLibraryD STATIC IMPORTED)
	set_property(TARGET aaxLibraryD PROPERTY IMPORTED_LOCATION "${AAX_SDK_ROOT}/Libs/Debug/AAXLibrary_x64_D.lib")
	target_link_libraries(${target} PRIVATE debug aaxLibraryD)

	add_library(aaxLibraryR STATIC IMPORTED)
	set_property(TARGET aaxLibraryR PROPERTY IMPORTED_LOCATION ${AAX_SDK_ROOT}/Libs/Release/AAXLibrary_x64.lib )
	target_link_libraries(${target} PRIVATE optimized aaxLibraryR)
endif()

# --- link to FFTW (optional)
if(LINK_FFTW)
	if(WIN)
  	add_definitions(-DHAVE_FFTW=1)
		message(STATUS "---> LINK FFTW Found. + Adding x64/libfftw3-3.lib to VS Project")

		# --- set the FFTW include directory
		target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${FFTW_SOURCE_ROOT})

		# --- add the fftw lib to the dependent lib list
		target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${FFTW_SOURCE_ROOT}/x64/libfftw3-3.lib)
	else()
		message(STATUS "---> LINK FFTW Found: + Adding /opt/local/include to the header search path.")
		message(STATUS "                      + Adding /opt/local/lib to the library search path.")

		# --- set the FFTW include directory
		target_include_directories(${target} PUBLIC "/opt/local/include")

		# --- add the fftw lib to the dependent lib list
		message(STATUS "                      + Adding /opt/local/lib/libfftw3.a to the link library list.")
		target_link_libraries(${target} PRIVATE libfftw3.a)

		set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: HAVE_FFTW=1>$<$<CONFIG:Release>: HAVE_FFTW=1>")
		message(STATUS "                      + Adding HAVE_FFTW to the pre-processor definitions.")

	endif()
endif()

# --- output folder for the plugin
if(MAC)
	set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER})
endif()

# --- setup the project
if(MAC)
	set_target_properties(${target} PROPERTIES BUNDLE TRUE)
	set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${target} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
  	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "aaxplugin")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")

	target_link_libraries(${target} PRIVATE "-framework Accelerate")
	target_link_libraries(${target} PRIVATE "-framework OpenGL")
	target_link_libraries(${target} PRIVATE "-framework CoreMIDI")
	target_link_libraries(${target} PRIVATE "-framework CoreServices")
	target_link_libraries(${target} PRIVATE "-framework Carbon")
	target_link_libraries(${target} PRIVATE "-framework Cocoa")
	target_link_libraries(${target} PRIVATE "-framework Quartz")
	target_link_libraries(${target} PRIVATE "-framework CoreFoundation")

	# --- remove -all_load from other flags
	set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_OTHER_LDFLAGS "")

	# --- eureka
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_DYLIB_CURRENT_VERSION "1.0")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_DYLIB_COMPATIBILITY_VERSION "1.0")

	# --- NOTE: you can change this to suit your company, year, etc...
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_CURRENT_YEAR "2020")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_AVID_COMPANY_STRING "YourCompany.")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_AVID_COPYRIGHT "2020_YourCompany")

	# --- set individual properties!
	set(CMAKE_XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL "$<$<CONFIG:Debug>:0>$<$<CONFIG:Release>:3>")
	set(CMAKE_XCODE_ATTRIBUTE_COPY_PHASE_STRIP "$<$<CONFIG:Debug>:NO>$<$<CONFIG:Release>:YES>")

	# --- APPEND preprocessor strings PER CONFIGURATION
	set_property(TARGET ${target} APPEND PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>:_DEBUG=1>$<$<CONFIG:Release>:_NDEBUG=1>")

	# --- note: " " prior to AAXPLUGIN etc... must be there to separate vars
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: VSTGUI_LIVE_EDITING=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AAXPLUGIN=1>$<$<CONFIG:Release>: AAXPLUGIN=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")

	if(WANT_AAX_SIDECHAIN)
		set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: WANT_SIDECHAIN=1>$<$<CONFIG:Release>: WANT_SIDECHAIN=1>")
	endif()

	if(HAS_AAX_GR_METER)
		set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: PT_GR_METER=1>$<$<CONFIG:Release>: PT_GR_METER=1>")
	endif()

	# --- other linker flags here:
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_OTHER_LDFLAGS "$<$<CONFIG:Debug>: >$<$<CONFIG:Release>:-Xlinker -headerpad -Xlinker 1500>")
elseif(WIN)
	add_definitions(-DAAXPLUGIN)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
	add_definitions(-DWINDOWS_VERSION)

	# --- for VSTGUI: ASPiK_SDK 1.5.1
	add_definitions(-DVSTGUI_DIRECT2D_SUPPORT=1)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEVELOPMENT")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")
	# ------------------------------------------------------

	if(WANT_AAX_SIDECHAIN)
	  add_definitions(-DWANT_SIDECHAIN)
	endif()

	if(HAS_AAX_GR_METER)
		add_definitions(-DPT_GR_METER)
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVSTGUI_LIVE_EDITING")

	set_target_properties(${target} PROPERTIES SUFFIX ".aaxplugin")

	# --- turn off incremental linking; this reduces plugin size by removing the .ilk file from the package
	set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")

	# --- add linked libs, as per Avid examples
	target_link_libraries(${target} PRIVATE "odbc32.lib")
	target_link_libraries(${target} PRIVATE "odbccp32.lib")
	target_link_libraries(${target} PRIVATE "psapi.lib")

	# --- set the output directories - note that an AAX plugin is actually a directory, not unlike a MacOS plugin (AU, VST, AAX)
	set_target_properties(${target}
    		PROPERTIES
    		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive"
			  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER}/$(Configuration)/$(ProjectName).aaxplugin/Contents/$(PlatformName)/"
   			LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${AAX_PLUGIN_FOLDER}/$(Configuration)/$(ProjectName).aaxplugin/Contents/$(PlatformName)/"
	)

	# --- RUN THE PRE-BUILD BAT FILE
	add_custom_command(TARGET ${target}
	                   PRE_BUILD
	                   COMMAND cmd /c "${AAX_SDK_ROOT}/Utilities/CreatePackage.bat" "$(OutDir)" "${AAX_SDK_ROOT}/Utilities/PlugIn.ico")
endif()


# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AAX_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OBJECTS_SOURCE_ROOT})

# --- from original xcconfig files: same for debug/release
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces/ACF)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Interfaces/ACF)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/MIDI)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/Platform/include)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/Extensions/Topology)
target_include_directories(${target} PUBLIC ${AAX_SDK_ROOT}/TI/SignalProcessing)


# ---------------------------------------------------------------------------------
#
# ---  Resources:
#
# ---------------------------------------------------------------------------------
set(resource_sources
	${RESOURCE_ROOT}/PluginGUI.uidesc
)

# --- attach to target
target_sources(${target} PRIVATE ${resource_sources})

if(XCODE)
	set_source_files_properties(${resource_sources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# --- set folder/filter
source_group(Resources FILES ${resource_sources})

if(XCODE)
	# --- this eliminates a warning that looks suspicious when debugging but is acutally not a problem
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${AAX_BUNDLE_ID}")

	# --- info.pList files
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS "PRODUCT_NAME=$(PRODUCT_NAME) EXECUTABLE_NAME=$(EXECUTABLE_NAME) PRODUCT_BUNDLE_IDENTIFIER=${AAX_BUNDLE_ID} DYLIB_CURRENT_VERSION=$(DYLIB_CURRENT_VERSION) VERSION_NUM=$(DYLIB_CURRENT_VERSION) VERSION_STR=$(DYLIB_CURRENT_VERSION) CURRENT_YEAR=$(CURRENT_YEAR) AVID_COPYRIGHT=$(AVID_COPYRIGHT) AVID_COMPANY_STRING=$(AVID_COMPANY_STRING)")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

	# --- copy folders (.bundle and .component are directories, so use remove/copy directory)
	#     NOTE: /Library/Application Support/Avid/Audio/Plug-Ins is the AAX location BUT this folder is protected so can't copy programmatically
	#
	#           User must manually copy from the /mac_build/AAX folder to the /Library/Application Support/Avid/Audio/Plug-Ins folder
elseif(WIN)
	# --- WIN resource files
	target_sources(${target} PRIVATE ${WIN_ROOT}/PluginGUIMain.rc)
	target_sources(${target} PRIVATE ${RESOURCE_ROOT}/PluginGUI.uidesc)
endif()
