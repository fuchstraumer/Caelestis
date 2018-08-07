#include "dialog/Dialog.hpp"
#import <Cocoa/Cocoa.h>

NSString* const kOkStr = @"OK";
NSString* const kCancelStr = @"Cancel";
NSString* const kYesStr = @"Yes";
NSString* const kNoStr = @"No";
NSString* const kQuitStr = @"Quit";

NSAlertStyle getAlertStyle(uint32_t style) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
   switch (style) {
      case dialog_type::Info:
         return NSAlertStyleInformational;
      case dialog_type::Warning:
         return NSAlertStyleWarning;
      case dialog_type::Error:
         return NSAlertStyleCritical;
      case dialog_type::Question:
         return NSAlertStyleWarning;
      default:
         return NSAlertStyleInformational;
   }
#else
   switch (style) {
    case dialog_type::Info:
        return NSInformationalAlertStyle;
    case dialog_type::Warning:
        return NSWarningAlertStyle;
    case dialog_type::Error:
        return NSCriticalAlertStyle;
    case dialog_type::Question:
        return NSWarningAlertStyle;
    default:
        return NSInformationalAlertStyle;
   }
#endif
}

void setButtons(NSAlert *alert, uint32_t buttons) {
   switch (buttons) {
    case button_type::Ok:
        [alert addButtonWithTitle:kOkStr];
        break;
    case button_type::OkCancel:
        [alert addButtonWithTitle:kOkStr];
        [alert addButtonWithTitle:kCancelStr];
        break;
    case button_type::YesNo:
        [alert addButtonWithTitle:kYesStr];
        [alert addButtonWithTitle:kNoStr];
        break;
    case button_type::Abort:
        [alert addButtonWithTitle:kQuitStr];
        break;
    default:
        [alert addButtonWithTitle:kOkStr];
   }
}

uint32_t getSelection(int index, uint32_t buttons) {
    switch (buttons) {
    case button_type::Ok:
        return index == NSAlertFirstButtonReturn ? selection_type::Ok : selection_type::Ignore;
    case button_type::OkCancel:
        if (index == NSAlertFirstButtonReturn) {
            return selection_type::Ok;
        } 
        else if (index == NSAlertSecondButtonReturn) {
            return selection_type::Cancel;
        } 
        else {
            return selection_type::Ignore;
        }
    case button_type::YesNo:
        if (index == NSAlertFirstButtonReturn) {
            return selection_type::Yes;
        } 
        else if (index == NSAlertSecondButtonReturn) {
            return selection_type::No;
        } 
        else {
            return selection_type::Ignore;
        }
    case button_type::Abort:
        return index == NSAlertFirstButtonReturn ? selection_type::Abort : selection_type::Ignore;
    default:
        return selection_type::Ignore;
    }
}

uint32_t ShowDialogImpl(const char *message, const char *title, uint32_t style, uint32_t buttons) {
   NSAlert *alert = [[NSAlert alloc] init];

   [alert setMessageText:[NSString stringWithCString:title
                                   encoding:[NSString defaultCStringEncoding]]];
   [alert setInformativeText:[NSString stringWithCString:message
                                       encoding:[NSString defaultCStringEncoding]]];

   [alert setAlertStyle:getAlertStyle(style)];
   setButtons(alert, buttons);

   // Force the alert to appear on top of any other windows
   [[alert window] setLevel:NSModalPanelWindowLevel];

   uint32_t selection = getSelection([alert runModal], buttons);
   [alert release];

   return selection;
}

