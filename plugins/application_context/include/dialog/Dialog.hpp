#pragma once
#ifndef APPLICATION_CONTEXT_PLATFORM_DIALOG_HPP
#define APPLICATION_CONTEXT_PLATFORM_DIALOG_HPP
#include <cstdint>

namespace dialog_type {
    enum e : uint32_t {
        None = 0,
        Info,
        Warning,
        Error,
        Question,
        Abort
    };
}

namespace button_type {
    enum e : uint32_t {
        None = 0,
        Ok,
        OkCancel,
        YesNo,
        YesNoCancel,
        AbortRetryIgnore,
        RetryCancel,
        Abort
    };
}

namespace selection_type {
    enum e : uint32_t {
        None = 0,
        Ok,
        Cancel,
        Yes,
        No,
        Abort,
        Retry,
        Ignore
    };
}

uint32_t ShowDialogImpl(const char* message, const char* title, uint32_t dialog_type, uint32_t buttons);

#endif //!APPLICATION_CONTEXT_PLATFORM_DIALOG_HPP
