To install and play with this demo project:

# ---- UNIVERSAL ---------------------------------------------------------------
1. Universal Build: place the DemoVolumePlugin into the /myprojects/ subfolder of your ALL_SDK container, as per the SDK docs and the tutorial videos
2. Run CMake to create the demo project; build it and run in your DAW(s)
# ------------------------------------------------------------------------------

# ------- AAX ------------------------------------------------------------------
1. AAX Build (individual): place the DemoVolumePlugin into the /myprojects/ subfolder of your AAX_SDK container, as per the SDK docs and the tutorial videos
2. Edit the DemoVolumePlugin/CMakeLists.txt with any text editor and set the universal build to FALSE
3. Set the three API flags accordingly (only AAX is TRUE)
4. Run CMake to create the demo project; build it and run in your DAW(s)

# --- Universal Build Flag - when using combined SDKs; can still build independently
#                            with individual flags below
set(UNIVERSAL_SDK_BUILD FALSE) 	# <-- set TRUE or FALSE

# --- Individual project builds
set(AAX_SDK_BUILD TRUE)# <-- set TRUE or FALSE
set(AU_SDK_BUILD FALSE)# <-- set TRUE or FALSE
set(VST_SDK_BUILD FALSE)# <-- set TRUE or FALSE
# ------------------------------------------------------------------------------

# ------- AU -------------------------------------------------------------------
1. AU Build (individual): place the DemoVolumePlugin into the /myprojects/ subfolder of your AU_SDK container, as per the SDK docs and the tutorial videos
2. Edit the DemoVolumePlugin/CMakeLists.txt with any text editor and set the universal build to FALSE
3. Set the three API flags accordingly (only AU is TRUE)
4. Run CMake to create the demo project; build it and run in your DAW(s)

# --- Universal Build Flag - when using combined SDKs; can still build independently
#                            with individual flags below
set(UNIVERSAL_SDK_BUILD FALSE) 	# <-- set TRUE or FALSE

# --- Individual project builds
set(AAX_SDK_BUILD FALSE)# <-- set TRUE or FALSE
set(AU_SDK_BUILD TRUE)# <-- set TRUE or FALSE
set(VST_SDK_BUILD FALSE)# <-- set TRUE or FALSE
# ------------------------------------------------------------------------------

# ------- VST3 -----------------------------------------------------------------
1. VST3 Build (individual): place the DemoVolumePlugin into the /myprojects/ subfolder of your VST3_SDK container, as per the SDK docs and the tutorial videos
2. Edit the DemoVolumePlugin/CMakeLists.txt with any text editor and set the universal build to FALSE
3. Set the three API flags accordingly (only VSTU is TRUE)
4. Run CMake to create the demo project; build it and run in your DAW(s)

# --- Universal Build Flag - when using combined SDKs; can still build independently
#                            with individual flags below
set(UNIVERSAL_SDK_BUILD FALSE) 	# <-- set TRUE or FALSE

# --- Individual project builds
set(AAX_SDK_BUILD FALSE)# <-- set TRUE or FALSE
set(AU_SDK_BUILD FALSE)# <-- set TRUE or FALSE
set(VST_SDK_BUILD TRUE)# <-- set TRUE or FALSE
# ------------------------------------------------------------------------------
