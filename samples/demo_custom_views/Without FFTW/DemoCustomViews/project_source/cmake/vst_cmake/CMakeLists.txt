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
set(WIN_ROOT "../../win/vst")
set(MAC_ROOT "../../mac/vst")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")

# --- local roots
set(KERNEL_SOURCE_ROOT "${SOURCE_ROOT}/PluginKernel")
set(OBJECTS_SOURCE_ROOT "${SOURCE_ROOT}/PluginObjects")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/CustomControls")
set(VST_SOURCE_ROOT "${SOURCE_ROOT}/vst_source")
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
# ---  VST3 Core plugin files
#
# ---------------------------------------------------------------------------------
set(vst_core_sources
	${VST_SOURCE_ROOT}/factory.cpp
	${VST_SOURCE_ROOT}/customparameters.h
	${VST_SOURCE_ROOT}/customparameters.cpp
	${VST_SOURCE_ROOT}/channelformats.h
	${VST_SOURCE_ROOT}/vst3plugin.h
	${VST_SOURCE_ROOT}/vst3plugin.cpp
)

# ---------------------------------------------------------------------------------
#
# ---  VST SDK files
#
# ---------------------------------------------------------------------------------
set(vstsdk_sources
	${SDK_ROOT}/public.sdk/source/vst/vstsinglecomponenteffect.cpp
	${SDK_ROOT}/public.sdk/source/vst/vstsinglecomponenteffect.h
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
# ---  VST3 target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_VST})

# --- 3.6.13 this is the proper default path; for MacOS it is the VST3 plugin default location, for Win it is empty ""
smtg_get_default_vst3_path(DEFAULT_VST3_FOLDER)
set(PLUGIN_TARGET_PATH "${DEFAULT_VST3_FOLDER}" CACHE PATH "Here you can redefine the VST3 plug-ins folder")
set(SMTG_PLUGIN_TARGET_PATH ${PLUGIN_TARGET_PATH})
# --- message(STATUS "set_target_properties ---> SMTG_PLUGIN_TARGET_PATH is set to : " ${SMTG_PLUGIN_TARGET_PATH})

# --- note this is placed *before* smtg_add_vst3plugin( ) on purpose!
if(LINK_FFTW)
	if(MAC)
		link_directories(/opt/local/lib)
	endif()
endif()

# --- add the plugin source and link to lib, etc... smtg_add_vst3plugin does all the setup
if(INCLUDE_FX_OBJECTS)
	smtg_add_vst3plugin(${target} ${kernel_sources} ${plugin_object_sources}  ${vst_core_sources} ${custom_vstgui_sources} ${vstsdk_sources})
else()
	smtg_add_vst3plugin(${target} ${kernel_sources} ${vst_core_sources} ${custom_vstgui_sources} ${vstsdk_sources})
endif()

# --- VST3 SDK 3.7.2 addition
target_compile_features(${target}
PUBLIC
    cxx_std_14
)

# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OBJECTS_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VST_SOURCE_ROOT})

# --- setup link lib
if(LINK_FFTW)

	if(WIN)
		add_definitions(-DHAVE_FFTW=1)
		target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${FFTW_SOURCE_ROOT})
		message(STATUS "---> LINK FFTW Found. Copying FFW DLL to the validator.exe folder.")

		set(VST_BIN_DEBUG ${WINBUILD_ROOT}/bin/Debug)
		set(VST_BIN_RELEASE ${WINBUILD_ROOT}/bin/Release)

		file(MAKE_DIRECTORY ${VST_BIN_DEBUG})
		file(MAKE_DIRECTORY ${VST_BIN_RELEASE})

		if(EXISTS ${VST_BIN_DEBUG})
		   configure_file(${FFTW_SOURCE_ROOT}/x64/libfftw3-3.dll ${VST_BIN_DEBUG}/libfftw3-3.dll COPYONLY)
		endif()

		if(EXISTS ${VST_BIN_RELEASE})
		   configure_file(${FFTW_SOURCE_ROOT}/x64/libfftw3-3.dll ${VST_BIN_RELEASE}/libfftw3-3.dll COPYONLY)
		endif()

		if(EXISTS ${VST_BIN_DEBUG}/libfftw3-3.dll)
			message(STATUS "     + copied FFTW DLL: " ${VST_BIN_DEBUG}/libfftw3-3.dll)
		else()
			message(STATUS "--- ERROR: Could not copy FFTW DLL to " ${VST_BIN_DEBUG}/libfftw3-3.dll)
			message(STATUS "           This will cause validator to fail as the project needs FFTW")
		endif()

		if(EXISTS ${VST_BIN_RELEASE}/libfftw3-3.dll)
			message(STATUS "     + copied FFTW DLL: " ${VST_BIN_RELEASE}/libfftw3-3.dll)
		else()
			message(STATUS "--- ERROR: Could not copy FFTW DLL to " ${VST_BIN_RELEASE}/libfftw3-3.dll)
			message(STATUS "           This will cause validator to fail as the project needs FFTW")
		endif()

		message(STATUS " ")

		# --- add the fftw lib to the dependent lib list

		target_link_libraries(${target} PRIVATE base sdk vstgui_support ${CMAKE_CURRENT_SOURCE_DIR}/${FFTW_SOURCE_ROOT}/x64/libfftw3-3.lib)
		message(STATUS "---> LINK FFTW Found: + Adding libfftw3-3.lib to the compiler libraries.")
		message(STATUS "                      + Adding HAVE_FFTW to the pre-processor definitions.")

	else()
		message(STATUS "---> LINK FFTW Found: + Adding /opt/local/include to the header search path.")
		message(STATUS "                      + Adding /opt/local/lib to the library search path.")

		target_include_directories(${target} PUBLIC "/opt/local/include")

		message(STATUS "                      + Adding /opt/local/lib/libfftw3.a to the link library list.")
		target_link_libraries(${target} PRIVATE base sdk vstgui_support libfftw3.a)

		target_compile_definitions(${target} PUBLIC HAVE_FFTW=1)
		message(STATUS "                      + Adding HAVE_FFTW to the pre-processor definitions.")

	endif()

