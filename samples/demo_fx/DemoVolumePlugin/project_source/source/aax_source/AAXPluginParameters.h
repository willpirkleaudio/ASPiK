// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxpluginparameters.h
//
/**
    \file   aaxpluginparameters.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  interface file for the AAXPluginParameters object; this code is
    		based heavily off of the monolithic plugin example in the AAX SDK
    		but custom fit to include the ASPiK kernel objects in a tight
    		synchronization.

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#pragma once
#ifndef AAX_PARAMETERS_H
#define AAX_PARAMETERS_H

#include "AAX_CEffectParameters.h"
#pragma warning(disable : 4985)	// --- 'ceil': attributes not present on previous declaration NOTE: for VS2008 only, see the google for more info

#include "AAX_CAtomicQueue.h"
#include "AAX_IParameter.h"
#include "AAX_IMIDINode.h"
#include "AAX_IString.h"
#include "AAX_IEffectDescriptor.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IPropertyMap.h"

#include <set>
#include <list>
#include <utility>
#include <vector>

// --- our plugin core object
#include "plugincore.h"
#include "customcontrols.h"

#if MAC
#include <CoreFoundation/CoreFoundation.h>
#else
#include <windows.h>
#endif

// --- see AAX_ITransport.h before enabling this: performance hits!
// #define ENABLE_EXTRA_HOST_INFO

// --- constants
const AAX_CTypeID PLUGIN_CUSTOMDATA_ID = 0; ///< custom data parameter number (we only need one)

// --- special ProTools meter
const AAX_CTypeID GR_MeterID = 'grMT';  ///< pro tools gr meter id
const unsigned int meterTapCount = 1;   ///< number of gr meters (we've only ever seen one on any AVID surface/SW)

// --- setup context struct
class AAXPluginParameters;
class GUIPluginConnector;
class PluginHostConnector;
class AAXMIDIEventQueue;

/**
 \struct pluginCustomData
 \ingroup AAX-Shell
 \brief
 Structure of data that is passed to GUI object once at creation time.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
struct pluginCustomData
{
    GUIPluginConnector* guiPlugin_Connector = nullptr; // added nullptr 5.9.18
    PluginCore* plugin_Core = nullptr;
};

/**
 \struct pluginPrivateData
 \ingroup AAX-Shell
 \brief
 Back-pointer to the parameters; this is described in detail in Designing Audio Effects in C++ 2nd Ed. by Will Pirkle
 and is part of the monolithic parameters AAX programming paradigm; this is very well documented in the
 AAX SDK documentation.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
struct pluginPrivateData
{
	AAXPluginParameters* aaxPluginParameters;
};

/**
 \struct AAXAlgorithm
 \ingroup AAX-Shell
 \brief
 Processing structure; this is described in detail in Designing Audio Effects in C++ 2nd Ed. by Will Pirkle
 and is part of the monolithic parameters AAX programming paradigm; this is very well documented in the
 AAX SDK documentation.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
// --- processing struct
struct AAXAlgorithm
{
    // --- audio
    pluginPrivateData*	privateData;			// --- the Monolithic Parameters
    float**				inputBufferPtrs;		// --- inputs buffer
    float**				outputBufferPtrs;		// --- outputs buffer
    int32_t*			bufferLength;			// --- buffer size (per channel)
#ifdef PT_GR_METER
    float**             grMeterPtrs;			// --- Meter taps; currrently support one GR meter (have never seen more than 1)
#endif
    
#ifdef WANT_SIDECHAIN
    int32_t*			sidechainChannel;		// --- sidechain channel pointer
#endif
    
    // --- MIDI
    AAX_IMIDINode*		midiInputNode;             // --- MIDI input node -> plugin
    AAX_IMIDINode*      midiTransportNode;         // --- for getting info about the host BPM, etc...
    
    // --- params
    int64_t*			currentStateNum;		// --- state value
};


#define kMaxAdditionalMIDINodes 15
#define kMaxAuxOutputStems 32

// --- NOTE: default here is 64 parameters; you should tweak this value to your final parameter count
//           for the smallest size plugin; undetermined if this is more efficient or not.
/**
 @kSynchronizedParameterQueueSize
 \ingroup AAX-Shell

 @brief This is the maximum size of the plugin-core's parameter list; make sure to adjust itg
 if your core needs more than 64 slots; part of the monolithic parameters AAX programming paradigm;
 this is very well documented in the AAX SDK documentation.
*/
#define kSynchronizedParameterQueueSize 64

