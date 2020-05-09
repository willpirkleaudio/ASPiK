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
set(SDK_ROOT ${AU_SDK_ROOT})

set(MAC_ROOT "../../mac/au")
set(SOURCE_ROOT "../../source")
set(RESOURCE_ROOT "../../resources")
set(FFTW_SOURCE_ROOT "${SOURCE_ROOT}/FFTW")

# --- local roots
set(KERNEL_SOURCE_ROOT "${SOURCE_ROOT}/PluginKernel")
set(OBJECTS_SOURCE_ROOT "${SOURCE_ROOT}/PluginObjects")
set(VSTGUI_SOURCE_ROOT "${SOURCE_ROOT}/CustomControls")
set(AU_SOURCE_ROOT "${SOURCE_ROOT}/au_source")
set(COCOA_SOURCE_ROOT "${SOURCE_ROOT}/au_source/cocoa_ui")

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
# ---  AU Core files
#
# ---------------------------------------------------------------------------------
if(IS_SYNTH_PLUGIN)
set(au_core_sources
	${AU_SOURCE_ROOT}/ausynthplugin.h
	${AU_SOURCE_ROOT}/ausynthplugin.cpp)
else()
set(au_core_sources
	${AU_SOURCE_ROOT}/aufxplugin.h
	${AU_SOURCE_ROOT}/aufxplugin.cpp)
endif()


# ---------------------------------------------------------------------------------
#
# ---  COCOA UI files
#
# ---------------------------------------------------------------------------------
set(cocoa_sources
	${COCOA_SOURCE_ROOT}/aucocoaviewfactory.mm
)

# ---------------------------------------------------------------------------------
#
# ---  AU SDK files
#
# ---------------------------------------------------------------------------------
set(aubase_sources
	${SDK_ROOT}/AUPublic/AUBase/AUBase.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUBase.h
	${SDK_ROOT}/AUPublic/AUBase/AUDispatch.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUDispatch.h
	${SDK_ROOT}/AUPublic/AUBase/AUInputElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUInputElement.h
	${SDK_ROOT}/AUPublic/AUBase/AUOutputElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUOutputElement.h
	${SDK_ROOT}/AUPublic/AUBase/AUPlugInDispatch.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUPlugInDispatch.h
	${SDK_ROOT}/AUPublic/AUBase/AUScopeElement.cpp
	${SDK_ROOT}/AUPublic/AUBase/AUScopeElement.h
	${SDK_ROOT}/AUPublic/AUBase/ComponentBase.cpp
	${SDK_ROOT}/AUPublic/AUBase/ComponentBase.h
)

set(auinstbase_sources
	${SDK_ROOT}/AUPublic/AUInstrumentBase/AUInstrumentBase.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/AUInstrumentBase.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/LockFreeFIFO.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/MIDIControlHandler.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthElement.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthElement.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthEvent.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNote.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNote.h
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNoteList.cpp
	${SDK_ROOT}/AUPublic/AUInstrumentBase/SynthNoteList.h
)

set(auotherbase_sources
	${SDK_ROOT}/AUPublic/OtherBases/AUEffectBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUEffectBase.h
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIBase.h
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIEffectBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/AUMIDIEffectBase.h
	${SDK_ROOT}/AUPublic/OtherBases/MusicDeviceBase.cpp
	${SDK_ROOT}/AUPublic/OtherBases/MusicDeviceBase.h
)

set(auutility_sources
	${SDK_ROOT}/AUPublic/Utility/AUBaseHelper.cpp
	${SDK_ROOT}/AUPublic/Utility/AUBaseHelper.h
	${SDK_ROOT}/AUPublic/Utility/AUBuffer.cpp
	${SDK_ROOT}/AUPublic/Utility/AUBuffer.h
	${SDK_ROOT}/AUPublic/Utility/AUMIDIDefs.h
	${SDK_ROOT}/AUPublic/Utility/AUSilentTimeout.h
)