else()
	target_link_libraries(${target} PRIVATE base sdk vstgui_support)
endif()

# --- preprocessor for D2D for windows
if(WIN)
	add_definitions(-DVSTGUI_DIRECT2D_SUPPORT=1)
endif()

# ---------------------------------------------------------------------------------
#
# ---  Resources:
#
# ---------------------------------------------------------------------------------
smtg_add_vst3_resource(${target} "${RESOURCE_ROOT}/PluginGUI.uidesc")

if(MAC)
	smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist" PREPROCESS)
elseif(WIN)
	target_sources(${target} PRIVATE ${WIN_ROOT}/PluginGUIMain.rc)
	target_sources(${target} PRIVATE ${RESOURCE_ROOT}/PluginGUI.uidesc)
endif()


# fix missing VSTPluginMain symbol when also building VST 2 version
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "")

if (WIN)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# for Xcode, add info.plist variables
if(XCODE)
	# --- info.pList files
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS "EXECUTABLE_NAME=$(EXECUTABLE_NAME) PRODUCT_BUNDLE_IDENTIFIER=${VST_BUNDLE_ID}")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")
endif()

# --- add post build rule to copy VST3 icon for bundle appearance of folder
if(WIN)
    set(DEFAULT_ICON_PATH ${SDK_ROOT}/vst3_doc/artwork/VST_Logo_Steinberg.ico)
    set(PROJECT_SOURCE_DIR ${SDK_ROOT})
    set(MSG_ICON_PATH "Path to the package icon (VST_Logo_Steinberg.ico) for Windows")
    if(EXISTS ${DEFAULT_ICON_PATH})
        set(SMTG_PACKAGE_ICON_PATH ${DEFAULT_ICON_PATH})
    else()
        set(SMTG_PACKAGE_ICON_PATH "" CACHE FILEPATH ${MSG_ICON_PATH})
        message(STATUS "SMTG_PACKAGE_ICON_PATH is not set to (as expected) : " ${DEFAULT_ICON_PATH})
    endif()

    # --- add the post build rule here:
    smtg_add_folder_icon(${target} ${SMTG_PACKAGE_ICON_PATH})
endif()

source_group(PluginKernel FILES ${kernel_sources})
source_group(PluginObjects FILES ${plugin_object_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
source_group(VST3Core FILES ${vst_core_sources})
source_group(VST3SDK FILES ${vstsdk_sources})