/**
 @TParamValPair
 \ingroup AAX-Shell

 @brief Defines a parameter-value pair for the monolithic parameters AAX programming paradigm;
        this is very well documented in the AAX SDK documentation.
*/
typedef std::pair<AAX_CParamID const, const AAX_IParameterValue*> TParamValPair;

/**
 \class AAXPluginParameters
 \ingroup AAX-Shell
 \brief
 The AAXPluginParameters object implements the monolithic parameters AAX plugin programming paradigm which
 is documented in detail in Designing Audio Effects in C++ 2nd Ed. by Will Pirkle as well as the AAX SDK

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class AAXPluginParameters : public AAX_CEffectParameters
{
public:
	AAXPluginParameters ();
    virtual ~AAXPluginParameters();

	/** creation function */
	static AAX_CEffectParameters* AAX_CALLBACK Create();

	/** grab param changes and add to defer list */
	virtual AAX_Result UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource );

    /** see AAX_CMonolithicParameters in SDK */
	void AddSynchronizedParameter(const AAX_IParameter& inParameter);

public:
	// --- Overrides from AAX_CEffectParameters
	virtual AAX_Result EffectInit() AAX_OVERRIDE;                ///< AAX Override
	virtual AAX_Result ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t iDataSize) const AAX_OVERRIDE;///< AAX Override
	virtual AAX_Result GenerateCoefficients() AAX_OVERRIDE;      ///< AAX Override
	virtual AAX_Result TimerWakeup() AAX_OVERRIDE;               ///< AAX Override
    AAX_Result GetParameterNormalizedValue (AAX_CParamID iParameterID, double * oValuePtr )  const AAX_OVERRIDE;
    
	/** static render function for processing; can not be distributed */
	static void AAX_CALLBACK StaticRenderAudio(AAXAlgorithm* const inInstancesBegin[], const void* inInstancesEnd);

    /** static function for description process; can not be distributed */
    static AAX_Result StaticDescribe(AAX_IComponentDescriptor& outDesc);

	/** mechism to transfer custom data to GUI (AAX approved) */
	virtual AAX_Result GetCustomData(AAX_CTypeID iDataBlockID, uint32_t iDataSize, void* oData, uint32_t* oDataWritten) const AAX_OVERRIDE;

    /** get info from host for this buffer process cycle */
    void updateHostInfo(AAXAlgorithm* ioRenderInfo, HostInfo* hostInfo);

    /** the audio processing function for AAX */
	void ProcessAudio(AAXAlgorithm* ioRenderInfo, const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);

	/**  update dirty parameters */
	void UpdatePluginParameters(const TParamValPair* inSynchronizedParamValues[], int32_t inNumSynchronizedParamValues);

    /** outbound parameter set meter info for GUI update */
    void updateOutboundAAXParameters();

    /**
     \struct SParamValList
     \ingroup AAX-Shell
     \brief
      See AAX_CMonolithicParameters in SDK; this is part of the strict parameter synchronization in monolithic AAX plugins.

     \author Will Pirkle http://www.willpirkle.com
     \remark This object is included ASPiK
     \version Revision : 1.0
     \date Date : 2018 / 09 / 7
    */
	struct SParamValList
	{
		// Using 4x the preset queue size: the buffer must be large enough to accommodate the maximum
		// number of updates that we expect to be queued between/before executions of the render callback.
		// The maximum queuing that will likely ever occur is during a preset change (i.e. a call to
		// SetChunk()), in which updates to all parameters may be queued in the same state frame. It is
		// possible that the host would call SetChunk() on the plug-in more than once before the render
		// callback executes, but probably not more than 2-3x. Therefore 4x seems like a safe upper limit
		// for the capacity of this buffer.
		static const int32_t sCap = 4*kSynchronizedParameterQueueSize;

		TParamValPair* mElem[sCap];
		int32_t mSize;

		SParamValList()
		{
			Clear();
		}

		void Add(TParamValPair* inElem)
		{
			AAX_ASSERT(sCap > mSize);
			if (sCap > mSize)
			{
				mElem[mSize++] = inElem;
			}
		}

		void Append(const SParamValList& inOther)
		{
			AAX_ASSERT(sCap >= mSize + inOther.mSize);
			for (int32_t i = 0; i < inOther.mSize; ++i)
			{
				Add(inOther.mElem[i]);
			}
		}

		void Append(const std::list<TParamValPair*>& inOther)
		{
			AAX_ASSERT(sCap >= mSize + (int64_t)inOther.size());
			for (std::list<TParamValPair*>::const_iterator iter = inOther.begin(); iter != inOther.end(); ++iter)
			{
				Add(*iter);
			}
		}

		void Merge(AAX_IPointerQueue<TParamValPair>& inOther)
		{
			do
			{
				TParamValPair* const val = inOther.Pop();
				if (NULL == val) { break; }
				Add(val);
			} while (1);
		}

		void Clear() ///< Zeroes out the mElem array; does not destroy any elements
		{
			std::memset(mElem, 0x0, sizeof(mElem));
			mSize = 0;
		}
	};

