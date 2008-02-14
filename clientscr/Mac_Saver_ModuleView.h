// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

//  Mac_Saver_ModuleView.h
//  BOINC_Saver_Module
//

#import <ScreenSaver/ScreenSaver.h>


@interface BOINC_Saver_ModuleView : ScreenSaverView 
{
    NSQuickDrawView * mainQDView;
    NSQuickDrawView * previewQDView;
    NSString *mBundleID;                        // our bundle ID
    
    IBOutlet id mConfigureSheet;		// our configuration sheet
    IBOutlet NSButton *mGoToBlankCheckbox;
    IBOutlet NSTextField *mBlankingTimeTextField;
    
    int mVersion;               // the version of our prefs
    NSString *mBlankingTimeString;
}

- (IBAction)closeSheetSave:(id) sender;
- (IBAction)closeSheetCancel:(id) sender;

@end

void setFrame(Rect *frame);
int initBOINCSaver(Boolean ispreview);
int drawGraphics(char **theMessage);
void drawPreview(GrafPtr aPort);
void closeBOINCSaver(void);
void displayText(const unsigned char *s, GrafPtr aPort);
extern void print_to_log_file(const char *format, ...);
