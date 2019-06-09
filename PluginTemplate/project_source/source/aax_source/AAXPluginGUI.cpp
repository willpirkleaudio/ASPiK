// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxplugingui.cpp
//
/**
    \file   aaxplugingui.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  implmentation file for the AAXPluginGUI object which creates and
    		destroys the ASPiK GUI object as well as pumps synchronized
    		parameter change information as part of the monolithic parameter
    		update loop.

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "AAXPluginGUI.h"

#if _WINDOWS // ------------------------------------------------------------------------
#include <windows.h>

void* hInstance = nullptr;

extern "C" BOOL WINAPI DllMain(HINSTANCE iInstance, DWORD iSelector, LPVOID iReserved)
{
	if(iSelector == DLL_PROCESS_ATTACH)
	{
		hInstance = (HINSTANCE)iInstance;
	}
	return true;
}
#endif //_WINDOWS -----------------------------------------------------------------------


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::AAXPluginGUI
//
/**
 \brief object constructor

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAXPluginGUI::AAXPluginGUI(void)
{
	guiWidth = 0;
	guiHeight = 0;
	pureCustomGUI = false;
    pluginGUI = nullptr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::~AAXPluginGUI
//
/**
 \brief object destructor

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAXPluginGUI::~AAXPluginGUI(void)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::Create()
//
/**
 \brief creation mechanism for this object

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_IEffectGUI* AAX_CALLBACK AAXPluginGUI::Create()
{
	return new AAXPluginGUI;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::CreateViewContainer()
//
/**
 \brief called when user opens the GUI

 NOTES:
 - this creates the ASPiK-GUI object
 - slightly different between Windows and MacOS versions
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginGUI::CreateViewContainer()
{
#if defined _WINDOWS || defined _WINDLL
	if(this->GetViewContainerType() == AAX_eViewContainer_Type_HWND)
	{
         customData.guiPlugin_Connector = nullptr;
        customData.plugin_Core = nullptr;
        uint32_t oSize;

        AAX_Result success = GetEffectParameters()->GetCustomData(PLUGIN_CUSTOMDATA_ID, sizeof(pluginCustomData*), &customData, &oSize);

        if(success != AAX_SUCCESS)
            return; // fail

        guiWidth = 0;
        guiHeight = 0;

        if(!customData.guiPlugin_Connector)
            return; // fail
        if(!customData.plugin_Core)
            return; // fail

        // --- first see if they have a custom GUI
        void* createdCustomGUI = nullptr;

        if(!createdCustomGUI)
        {
            // --- set up the GUI parameter copy list
            std::vector<PluginParameter*>* PluginParameterPtr = customData.plugin_Core->makePluginParameterVectorCopy();
            
            pluginGUI = new VSTGUI::PluginGUI("PluginGUI.uidesc");
            if(pluginGUI)
            {
                // --- create GUI
                bool opened = pluginGUI->open("Editor", this->GetViewContainerPtr(), PluginParameterPtr, VSTGUI::kHWND, customData.guiPlugin_Connector, nullptr);
                
                // --- delete the PluginParameterPtr guts, and pointer too...
                for(std::vector<PluginParameter*>::iterator it = PluginParameterPtr->begin(); it !=  PluginParameterPtr->end(); ++it)
                {
                    delete *it;
                }
                delete PluginParameterPtr;
                
                if(opened)
                {
                    pluginGUI->setAAXViewContainer(GetViewContainer());
                    float width, height = 0.f;
                    
                    pluginGUI->getSize(width, height);
                    guiWidth = (int)width;
                    guiHeight = (int)height;
                    
                    pluginGUI->setGUIWindowFrame((IGUIWindowFrame*)this);
                }
            }
            return;
        }
    }
    
#else
    if(this->GetViewContainerType() == AAX_eViewContainer_Type_NSView)
    {
        customData.guiPlugin_Connector = nullptr;
        customData.plugin_Core = nullptr;
        uint32_t oSize;

        AAX_Result success = GetEffectParameters()->GetCustomData(PLUGIN_CUSTOMDATA_ID, sizeof(pluginCustomData*), &customData, &oSize);

        if(success != AAX_SUCCESS)
            return; // fail

        guiWidth = 0;
        guiHeight = 0;

        if(!customData.guiPlugin_Connector)
            return; // fail
        if(!customData.plugin_Core)
            return; // fail

        // --- first see if they have a custom GUI
        void* createdCustomGUI = nullptr;

        if(!createdCustomGUI)
        {
            // --- Look for a resource in the main bundle by name and type.
            //     NOTE: this is set in info.plist
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFStringCreateWithCString(nullptr, customData.plugin_Core->getAAXBundleID(), kCFStringEncodingASCII));
            if (bundle == nullptr) return;

            // --- get .uidesc file
            CFURLRef bundleURL = CFBundleCopyResourceURL(bundle, CFSTR("PluginGUI"), CFSTR("uidesc"),nullptr);
            CFStringRef xmlPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
            int nSize = CFStringGetLength(xmlPath);
            char* path = new char[nSize+1];
            memset(path, 0, (nSize+1)*sizeof(char));
            CFStringGetFileSystemRepresentation(xmlPath, path, nSize+1);
            CFRelease(xmlPath);

            // --- set up the GUI parameter copy list
            std::vector<PluginParameter*>* PluginParameterPtr = customData.plugin_Core->makePluginParameterVectorCopy();

            pluginGUI = new VSTGUI::PluginGUI(path);
            if(pluginGUI)
            {
                // --- create GUI
                bool opened = pluginGUI->open("Editor", this->GetViewContainerPtr(), PluginParameterPtr, VSTGUI::kNSView, customData.guiPlugin_Connector, nullptr);

                // --- delete the PluginParameterPtr guts, and pointer too...
                for(std::vector<PluginParameter*>::iterator it = PluginParameterPtr->begin(); it !=  PluginParameterPtr->end(); ++it)
                {
                    delete *it;
                }
                delete PluginParameterPtr;
                delete [] path;

                if(opened)
                {
                    pluginGUI->setAAXViewContainer(GetViewContainer());
                    float width, height = 0.f;

                    pluginGUI->getSize(width, height);
                    guiWidth = (int)width;
                    guiHeight = (int)height;

                    pluginGUI->setGUIWindowFrame((IGUIWindowFrame*)this);
                }
            }
        }
    }
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::Draw()
//
/**
 \brief unused

*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginGUI::Draw(AAX_Rect* iDrawRect)
{
	return AAX_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::Draw()
//
/**
 \brief unused

 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginGUI::TimerWakeup(void)
{
	return AAX_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::ParameterUpdated()
//
/**
 \brief part of massive parameter update callback loop for the monolithic parameter; this
        will be called once the parameter has *actually* made it to the audio processing and
        keeps things looking tightly bound

 NOTES:
 - this creates the ASPiK-GUI object
 - slightly different between Windows and MacOS versions
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginGUI::ParameterUpdated(const char* iParameterID)
{
    AAX_IParameter* pParam = nullptr;
    GetEffectParameters()->GetParameter(iParameterID, &pParam);

    if(pParam && pluginGUI)
    {
        double controlValue = 0.0;
        pParam->GetValueAsDouble(&controlValue);
        int nTag = atoi(iParameterID) - 1;
        pluginGUI->updateGUIControlAAX(nTag, (float)controlValue);
        fprintf(stderr, "updateGUIControlsAAX: %f\n", controlValue);

        return AAX_SUCCESS;
    }

	return AAX_ERROR_INVALID_PARAMETER_ID;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::DeleteViewContainer()
//
/**
 \brief called when user closes GUI

 NOTES:
 - this destroys the ASPiK-GUI object
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginGUI::DeleteViewContainer()
{
    // --- destroy GUI
    if(pluginGUI)
    {
        // --- close it
        pluginGUI->close();

        // --- ref count should be exactly 1
        if(pluginGUI->getNbReference() != 1)
            AAX_ASSERT(0);

        // --- self-destruct
        VSTGUI::PluginGUI* oldGUI = pluginGUI;
        pluginGUI = 0;
        oldGUI->forget();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::GetViewSize()
//
/**
 \brief returns size of view container

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result AAXPluginGUI::GetViewSize(AAX_Point* oEffectViewSize) const
{
    oEffectViewSize->horz = (float)guiWidth;
    oEffectViewSize->vert = (float)guiHeight;

    return AAX_SUCCESS;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AAXPluginGUI::CreateViewContents()
//
/**
 \brief unused; will be removed in future

 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void AAXPluginGUI::CreateViewContents()
{
    // --- unused
}