private:
	// --- see AAX_CMonolithicParameters in SDK
    //     these are identically declared and used with the AAX_CMonolithicParameters sample code
    //     this is the AVID method of synchronizing the playback-head to the controls
	typedef std::set<const AAX_IParameter*> TParamSet;                              ///< dirty parameters
	typedef std::pair<int64_t, std::list<TParamValPair*> > TNumberedParamStateList; ///< state list
	typedef AAX_CAtomicQueue<TNumberedParamStateList, 256> TNumberedStateListQueue; ///< atomic queue for params

	typedef AAX_CAtomicQueue<const TParamValPair, 16*kSynchronizedParameterQueueSize> TParamValPairQueue; ///< queue: NOTE: kSynchronizedParameterQueueSize = 64; see note about increasing this for plugin cores with more than 64 parameters!

	SParamValList GetUpdatesForState(int64_t inTargetStateNum);     ///< monolithic parameters clone
	void DeleteUsedParameterChanges();                              ///< monolithic parameters clone
 	std::set<std::string> mSynchronizedParameters;                  ///< monolithic parameters clone
	int64_t mStateCounter;                                          ///< monolithic parameters clone
	TParamSet mDirtyParameters;                                     ///< monolithic parameters clone
	TNumberedStateListQueue mQueuedParameterChanges;                ///< monolithic parameters clone
	TNumberedStateListQueue mFinishedParameterChanges;              ///< monolithic parameters clone Parameter changes ready for deletion
	TParamValPairQueue mFinishedParameterValues;                    ///< monolithic parameters clone Parameter values ready for deletion
	int64_t mCurrentStateNum;                                       ///< monolithic parameters clone

    // --- soft bypass flag
    bool softBypass = false; ///< bypass

    // --- plugin core and interfaces
    PluginCore* pluginCore = nullptr;                   ///< ASPiK core
    GUIPluginConnector* guiPluginConnector = nullptr;   ///< GUI Plugin interface
    PluginHostConnector* pluginHostConnector = nullptr; ///< Plugin Host interface
    AAXMIDIEventQueue* midiEventQueue = nullptr;        ///< double-buffered-queue for MIDI messaging
    AAX_CParameterManager mMeterParameterManager;
    
    AAX_Result SetMeterParameterNormalizedValue (AAX_CParamID iParameterID, double aValue)
    {
        AAX_IParameter* parameter = mMeterParameterManager.GetParameterByID(iParameterID);
        if (parameter == 0)
            return AAX_ERROR_INVALID_PARAMETER_ID;
        
        parameter->SetNormalizedValue ( aValue );
        return AAX_SUCCESS;
    }
    
    /** helper for channel format identification; converts AAX_EStemFormat into ASPiK channel format enumeration */
    uint32_t getChannelFormatForAAXStemFormat(AAX_EStemFormat format)
    {
        switch(format)
        {
            case AAX_eStemFormat_None: {
                return kCFNone; }

            case AAX_eStemFormat_Mono: {
                return kCFMono; }

            case AAX_eStemFormat_Stereo: {
                return kCFStereo; }

            case AAX_eStemFormat_LCR: {
                return kCFLCR; }

            case AAX_eStemFormat_LCRS: {
                return kCFLCRS; }

            case AAX_eStemFormat_Quad: {
                return kCFQuad; }

            case AAX_eStemFormat_5_0: {
                return kCF5p0; }

            case AAX_eStemFormat_5_1: {
                return kCF5p1; }

            case AAX_eStemFormat_6_0: {
                return kCF6p0; }

            case AAX_eStemFormat_6_1: {
                return kCF6p1; }

            case AAX_eStemFormat_7_0_SDDS: {
                return kCF7p0Sony; }

            case AAX_eStemFormat_7_0_DTS: {
                return kCF7p0DTS; }

            case AAX_eStemFormat_7_1_SDDS: {
                return kCF7p1Sony; }

            case AAX_eStemFormat_7_1_DTS: {
                return kCF7p1DTS; }

            case AAX_eStemFormat_7_1_2: {
                return kCF7p1Proximity; }

            default: {
                return kCFNone; }
        }
        return kCFNone;
    }

