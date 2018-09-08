#include "dialog/Dialog.hpp"
#define WIN32_MEAN_AND_LEAN
#include <windows.h>

UINT GetIcon(uint32_t type) {
    switch(type) {
    case dialog_type::Info:
        return MB_ICONINFORMATION;
    case dialog_type::Warning:
        return MB_ICONWARNING;
    case dialog_type::Error:
        return MB_ICONERROR;
    case dialog_type::Question:
        return MB_ICONQUESTION;
    default:
        return MB_ICONINFORMATION;
    };
}

UINT GetButtons(uint32_t buttons) {
    switch (buttons) {
    case button_type::Ok:
        return MB_OK;
    case button_type::OkCancel:
        return MB_OKCANCEL;
    case button_type::YesNo:
        return MB_YESNO;
    case button_type::YesNoCancel:
        return MB_YESNOCANCEL;
    case button_type::AbortRetryIgnore:
        return MB_ABORTRETRYIGNORE;
    case button_type::RetryCancel:
        return MB_RETRYCANCEL;
    default:
        return MB_OK;
    };
}

uint32_t GetResponse(int response) {
    switch (response) {
    case IDOK:
        return selection_type::Ok;
    case IDCANCEL:
        return selection_type::Cancel;
    case IDYES:
        return selection_type::Yes;
    case IDNO:
        return selection_type::No;
    case IDABORT:
        return selection_type::Abort;
    case IDRETRY:
        return selection_type::Retry;
    case IDIGNORE:
        return selection_type::Ignore;
    default:
        return selection_type::Ok;
    };
}

uint32_t ShowDialogImpl(const char* message, const char* title, uint32_t dialog_type, uint32_t buttons) {
    UINT flags = MB_TASKMODAL;
    flags |= GetIcon(dialog_type);
    flags |= GetButtons(buttons);
    return GetResponse(MessageBox(NULL, (LPCTSTR)message, (LPCTSTR)title, flags));
}
