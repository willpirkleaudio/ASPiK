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

# --- default project folder location
set(PROJECT_FOLDER "../../")

message(STATUS "------> Starting AAX Project Build:")
message(STATUS "        AAX SDK Location: ${AAX_SDK_ROOT}")
message(STATUS "")

# ---------------------------------------------------------------------------------
#
# ---  Basic Setup
#
# ---------------------------------------------------------------------------------
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")

# ---  Export no symbols by default AAX
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# --- find mac SDK
if(MAC)
    # ---  macOS defaults
    if(NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET})
    	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "macOS deployment target")
    endif()

    if(NOT DEFINED ENV{SDKROOT})
		execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # --- ON for AAX
    option(BUILD_UNIVERSAL_BINARY "Build universal binary (32 & 64 bit)" ON)
    if(BUILD_UNIVERSAL_BINARY)
        set(CMAKE_OSX_ARCHITECTURES "x86_64;i386" CACHE STRING "macOS universal binary")
	    set(CMAKE_XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_32_64_BIT)")
        set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "$<$<CONFIG:Debug>:YES>$<$<CONFIG:Release>:NO>")
    endif()
endif()

# ---  Setup Configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE STRING "Generate compile commands" FORCE)
set(CMAKE_SKIP_ASSEMBLY_SOURCE_RULES ON)
set(CMAKE_SKIP_INSTALL_RULES ON)
set(CMAKE_SKIP_PREPROCESSED_SOURCE_RULES ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Predefined)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- setup CRT
if(WIN)
	add_compile_options(/MP)
	add_compile_options(/bigobj)
	add_compile_options($<$<CONFIG:Debug>:/ZI>)
	# ---  /MDd = MultiThreadedDLL Debug Runtime
	# ---  /MD  = MultiThreadedDLL Runtime
	add_compile_options($<$<CONFIG:Debug>:/MDd>)
	add_compile_options($<$<CONFIG:Release>:/MD>)
endif()

# ---------------------------------------------------------------------------------
#
# --- RAFX AAX Ported Project
#
# ---------------------------------------------------------------------------------
project(${EXP_SUBPROJECT_NAME_AAX})

# --- set the project as startup rather than ALL_BUILD
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT ${EXP_SUBPROJECT_NAME_AAX})

if(UNIX)
	if(XCODE)
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
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

# --- add RackAFX ported project(s)
#
message(STATUS "     -> Adding AAX ported project in source folder: ${GRANDPARENT_DIR}/project_source")
message(STATUS "        AAX Target Name: ${EXP_SUBPROJECT_NAME_AAX}")
message(STATUS "")

# --- add the project
add_subdirectory(${PROJECT_FOLDER}project_source/${AAX_CMAKE_FOLDER} ${PROJECT_FOLDER}project_source/${AAX_CMAKE_FOLDER})