#if defined _WINDOWS || defined _WINDLL
    wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
    {
        wchar_t* wString = new wchar_t[4096];
        MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
        return wString;
    }
    // --- getMyDLLDirectory()
    //     returns the directory where the .component resides
    char* getMyDLLDirectory(const char* dllName)
    {
        wchar_t* cPluginName = convertCharArrayToLPCWSTR(dllName);
        HMODULE hmodule = GetModuleHandle(cPluginName);
        
        TCHAR dir[MAX_PATH];
        memset(&dir[0], 0, MAX_PATH*sizeof(TCHAR));
        dir[MAX_PATH-1] = '\0';
        
        if(hmodule)
            GetModuleFileName(hmodule, &dir[0], MAX_PATH);
        else
            return nullptr;
        
        // --- use string tools
        std::wstring backslash(L"\\");
        std::wstring strPlugin(cPluginName);
        std::wstring strDir(&dir[0]);
        int pathLen = strDir.size() - strPlugin.size() - backslash.size();
        if (pathLen > 0)
        {
            std::wstring strPath = strDir.substr(0, pathLen);
            char* str = new char[MAX_PATH];
            sprintf(str, "%ls", strPath.c_str());
            delete[] cPluginName;
            
            return str; // dllPath.c_str();
        }
        return "";
    }
    
#else
    char* getMyComponentDirectory(CFStringRef bundleID)
    {
        if (bundleID != nullptr)
        {
            CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
            if(helixBundle != nullptr)
            {
                CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
                if(bundleURL != nullptr)
                {
                    CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(nullptr, bundleURL);
                    
                    CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                    CFRelease(componentFolderPathURL);
                    
                    if(myComponentPath != nullptr)
                    {
                        int nSize = CFStringGetLength(myComponentPath);
                        char* path = new char[nSize+1];
                        memset(path, 0, (nSize+1)*sizeof(char));
                        
                        bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                        CFRelease(myComponentPath);
                        
                        if(success) return path;
                        else return nullptr;
                    }
                    CFRelease(bundleURL);
                }
            }
            CFRelease(bundleID);
        }
        return nullptr;
    }
#endif
};

/**
 \class PluginHostConnector
 \ingroup AU-Shell
 \brief
 The PluginHostConnector implements the IPluginHostConnector interface for the plugin shell object. For AAX, this requires implementing only one method, sendHostMessage( ). Only one message is processed for sendGUIUpdate that provides a mechanism to update the GUI controls from the plugin core. Note that this is not an ideal solution for most problems (e.g. linking controls together intelligently) -- you should always consider using a custom sub-controller and/or custom view to do this properly. On occasion, a more difficult scenario may arise (e.g. MIDI learn button, that must wait for user input from a MIDI instrument to toggle states) where the sendGUIUpdate method may be appropriate. See the example in the ASPiK SDK for more information.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class PluginHostConnector : public IPluginHostConnector
{
public:
    PluginHostConnector(AAX_IEffectParameters* _aaxParameters) {aaxParameters = _aaxParameters;}
    virtual ~PluginHostConnector(){}

    /**
     \brief process a message; by default it processes sendGUIUpdate to safely send a parameter change event but you can add your own custom message processing here (be careful)

     \param hostMessageInfo custom structure with message information; for sendGUIUpdate the message data arrives in the hostMessageInfo.guiUpdateData list; the function iterates over the list and sets AAX parameters with a thread-safe function specifically for external set-operations like this.
    */
    virtual void sendHostMessage(const HostMessageInfo& hostMessageInfo)
    {
        switch(hostMessageInfo.hostMessage)
        {
            case sendGUIUpdate:
            {
                GUIUpdateData guiUpdateData = hostMessageInfo.guiUpdateData;

				for (uint32_t i = 0; i < guiUpdateData.guiParameters.size(); i++)
                {
                    GUIParameter guiParam = guiUpdateData.guiParameters[i];

                    std::stringstream str;
                    str << guiParam.controlID + 1;
                    AAX_IParameter* oParameter;
                    AAX_Result result = aaxParameters->GetParameter(str.str().c_str(), &oParameter);
                    if(AAX_SUCCESS == result)
                    {
                        oParameter->SetValueWithDouble(guiParam.actualValue);
                    }
                }

                // --- clean up
				for (uint32_t i = 0; i < guiUpdateData.guiParameters.size(); i++)
                    guiUpdateData.guiParameters.pop_back();

                break;
            }
            default:
                break;
        }
    }

