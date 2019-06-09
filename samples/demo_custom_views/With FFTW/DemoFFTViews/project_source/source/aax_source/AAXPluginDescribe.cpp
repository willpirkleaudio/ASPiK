// -----------------------------------------------------------------------------
//    ASPiK AAX Shell File:  aaxplugindescribe.cpp
//
/**
    \file   aaxplugindescribe.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  static description file for the monolithic plugin paradigm; note that
    		these functions are not part of any object

			- included in ASPiK(TM) AAX Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AAX Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "AAXPluginDescribe.h"
#include "AAXPluginGUI.h"

#include "AAX_ICollection.h"
#include "AAX_IComponentDescriptor.h"
#include "AAX_IEffectDescriptor.h"

#include "AAX_IPropertyMap.h"
#include "AAX_Errors.h"
#include "AAX_Assert.h"

#include "channelformats.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	DescribeAlgComponent
//
/**
 \brief decodes supported channel I/O combinations and replies if we support them

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DescribeAlgComponent(AAX_EStemFormat inStemFormat, AAX_IComponentDescriptor& outDesc, AAX_EStemFormat outStemFormat, PluginCore* plugin)
{
    AAX_Result err = AAX_SUCCESS;

	// --- setup properties
	AAX_IPropertyMap* properties = outDesc.NewPropertyMap();
	AAX_ASSERT (properties);
    if ( !properties ) return;

    if ( !plugin ) return;

	properties->AddProperty(AAX_eProperty_ManufacturerID, plugin->getAAXManufacturerID());
	properties->AddProperty(AAX_eProperty_ProductID, plugin->getAAXProductID());
	properties->AddProperty(AAX_eProperty_InputStemFormat, inStemFormat);
	properties->AddProperty(AAX_eProperty_OutputStemFormat, outStemFormat);
	properties->AddProperty(AAX_eProperty_CanBypass, true);

    if(!plugin->hasCustomGUI())
        properties->AddProperty(AAX_eProperty_UsesClientGUI, true); 	// Register for auto-GUI

    // --- currently support one mixed I/O combo: mono-in, stereo-out
    //     otherwise, input Format = output Format
    //     if you need to add some strange I/O combo, make your own ID and add here
    if(inStemFormat == AAX_eStemFormat_Mono && outStemFormat == AAX_eStemFormat_Stereo)
    {
        // --- mono -> stereo
        err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_MonoIn_StereoOut );
        err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
        err = AAXPluginParameters::StaticDescribe(outDesc);
        AAX_ASSERT(err == AAX_SUCCESS);
    }
    else
    {
        switch( inStemFormat )
        {
            case AAX_eStemFormat_Mono:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_Mono );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_Stereo:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_Stereo );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_LCR:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_LCR );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_LCRS:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_LCRS );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_Quad:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_QUAD );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_5_0:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_5_0 );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_5_1:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_5_1 );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_6_0:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_6_0 );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_6_1:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_6_1 );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_7_0_SDDS:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_7_0_SDDS );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_7_1_SDDS:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_7_1_SDDS );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_7_0_DTS:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_7_0_DTS );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_7_1_DTS:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_7_1_DTS );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            case AAX_eStemFormat_7_1_2:
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_7_1_2 );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
            default: // stereo I/O
            {
                err = properties->AddProperty ( AAX_eProperty_PlugInID_Native, kPluginCore_PlugInID_Native_Stereo );
                err = outDesc.AddProcessProc_Native(AAXPluginParameters::StaticRenderAudio, properties);
                err = AAXPluginParameters::StaticDescribe(outDesc);
                AAX_ASSERT (err == AAX_SUCCESS);
                break;
            }
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	GetASPiKPlugInDescription
//
/**
 \brief description of options such as pro tools gain reduction meter, plugin names, and the
        GUI creation function name; part of early description/registration

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result GetASPiKPlugInDescription(AAX_IEffectDescriptor& outDescriptor, PluginCore* plugin)
{
	int	err = AAX_SUCCESS;
	AAX_IComponentDescriptor* compDesc = outDescriptor.NewComponentDescriptor ();
    if (!compDesc)
        return AAX_ERROR_NULL_OBJECT;
    if (!plugin)
        return AAX_ERROR_NULL_OBJECT;

 	// --- Effect identifiers
    outDescriptor.AddName(plugin->getPluginName());
	outDescriptor.AddName(plugin->getShortPluginName());

	// --- setup category
    if(plugin->getPluginType() == pluginType::kSynthPlugin)
        outDescriptor.AddCategory(AAX_ePlugInCategory_SWGenerators);
    else
        outDescriptor.AddCategory(plugin->getAAXPluginCategory());

    // --- setup channel algorithms
    for(uint32_t i = 0; i < plugin->getNumSupportedIOCombinations(); i++)
    {
        // --- AAX requires audio inputs for synths, which have no inputs
        AAX_EStemFormat inputFormat;
        if(plugin->getPluginType() == pluginType::kSynthPlugin)
            inputFormat = getAAXStemFormatForChannelFormat(plugin->getChannelOutputFormat(i));
        else
            inputFormat = getAAXStemFormatForChannelFormat(plugin->getChannelInputFormat(i));
       
        AAX_EStemFormat outputFormat = getAAXStemFormatForChannelFormat(plugin->getChannelOutputFormat(i));

        compDesc->Clear ();
        DescribeAlgComponent (inputFormat , *compDesc, outputFormat, plugin );
        err = outDescriptor.AddComponent ( compDesc );
        AAX_ASSERT (err == AAX_SUCCESS);
    }

	// --- Data model
	err = outDescriptor.AddProcPtr( (void *) AAXPluginParameters::Create, kAAX_ProcPtrID_Create_EffectParameters );
	AAX_ASSERT (err == AAX_SUCCESS);

    if(plugin->hasCustomGUI())
    {
		outDescriptor.AddProcPtr((void*)AAXPluginGUI::Create, kAAX_ProcPtrID_Create_EffectGUI);
		AAX_ASSERT (err == AAX_SUCCESS);
    }

#ifdef PT_GR_METER
    // --- meter display properties
    AAX_IPropertyMap* meterProperties = outDescriptor.NewPropertyMap();
    AAX_ASSERT (meterProperties);
    meterProperties->AddProperty ( AAX_eProperty_Meter_Type, AAX_eMeterType_CLGain );
    meterProperties->AddProperty ( AAX_eProperty_Meter_Orientation, AAX_eMeterOrientation_Default );
    outDescriptor.AddMeterDescription( GR_MeterID, "GRMeter", meterProperties );
#endif

	return AAX_SUCCESS;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	GetEffectDescriptions
//
/**
 \brief final part of description; includes manufacturer name, etc...

 NOTES:
 - see Designing Audio Effects in C++ 2nd Ed. by Will Pirkle for more information and an AAX Programming Guide
 - see AAX SDK for more information on this function and its parameters
 */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AAX_Result GetEffectDescriptions ( AAX_ICollection * outCollection )
{
	AAX_Result result = AAX_SUCCESS;

    // --- only for describing, not shared
    PluginCore* pluginCore = new PluginCore;
    if (!pluginCore)
        return AAX_ERROR_NULL_OBJECT;

	AAX_IEffectDescriptor* plugInDescriptor = outCollection->NewDescriptor();
	if ( plugInDescriptor )
	{
		result = GetASPiKPlugInDescription(*plugInDescriptor, pluginCore);
		if ( result == AAX_SUCCESS )
			outCollection->AddEffect ( pluginCore->getAAXEffectID(), plugInDescriptor );
	}
	else
	{
		result = AAX_ERROR_NULL_OBJECT;
	}

	outCollection->SetManufacturerName(pluginCore->getVendorName());
	outCollection->AddPackageName(pluginCore->getPluginName());
	outCollection->AddPackageName(pluginCore->getShortPluginName());
	outCollection->SetPackageVersion(1);

    delete pluginCore;

	return result;
}

