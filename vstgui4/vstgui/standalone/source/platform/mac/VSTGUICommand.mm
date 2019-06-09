// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#import "VSTGUICommand.h"

//------------------------------------------------------------------------
@implementation VSTGUICommand

- (const VSTGUI::Standalone::Detail::CommandWithKey&)command
{
	return self->_cmd;
}

@end