protected:
    AAX_IEffectParameters* aaxParameters; ///< parent parameters; lifelong existence
};


/**
 \class CustomViewController
 \ingroup AAX-Shell
 \brief
 The CustomViewController is part of the safe ICustomView feature in ASPiK. The CustomViewController maintains an ICustomView pointer. When the GUI registers and de-registers custom views during creation or destuction, the plugin shell is responsible for making sure that the original ICustomView pointer registered with the plugin core object *never goes out of scope* and this object is part of that system.

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
class CustomViewController : public ICustomView
{
public:
    /** constructor: pass the newly registered ICustomView* */
    CustomViewController(ICustomView* _customViewIF) { customViewIF = _customViewIF; }
    virtual ~CustomViewController() {}

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void updateView()
    {
        if (customViewIF)
            customViewIF->updateView();
    }

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void pushDataValue(double data)
    {
        if (customViewIF)
            customViewIF->pushDataValue(data);
    }

    /** forward call to ICustomView interface *if it is still alive* */
    virtual void sendMessage(void* data)
    {
        if (customViewIF)
            customViewIF->sendMessage(data);
    }

    /** set a NEW ICustomView* -- this is called when a custom view is registered */
    void setCustomViewPtr(ICustomView* _customViewIF) { customViewIF = _customViewIF; }

    /** get the current ICustomView* -- note this is read-only (const) and it also MAY be NULL */
    const ICustomView* getCustomViewPtr() { return customViewIF; }

    /** clear the stored ICustomView* -- this is called when a custom view is de-registered */
    void clearCustomViewPtr() { customViewIF = nullptr; }


private:
    ICustomView* customViewIF = nullptr; ///< the currently active ICustomView interface, registered from the GUI
};

