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

message(STATUS "------> Starting AU Project Build:")
message(STATUS "        AU SDK Location: ${AU_SDK_ROOT}")
message(STATUS "")

# ---------------------------------------------------------------------------------
#
# ---  Basic Setup
#
# ---------------------------------------------------------------------------------
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")

# ---  Export no symbols by default
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# ---  Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(MAC)
    # ---  macOS defaults
    if(NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET})
    	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "macOS deployment target")
    endif()

    if(NOT DEFINED ENV{SDKROOT})
		execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    option(BUILD_UNIVERSAL_BINARY "Build universal binary (32 & 64 bit)" OFF)
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

# --- these will show up as '-DDEVELOPMENT' but #ifdef DEVELOPMENT is they syntax (as usual)
add_compile_options($<$<CONFIG:Debug>:-DDEVELOPMENT=1>)
add_compile_options($<$<CONFIG:Release>:-DRELEASE=1>)

# ---------------------------------------------------------------------------------
#
# --- RAFX AU Ported Project
#
# ---------------------------------------------------------------------------------
project(${EXP_SUBPROJECT_NAME_AU})

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
endif()

# --- Output Directory for AU Plug-Ins (will also copy to the ~Library/Audio/Plug-Ins/Components folder later)
# set(AU_OUTPUT_DIR ${CMAKE_BINARY_DIR}/AUPlugin)

# --- add RackAFX ported project(s)
#
message(STATUS "     -> Adding AU ported project in source folder: ${GRANDPARENT_DIR}/project_source")
message(STATUS "        AU Target Name: ${EXP_SUBPROJECT_NAME_AU}")
message(STATUS "")

# --- add the project
add_subdirectory(${PROJECT_FOLDER}project_source/${AU_CMAKE_FOLDER} ${PROJECT_FOLDER}project_source/${AU_CMAKE_FOLDER})
