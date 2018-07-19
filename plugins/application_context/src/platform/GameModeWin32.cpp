#include "GameModeWin32.hpp"
#include <system_error>
#include <forward_list>
#define WIN32_MEAN_AND_LEAN
#include <windows.h.>
#include <expandedresources.h>
#include "easylogging++.h"

static std::forward_list<ExclusiveModeCallbackFn> callbacks;
static bool exclusiveCapable = false;
static bool hasExpandedResources = false;
static size_t exclusiveCoreCount = 0;

struct WSAPI_error : public std::system_error {
    using std::system_error::system_error;
};

template<typename T>
void ThrowIfFailed(HRESULT hr, T&& msg) {
    if (FAILED(hr)) {
        throw WSAPI_error{ hr, std::system_category(), std::forward<T>(msg) };
    }
}

template<typename T>
void WarnIfFailed(HRESULT hr, T&& msg) {
    if (FAILED(hr)) {
        LOG(WARNING) << std::forward<T>(msg);
    }
}

template<typename T>
void WarnAndMutateStateIfFailed(HRESULT hr, bool& val, T&& msg) {
    if (FAILED(hr)) {
        val = !val;
        LOG(WARNING) << std::forward<T>(msg);
    }
}

void QueryGameModeCapabilities() {
    ULONG exclusive_cpu_count;
    WarnIfFailed(GetExpandedResourceExclusiveCpuCount(&exclusive_cpu_count), "Failed to get exclusive CPU count");
    if (exclusive_cpu_count) {
        exclusiveCoreCount = static_cast<size_t>(exclusive_cpu_count);
        exclusiveCapable = true;
    }
    else {
        WarnIfFailed(ReleaseExclusiveCpuSets(), "Failed to release exclusive CPU sets");
    }
}

void GameModeFrameFn() {
    // Set boolean flag to false so we skip this check if we shouldn't be doing it, and only warn once.
    static bool query_game_mode = true;
    if (exclusiveCapable && query_game_mode) {
        BOOL has_expanded_resources;
        WarnAndMutateStateIfFailed(HasExpandedResources(&has_expanded_resources), query_game_mode, "Failed to query if application has expanded resources!");
        if (static_cast<bool>(has_expanded_resources) != hasExpandedResources) {
            hasExpandedResources = static_cast<bool>(has_expanded_resources);
            for (auto& notif : callbacks) {
                notif(hasExpandedResources);
            }
        }
    }
}

bool ExclusiveCapable() {
    return exclusiveCapable;
}

size_t ExclusiveCoreCount() {
    return exclusiveCoreCount;
}

bool HasExpandedResources() {
    return hasExpandedResources;
}

void RegisterExclusiveModeCallback(ExclusiveModeCallbackFn fn) {
    callbacks.emplace_front(fn);
}