/**
 \class GUIPluginConnector
 \ingroup AAX-Shell
 \brief
 The GUIPluginConnector interface creates a safe message mechanism for the GUI to issue requests to the plugin shell. The following messages are processed via functions:

 - tell core that a parameter has changed (asynchronous)
 - set the AAX normalized parameter value (thread-safe)
 - get the AAX parameter value in normalized form (thread-safe)
 - register and de-register sub-controllers (normally not used, but here for future expansion)
 - register and de-register custom-views
 - send plugin core messages about current GUI states (did open, will close, timer ping)

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
class GUIPluginConnector : public IGUIPluginConnector
{
public:
    /** construct with two pointers */
    GUIPluginConnector(AAXPluginParameters* _aaxParameters, PluginCore* _pluginCore){pluginCore = _pluginCore; aaxParameters = _aaxParameters;}

    /** destroy subcontroller and custom view container objects */
    virtual ~GUIPluginConnector()
    {
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            delete it->second;
        }
        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            delete it->second;
        }
    }

    /** inform core that parameter has changed (NOT to be used to alter plugin states!) */
    virtual void parameterChanged(int32_t controlID, double actualValue, double /*normalizedValue*/)
    {
        if(pluginCore)
            pluginCore->guiParameterChanged(controlID, actualValue);
    }

    /** get AAX parameter in normalized form */
    virtual double getNormalizedPluginParameter(int32_t controlID)
    {
        //if(aaxParameters->)
        // --- NOTE: this is the proper way for the GUI to get parameter values
        std::stringstream str;
        str << controlID + 1;
        double param = 0.0;
        AAX_Result result = aaxParameters->GetParameterNormalizedValue(str.str().c_str(), &param);
        if(AAX_SUCCESS == result)
            return param;

        return 0.0;
    }

    /** set AAX parameter in normalized form */
    virtual void setNormalizedPluginParameter(int32_t controlID, double value)
    {
        // --- NOTE: this is the proper way for the GUI to set parameter values
        std::stringstream str;
        str << controlID + 1;
        aaxParameters->SetParameterNormalizedValue(str.str().c_str(), value);
    }

    /** set start automation handler (touch) */
    virtual void beginParameterChangeGesture(int controlTag)
    {
        if(aaxParameters)
        {
            std::stringstream str;
            str << controlTag+1;
            aaxParameters->TouchParameter(str.str().c_str());
        }
    }

    /** set stop automation handler (release) */
    virtual void endParameterChangeGesture(int controlTag)
    {
        if(aaxParameters )
        {
            std::stringstream str;
            str << controlTag+1;
            aaxParameters->ReleaseParameter(str.str().c_str());
        }
    }

    /** store sub-controller's ICustomView and send message to core */
    virtual bool registerSubcontroller(std::string subcontrollerName, ICustomView* customViewConnector)
    {
        // --- do we have this in our map already?
        CustomViewController* pCVC = nullptr;

        customViewControllerMap::const_iterator it = customSubControllerMap.find(subcontrollerName);
        if (it != customSubControllerMap.end())
        {
            pCVC = it->second;
            pCVC->setCustomViewPtr(customViewConnector);
        }
        else
        {
            pCVC = new CustomViewController(customViewConnector);
            customSubControllerMap.insert(std::make_pair(subcontrollerName, pCVC));
        }

        MessageInfo info(PLUGINGUI_REGISTER_SUBCONTROLLER);
        info.inMessageString = subcontrollerName;
        info.inMessageData = pCVC;

        if (pluginCore && pluginCore->processMessage(info))
            return true;

        return false;
    }

    /** remove sub-controller's ICustomView and send message to core */
    virtual bool deRregisterSubcontroller(ICustomView* customViewConnector)
    {
        CustomViewController* pCVC = getCustomSubController(customViewConnector);
        if (pCVC)
        {
            pCVC->clearCustomViewPtr();

            MessageInfo info(PLUGINGUI_DE_REGISTER_SUBCONTROLLER);
            info.inMessageString = "";
            info.inMessageData = pCVC;

            if (pluginCore && pluginCore->processMessage(info))
                return true;
        }

        return false;
    }

    /** store custom view's ICustomView and send message to core */
    virtual bool registerCustomView(std::string customViewName, ICustomView* customViewConnector)
    {
        // --- do we have this in our map already?
        CustomViewController* pCVC = nullptr;

        customViewControllerMap::const_iterator it = customViewMap.find(customViewName);
        if (it != customViewMap.end())
        {
            pCVC = it->second;
            pCVC->setCustomViewPtr(customViewConnector);
        }
        else
        {
            pCVC = new CustomViewController(customViewConnector);
            customViewMap.insert(std::make_pair(customViewName, pCVC));
        }

        MessageInfo info(PLUGINGUI_REGISTER_CUSTOMVIEW);
        info.inMessageString = customViewName;
        info.inMessageData = pCVC;

        if(pluginCore && pluginCore->processMessage(info))
            return true;

        return false;
    }

    /** remove custom view's ICustomView and send message to core */
    virtual bool deRegisterCustomView(ICustomView* customViewConnector)
    {
        CustomViewController* pCVC = getCustomViewController(customViewConnector);
        if (pCVC)
        {
            // --- clear it
            pCVC->clearCustomViewPtr();

            MessageInfo info(PLUGINGUI_DE_REGISTER_CUSTOMVIEW);
            info.inMessageString = "";
            info.inMessageData = pCVC;

            if (pluginCore && pluginCore->processMessage(info))
                return true;
        }

        return false;
    }

    /** send message to core: GUI has been opened and all VISIBLE controls are active */
    virtual bool guiDidOpen()
    {
        if(!pluginCore) return false;
        MessageInfo info(PLUGINGUI_DIDOPEN);
        return pluginCore->processMessage(info);
    }

    /** send message to core: GUI will close; all controls active but will vanish */
    virtual bool guiWillClose()
    {
        if(!pluginCore) return false;

        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            it->second->clearCustomViewPtr();
        }
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            it->second->clearCustomViewPtr();
        }

        MessageInfo info(PLUGINGUI_WILLCLOSE);
        return pluginCore->processMessage(info);
    }

    /** send message to core: repaint timer */
    virtual bool guiTimerPing()
    {
        if(!pluginCore) return false;
        MessageInfo info(PLUGINGUI_TIMERPING);
        return pluginCore->processMessage(info);
    }

    /** send message to core: a non-bound variable has changed (NOT implemented or needed, but may be added for your own customization) */
    virtual bool checkNonBoundValueChange(int tag, float normalizedValue)
    {
        if(!pluginCore) return false;

        // --- do any additional stuff here
        // --- dispatch non-bound value changes directly to receiver

        return false;
    }