set(aupublicutility_sources
	${SDK_ROOT}/PublicUtility/CAAtomic.h
	${SDK_ROOT}/PublicUtility/CAAtomicStack.h
	${SDK_ROOT}/PublicUtility/CAAudioChannelLayout.cpp
	${SDK_ROOT}/PublicUtility/CAAudioChannelLayout.h
	${SDK_ROOT}/PublicUtility/CAAutoDisposer.h
	${SDK_ROOT}/PublicUtility/CABufferList.cpp
	${SDK_ROOT}/PublicUtility/CABufferList.h
	${SDK_ROOT}/PublicUtility/CAByteOrder.h
	${SDK_ROOT}/PublicUtility/CADebugger.cpp
	${SDK_ROOT}/PublicUtility/CADebugger.h
	${SDK_ROOT}/PublicUtility/CADebugMacros.cpp
	${SDK_ROOT}/PublicUtility/CADebugMacros.h
	${SDK_ROOT}/PublicUtility/CADebugPrintf.cpp
	${SDK_ROOT}/PublicUtility/CADebugPrintf.h
	${SDK_ROOT}/PublicUtility/CAException.h
	${SDK_ROOT}/PublicUtility/CAGuard.cpp
	${SDK_ROOT}/PublicUtility/CAGuard.h
	${SDK_ROOT}/PublicUtility/CAHostTimeBase.cpp
	${SDK_ROOT}/PublicUtility/CAHostTimeBase.h
	${SDK_ROOT}/PublicUtility/CALogMacros.h
	${SDK_ROOT}/PublicUtility/CAMath.h
	${SDK_ROOT}/PublicUtility/CAMutex.cpp
	${SDK_ROOT}/PublicUtility/CAMutex.h
	${SDK_ROOT}/PublicUtility/CAReferenceCounted.h
	${SDK_ROOT}/PublicUtility/CAStreamBasicDescription.cpp
	${SDK_ROOT}/PublicUtility/CAStreamBasicDescription.h
	${SDK_ROOT}/PublicUtility/CAThreadSafeList.h
	${SDK_ROOT}/PublicUtility/CAVectorUnit.cpp
	${SDK_ROOT}/PublicUtility/CAVectorUnit.h
	${SDK_ROOT}/PublicUtility/CAVectorUnitTypes.h
	${SDK_ROOT}/PublicUtility/CAXException.cpp
	${SDK_ROOT}/PublicUtility/CAXException.h
)

# ---------------------------------------------------------------------------------
#
# ---  VSTGUI4 files
#
# ---------------------------------------------------------------------------------
set(vstgui_sources
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_mac.mm
	${VSTGUI_ROOT}/vstgui4/vstgui/vstgui_uidescription_mac.mm
)

# ---------------------------------------------------------------------------------
#
# ---  setup IDE folders/filters
#
# ---------------------------------------------------------------------------------
source_group(PluginKernel FILES ${kernel_sources})
source_group(PluginObjects FILES ${plugin_object_sources})
source_group(CustomVSTGUI FILES ${custom_vstgui_sources})
source_group(AUCore FILES ${au_core_sources})
source_group(vstgui4 FILES ${vstgui_sources})
source_group(AUSDK\\AUBase FILES ${aubase_sources})
source_group(AUSDK\\AUInstrumentBase FILES ${auinstbase_sources})
source_group(AUSDK\\OtherBases FILES ${auotherbase_sources})
source_group(AUSDK\\Utility FILES ${auutility_sources})
source_group(AUSDK\\PublicUtility FILES ${aupublicutility_sources})

# ---------------------------------------------------------------------------------
#
# ---  COCOA UI target:
#
# ---------------------------------------------------------------------------------
set(cocoatarget ${EXP_SUBPROJECT_NAME_AU}_CocoaUI)

# ---  add the plugin source and link to lib, etc…
add_library(${cocoatarget} MODULE ${cocoa_sources})

# source_group(CocoaUI FILES ${cocoa_sources})

# --- output folder for the plugin
set_target_properties(${cocoatarget} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER})

# --- set installation directory (must have this for cocoa)
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INSTALL_PATH "$(USER_LIBRARY_DIR)/Bundles" XCODE_ATTRIBUTE_SKIP_INSTALL "Yes")

# --- product name matches AU plugin target name
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_NAME ${EXP_SUBPROJECT_NAME_AU})

# --- save product name for later
get_target_property(COCOAPRODUCT_NAME ${cocoatarget} XCODE_ATTRIBUTE_PRODUCT_NAME)

# --- setup project
if(XCODE)
	set_target_properties(${cocoatarget} PROPERTIES BUNDLE TRUE)
	set_target_properties(${cocoatarget} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${cocoatarget} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
	set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")


	# --- note: " " prior to AUPLUGIN etc... must be there to separate vars
	set_property(TARGET ${cocoatarget} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AUPLUGIN=1>$<$<CONFIG:Release>: AUPLUGIN=1>")
	set_property(TARGET ${cocoatarget} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")

	target_link_libraries(${cocoatarget} PRIVATE "-framework Cocoa")
	target_link_libraries(${cocoatarget} PRIVATE "-framework AudioToolbox")
	target_link_libraries(${cocoatarget} PRIVATE "-framework AudioUnit")

	# ---  setup header search paths
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${VSTGUI_ROOT}/vstgui4)
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OBJECTS_SOURCE_ROOT})
	target_include_directories(${cocoatarget} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})
endif()

