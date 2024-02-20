// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "coffscreencontext.h"
#include "cframe.h"
#include "cbitmap.h"
#include "platform/platformfactory.h"
#include "platform/iplatformgraphicsdevice.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (CBitmap* bitmap)
: CDrawContext (CRect (0, 0, bitmap->getWidth (), bitmap->getHeight ()))
, bitmap (bitmap)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const CRect& surfaceRect)
: CDrawContext (surfaceRect)
{
}

//-----------------------------------------------------------------------------
COffscreenContext::COffscreenContext (const PlatformGraphicsDeviceContextPtr device,
									  const CRect& surfaceRect,
									  const PlatformBitmapPtr& platformBitmap)
: CDrawContext (device, surfaceRect, platformBitmap->getScaleFactor ())
, bitmap (makeOwned<CBitmap> (platformBitmap))
{
}

//-----------------------------------------------------------------------------
void COffscreenContext::copyFrom (CDrawContext *pContext, CRect destRect, CPoint srcOffset)
{
	if (bitmap)
		bitmap->draw (pContext, destRect, srcOffset);
}

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (CFrame* frame, CCoord width, CCoord height, double scaleFactor)
{
	return create ({width, height}, scaleFactor);
}
#endif

//-----------------------------------------------------------------------------
SharedPointer<COffscreenContext> COffscreenContext::create (const CPoint& size, double scaleFactor)
{
	if (size.x >= 1. && size.y >= 1.)
	{
		if (auto graphicsDevice =
				getPlatformFactory ().getGraphicsDeviceFactory ().getDeviceForScreen (
					DefaultScreenIdentifier))
		{
			if (auto bitmap = getPlatformFactory ().createBitmap (size * scaleFactor))
			{
				bitmap->setScaleFactor (scaleFactor);
				if (auto context = graphicsDevice->createBitmapContext (bitmap))
				{
					CRect surfaceRect (CPoint (), size * scaleFactor);
					return makeOwned<COffscreenContext> (context, surfaceRect, bitmap);
				}
			}
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getWidth () const
{
	return bitmap ? bitmap->getWidth () : 0.;
}

//-----------------------------------------------------------------------------
CCoord COffscreenContext::getHeight () const
{
	return bitmap ? bitmap->getHeight () : 0.;
}

//-----------------------------------------------------------------------------
SharedPointer<CBitmap> renderBitmapOffscreen (
    const CPoint& size, double scaleFactor,
    const std::function<void (CDrawContext& drawContext)> drawCallback)
{
	auto context = COffscreenContext::create (size, scaleFactor);
	if (!context)
		return nullptr;
	context->beginDraw ();
	drawCallback (*context);
	context->endDraw ();
	return shared (context->getBitmap ());
}

} // VSTGUI