protected:
    PluginCore* pluginCore = nullptr;                   ///< the core object
//    AAX_IEffectParameters* aaxParameters = nullptr;     ///< the parent object
    AAXPluginParameters* aaxParameters = nullptr;     ///< the parent object

    // --- this is for supporting the persistent interface pointer for the core object
    //     and is required by ASPiK Specifications
    typedef std::map<std::string, CustomViewController*> customViewControllerMap;   ///< map of custom view controllers
    customViewControllerMap customViewMap;

    /** get a custom view controller object (internal use only) */
    CustomViewController* getCustomViewController(ICustomView* customViewConnector)
    {
        for (customViewControllerMap::const_iterator it = customViewMap.begin(), end = customViewMap.end(); it != end; ++it)
        {
            if (it->second->getCustomViewPtr() == customViewConnector)
                return it->second;
        }

        return nullptr;
    }

    customViewControllerMap customSubControllerMap;                                 ///< map of custom sub-controllers
    /** get a sub-controller controller object (internal use only) */
    CustomViewController* getCustomSubController(ICustomView* customViewConnector)
    {
        for (customViewControllerMap::const_iterator it = customSubControllerMap.begin(), end = customSubControllerMap.end(); it != end; ++it)
        {
            if (it->second->getCustomViewPtr() == customViewConnector)
                return it->second;
        }

        return nullptr;
    }
};

/**
 \class AAXMIDIEventQueue
 \ingroup AAX-Shell
 \brief
 The AAXMIDIEventQueue interface queues incoming MIDI messages and blasts them out during the buffer processing phase.

 NOTES:
 - this is a simple interface because the MIDI information is already pre-packaged in queue format with
   time-stamped MIDI events
 - this just wraps the midi events into the ASPiK IMidiEventQueue

 \author Will Pirkle http://www.willpirkle.com
 \remark This object is included in ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
class AAXMIDIEventQueue : public IMidiEventQueue
{
public:
    /** consruct with a core pointer */
    AAXMIDIEventQueue(PluginCore* _pluginCore)
    : ioPacketPtr(0),
    midiBuffersize(0)
    {
        pluginCore = _pluginCore;
    };

    virtual ~AAXMIDIEventQueue(){}

    /** send a new batch of MIDI packets into the object */
    void setMIDIpackets(const AAX_CMidiPacket*& _ioPacketPtr, uint32_t& _midiBuffersize)
    {
        ioPacketPtr = _ioPacketPtr;
        midiBuffersize = _midiBuffersize;

    }

    /** event count for this bunch */
    virtual unsigned int getEventCount()
    {
        return midiBuffersize;
    }

    /** send MIDI event with this sample offset to the core for processing */
    virtual bool fireMidiEvents(unsigned int sampleOffset)
    {
        while( (midiBuffersize > 0) && (NULL != ioPacketPtr) && ((ioPacketPtr->mTimestamp <= sampleOffset)))
        {
            const uint8_t uMessage = (ioPacketPtr->mData[0] & 0xF0);	// message
            const uint8_t uChannel = (ioPacketPtr->mData[0] & 0x0F);	// channel

            midiEvent event;
            event.midiMessage = (unsigned int)uMessage;
            event.midiChannel = (unsigned int)uChannel;
            event.midiData1 = (unsigned int)ioPacketPtr->mData[1];
            event.midiData2 = (unsigned int)ioPacketPtr->mData[2];
            event.midiSampleOffset = sampleOffset;

            // --- send to core for processing
            if(pluginCore)
                pluginCore->processMIDIEvent(event);

            ++ioPacketPtr;
            --midiBuffersize;
        }
        return true;
    }

protected:
    PluginCore* pluginCore = nullptr;       ///< core
    const AAX_CMidiPacket* ioPacketPtr;     ///< array of packets
    uint32_t midiBuffersize = 0;            ///< midi buffer size for each bunch of packets
};

#endif