# ---------------------------------------------------------------------------------
#
# ---  AU target:
#
# ---------------------------------------------------------------------------------
set(target ${EXP_SUBPROJECT_NAME_AU})

# --- note this is placed *before* add_library( ) on purpose!
if(LINK_FFTW)
	link_directories(/opt/local/lib)
endif()

# ---  add the plugin source
if(INCLUDE_FX_OBJECTS)
	add_library(${target} MODULE ${kernel_sources} ${plugin_object_sources} ${au_core_sources} ${vstgui_sources} ${custom_vstgui_sources} ${aubase_sources} ${auinstbase_sources} ${auotherbase_sources} ${auutility_sources} ${aupublicutility_sources})
else()
	add_library(${target} MODULE ${kernel_sources} ${au_core_sources} ${vstgui_sources} ${custom_vstgui_sources} ${aubase_sources} ${auinstbase_sources} ${auotherbase_sources} ${auutility_sources} ${aupublicutility_sources})
endif()

# --- output folder for the plugin
set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER})

# --- add the exported symbol file
if(IS_SYNTH_PLUGIN)
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT}/ausynthplugin.exp")
else()
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT}/aufxplugin.exp")
endif()

# --- setup the project
if(XCODE)
	set_target_properties(${target} PROPERTIES BUNDLE TRUE)
	set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET default)
	set_target_properties(${target} PROPERTIES CMAKE_C_VISIBILITY_PRESET default)
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
  set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "component")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")
	set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")

	# --- note: " " prior to AUPLUGIN etc... must be there to separate vars
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: VSTGUI_LIVE_EDITING=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: AUPLUGIN=1>$<$<CONFIG:Release>: AUPLUGIN=1>")
	set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: DEVELOPMENT=1>$<$<CONFIG:Release>: RELEASE=1>")
	if(LINK_FFTW)
		set_property(TARGET ${target} APPEND_STRING PROPERTY XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "$<$<CONFIG:Debug>: HAVE_FFTW=1>$<$<CONFIG:Release>: HAVE_FFTW=1>")
	endif()

	target_link_libraries(${target} PRIVATE "-framework OpenGL")
	target_link_libraries(${target} PRIVATE "-framework CoreMIDI")
	target_link_libraries(${target} PRIVATE "-framework Accelerate")
	target_link_libraries(${target} PRIVATE "-framework QuartzCore")
	target_link_libraries(${target} PRIVATE "-framework Cocoa")
	target_link_libraries(${target} PRIVATE "-framework AudioToolbox")
	target_link_libraries(${target} PRIVATE "-framework AudioUnit")
	target_link_libraries(${target} PRIVATE "-framework CoreAudio")
	target_link_libraries(${target} PRIVATE "-framework CoreFoundation")

	if(LINK_FFTW)
		message(STATUS "---> LINK FFTW Found: + Adding /opt/local/include to the header search path.")
		message(STATUS "                      + Adding /opt/local/lib to the library search path.")

		# --- set the FFTW include directory
		target_include_directories(${target} PUBLIC "/opt/local/include")

		# --- add the fftw lib to the dependent lib list
		message(STATUS "                      + Adding /opt/local/lib/libfftw3.a to the link library list.")
		target_link_libraries(${target} PRIVATE libfftw3.a)

		target_compile_definitions(${target} PUBLIC HAVE_FFTW=1)
		message(STATUS "                      + Adding HAVE_FFTW to the pre-processor definitions.")
	endif()
endif()


