#pragma once

constexpr bool DebugAssertEnabled() {
#ifndef CA_DEBUG
    return false;
#else
    return true;
#endif
}
