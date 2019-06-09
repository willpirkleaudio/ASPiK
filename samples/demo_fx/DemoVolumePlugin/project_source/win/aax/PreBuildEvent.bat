set OutDir=%1

CALL C:\AAX_SDKs\AAX_SDK\Utilities\CreatePackage.bat %OutDir% C:\AAX_SDKs\AAX_SDK\Utilities\PlugIn.ico

REM $(Configuration)\$(ProjectName).aaxplugin\Contents\$(PlatformName)\

odbc32.lib;odbccp32.lib;psapi.lib;%(AdditionalDependencies)

C:\AAX_SDKs\AAX_SDK\Libs\Debug\AAXLibrary_x64_D.lib;odbc32.lib;odbccp32.lib;psapi.lib;kernel32.lib;user32.lib;gdi32.lib;win
spool.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;comdlg32.lib;advapi32.lib


AAXPLUGIN;_DEBUG;WIN32;_WINDOWS;WINDOWS_VERSION;_CRT_SECURE_NO_DEPRECATE;_SCL_SECURE_NO_DEPRECATE;%(PreprocessorDefinitions)