# ---  setup header search paths
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/)
target_include_directories(${target} PUBLIC ${VSTGUI_ROOT}/vstgui4)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/PublicUtility)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/Utility)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/AUBase)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/AUInstrumentBase)
target_include_directories(${target} PUBLIC ${SDK_ROOT}/AUPublic/OtherBases)
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OBJECTS_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${AU_SOURCE_ROOT})
target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${VSTGUI_SOURCE_ROOT})


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
set_source_files_properties(${resource_sources} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# --- set folder/filter
source_group(Resources FILES ${resource_sources})

# --- info.pList file for target
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")

# Mike
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist")

# --- need to edit this manually due to error with CMake placing returns in whitespaces in the formatted "Your Company : My Plugin" string...
if(IS_SYNTH_PLUGIN)
	file(READ "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist" INFOPLIST_FILE_GUTSA)
	string(REPLACE "AUFXPluginFactory" "AUSynthPluginFactory" INFOPLIST_FILE_GUTS ${INFOPLIST_FILE_GUTSA})
else()
	file(READ "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist" INFOPLIST_FILE_GUTS)
endif()

string(LENGTH ${INFOPLIST_FILE_GUTS} TOTAL_GUTS_LEN)

# --- get left half
set(LEFT_CHUNK_TAG "<key>name</key>")
string(LENGTH ${LEFT_CHUNK_TAG} LEFT_CHUNK_TAG_LEN)
string(FIND ${INFOPLIST_FILE_GUTS} "<key>name</key>" LEFT_CHUNK_END)
MATH(EXPR LEFT_CHUNK_POS "${LEFT_CHUNK_END} + ${LEFT_CHUNK_TAG_LEN}")
string(SUBSTRING ${INFOPLIST_FILE_GUTS} 0 ${LEFT_CHUNK_POS} INFOPLIST_FILE_LEFT_CHUNK)

# --- get right half
string(FIND ${INFOPLIST_FILE_GUTS} "<key>sandboxSafe</key>" RIGHT_CHUNK_START)
MATH(EXPR RIGHT_CHUNK_LEN "${TOTAL_GUTS_LEN} - ${RIGHT_CHUNK_START}")
string(SUBSTRING ${INFOPLIST_FILE_GUTS} ${RIGHT_CHUNK_START} ${RIGHT_CHUNK_LEN} INFOPLIST_FILE_RIGHT_CHUNK)

# --- form substring string
string(CONCAT INFO_LIST_COMP_NAME "<string>" ${INFOPLIST_PI_VENDOR_COLON_NAME_CODE} "</string>")

# --- form completed string
string(CONCAT INFOPLIST_FILE_GUTS_MODIFIED ${INFOPLIST_FILE_LEFT_CHUNK} "\n      " ${INFO_LIST_COMP_NAME} "\n      " ${INFOPLIST_FILE_RIGHT_CHUNK})

# --- update the file (destructive)
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/Info.plist" ${INFOPLIST_FILE_GUTS_MODIFIED})

# --- this eliminates a warning that looks suspicious when debugging but is actally not a problem
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${AU_BUNDLE_ID}")
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${AU_BUNDLE_ID}")

# --- preprocessor defines (can NOT include spaces in any strings)
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS "EXECUTABLE_NAME=$(EXECUTABLE_NAME) PRODUCT_BUNDLE_IDENTIFIER=${AU_BUNDLE_ID} VENDOR_4CHAR_CODE=${4CHAR_AAX_MANUFACTURER_ID_PROPER} PI_4CHAR_CODE=${4CHAR_CODE_PROPER} AU_TYPE_CODE=${INFOPLIST_AU_TYPE_CODE}")
set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

# --- info.pList file for cocoa GUI target
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${MAC_ROOT}/cocoaInfo.plist")
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS "EXECUTABLE_NAME=$(EXECUTABLE_NAME) PRODUCT_BUNDLE_IDENTIFIER=${AU_BUNDLE_ID} COCOA_VIEW_FACTORY=${AU_COCOA_VIEWFACTORY_NAME}")
set_target_properties(${cocoatarget} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")

# --- copy folders (.bundle and .component are directories, so use remove/copy directory)
set(TARGET_PATH_DEBUG "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/Debug/${target}.component")
set(COCOATARGET_PATH_DEBUG "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/Debug/${COCOAPRODUCT_NAME}.bundle")

set(TARGET_PATH_RELEASE "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/Release/${target}.component")
set(COCOATARGET_PATH_RELEASE "${CMAKE_BINARY_DIR}/${AU_PLUGIN_FOLDER}/Release/${COCOAPRODUCT_NAME}.bundle") 

set(COCOA_IN_RSRSC_PATH_DEBUG ${TARGET_PATH_DEBUG}/Contents/Resources/${COCOAPRODUCT_NAME}.bundle)
set(COCOA_IN_RSRSC_PATH_RELEASE ${TARGET_PATH_RELEASE}/Contents/Resources/${COCOAPRODUCT_NAME}.bundle)

if(XCODE)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory "$<$<CONFIG:Debug>:${COCOA_IN_RSRSC_PATH_DEBUG}>$<$<CONFIG:Release>:${COCOA_IN_RSRSC_PATH_RELEASE}>")
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "$<$<CONFIG:Debug>:${COCOATARGET_PATH_DEBUG}>$<$<CONFIG:Release>:${COCOATARGET_PATH_RELEASE}>" "$<$<CONFIG:Debug>:${COCOA_IN_RSRSC_PATH_DEBUG}>$<$<CONFIG:Release>:${COCOA_IN_RSRSC_PATH_RELEASE}>")

	# --- copy plugin into library folder
	set(PLUGIN_FOLDER ~/Library/Audio/Plug-Ins/Components/${target}.component)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLUGIN_FOLDER})
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "$<$<CONFIG:Debug>:${TARGET_PATH_DEBUG}>$<$<CONFIG:Release>:${TARGET_PATH_RELEASE}>" ${PLUGIN_FOLDER})
endif()
