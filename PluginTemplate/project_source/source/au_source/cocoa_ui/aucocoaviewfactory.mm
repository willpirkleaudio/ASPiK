// -----------------------------------------------------------------------------
//    ASPiK AU Shell File:  aucocoaviewfactory.mm
//
/**
    \file   aucocoaviewfactory.mm
    \author Will Pirkle
    \date   17-September-2018
    \brief  contains the frame code for the cocoa view used in AU plugins with 
    	    the ASPiK plugin framework.

		- included in ASPiK(TM) AU Plugin Shell
    		- Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
    		- see the book for detailed explanations of functions and an
    		  AU Programming Guide
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#import <Cocoa/Cocoa.h>
#import <AudioUnit/AUCocoaUIView.h>

#import "plugindescription.h"
class AUGUIWindowFrame;

/**
 \class AudioUnitNSView
 \ingroup AU-Shell
 \brief
 The cocoa NSView class for this plugin
 
 \author Will Pirkle http://www.willpirkle.com
 \remark designed by Will Pirkle for Tritone Syestems, Inc for ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
@interface AudioUnitNSView : NSView
{
    AudioUnit mAU;
    AUGUIWindowFrame* m_pGUIWindowFrame;
    bool isClosing;
}

/** init the GUI */
- (id) initWithVSTGUI:(AudioUnit) AU preferredSize:(NSSize)size;
- (IGUIWindowFrame*) getGUIFrame; ///< for resizing

@end

/**
 \class AU_COCOA_VIEWFACTORY_NAME
 \ingroup AU-Shell
 \brief
 The cocoa NSView factory (name is unique and defined in plugindefinition.h)
 
 \author Will Pirkle http://www.willpirkle.com
 \remark designed by Will Pirkle for Tritone Syestems, Inc for ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
 */
@interface AU_COCOA_VIEWFACTORY_NAME : NSObject <AUCocoaUIBase>
{
    IBOutlet AudioUnitNSView *uiFreshlyLoadedView;	// This data member needs to be the same class as the view class the factory will return
}

- (NSString *) description;	// string description of the view

@end

@implementation AU_COCOA_VIEWFACTORY_NAME

enum
{
	kOpenGUI = 64000,
    kCloseGUI
};

// version 0
- (unsigned) interfaceVersion {
	return 0;
}

// string description of the Cocoa UI
- (NSString *) description {
	return @"Cocoa UI for Audio Unit Plugin";
}

// N.B.: this class is simply a view-factory,
// returning a new autoreleased view each time it's called.
- (NSView *)uiViewForAudioUnit:(AudioUnit)inAU withSize:(NSSize)inPreferredSize
{
    // --- create the view
    uiFreshlyLoadedView = [[AudioUnitNSView alloc] initWithVSTGUI:inAU preferredSize:inPreferredSize];
    
    // --- the struct for communicating with shell
    VIEW_STRUCT viewStruct = {0};
    viewStruct.pWindow =(void*)uiFreshlyLoadedView;
    viewStruct.au = inAU;
    
    // --- set frame for plugin
    IGUIWindowFrame* pFrame = [uiFreshlyLoadedView getGUIFrame];
    viewStruct.pGUIFrame = pFrame;
    UInt32 size = sizeof(viewStruct);

    // --- open VSTGUI
	if(AudioUnitSetProperty(inAU, kOpenGUI, kAudioUnitScope_Global, 0, (void*)&viewStruct, size) != noErr)
		return nil;
    
    // --- size is returned in the struct
    NSRect newSize = NSMakeRect (0, 0, viewStruct.width, viewStruct.height);

    // --- set the new size
    [uiFreshlyLoadedView setFrame:newSize];
    NSView *returnView = uiFreshlyLoadedView;
    uiFreshlyLoadedView = nil;	// zero out pointer.  This is a view factory.  Once a view's been created
                                // and handed off, the factory keeps no record of it.
    
    return [returnView autorelease];
}

@end

//--------------------------------------------------------------------------------------------------------------
/**
 \class AUGUIWindowFrame
 \ingroup AU-Shell
 \brief
 The ASPiK IGUIWindowFrame object for AU plugin shell.
 
 \author Will Pirkle http://www.willpirkle.com
 \remark designed by Will Pirkle for Tritone Syestems, Inc for ASPiK
 \version Revision : 1.0
 \date Date : 2018 / 09 / 7
*/
//--------------------------------------------------------------------------------------------------------------
class AUGUIWindowFrame : public IGUIWindowFrame
{
public:
    AUGUIWindowFrame (NSView* parent) : parent (parent) {}
    virtual ~AUGUIWindowFrame (){}
    
    virtual bool setWindowFrameSize(double left = 0, double top = 0, double right = 0, double bottom = 0)
    {
        NSRect newSize = NSMakeRect ([parent frame].origin.x, [parent frame].origin.y, right - left, bottom - top);
        [parent setFrame:newSize];
        return true;
    }
    
    virtual bool getWindowFrameSize(double& left, double&  top, double&  right, double&  bottom)
    {
        left = [parent frame].origin.x;
        top = [parent frame].origin.y;
        right = [parent frame].origin.x + [parent frame].size.width;
        bottom = [parent frame].origin.y + [parent frame].size.height;
        return true;
    }
    
protected:
    NSView* parent;
    
};

@implementation AudioUnitNSView

- (id) initWithVSTGUI:(AudioUnit)AU preferredSize:(NSSize)size
{
    self = [super initWithFrame:NSMakeRect (0, 0, size.width, size.height)];
    m_pGUIWindowFrame = NULL;
 
    if(self)
    {
        m_pGUIWindowFrame = new AUGUIWindowFrame(self);
        mAU = AU;
        isClosing = false;
    }
    
    return self;
}

// --- use normal drawing coords
- (BOOL)isFlipped {
    return NO; }

// --- change size of frame
- (void) setFrame:(NSRect)newSize
{
    [super setFrameSize: newSize.size];
}

// --- this is a fix for the Studio One GUI display issue; appears to be DAW problem, not ASPiK problem
- (void)viewDidMoveToWindow;
{
    NSRect frameSize = [super frame];
    NSView* contentV = [[super window] contentView];
    NSRect selfRelativeToContent = [self convertRect:contentV.bounds toView:nil];
    
    // --- trapping this condition fixes the problem with Studio One
    //     12.22.19 -- found this condition identifies the error
    if(selfRelativeToContent.size.height - selfRelativeToContent.origin.y == frameSize.size.height)
    {
        NSPoint point;
        point.x = 0;
        point.y = selfRelativeToContent.origin.y;
        [super setFrameOrigin:point];
    }
}

- (void)willRemoveSubview:(NSView *)subview
{
    VIEW_STRUCT viewStruct;
    viewStruct.pWindow =(void*)self;
    UInt32 size = sizeof(viewStruct);
    
    // --- close VSTGUI editor
    if(!isClosing)
    {
        isClosing = true;

        if (AudioUnitSetProperty (mAU, kCloseGUI, kAudioUnitScope_Global, 0, (void*)&viewStruct, size) != noErr)
            return;
    }
}

- (IGUIWindowFrame*) getGUIFrame
{
    return m_pGUIWindowFrame;
}

- (void)dealloc
{
    if(m_pGUIWindowFrame)
        delete m_pGUIWindowFrame;
    m_pGUIWindowFrame = NULL;
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}


@end

