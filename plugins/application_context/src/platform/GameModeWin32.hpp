#pragma once
#ifndef GAME_MODE_WIN32_HPP
#define GAME_MODE_WIN32_HPP

// Call on load to set state variables, query for any capabilities we might need
void QueryGameModeCapabilities();
// Call once per frame to potentially enable game mode.
void GameModeFrameFn();
bool ExclusiveCapable();
size_t ExclusiveCoreCount();
bool HasExpandedResources();
using ExclusiveModeCallbackFn = void(*)(bool mode_value);
void RegisterExclusiveModeCallback(ExclusiveModeCallbackFn fn);

#endif //GAME_MODE_WIN32_HPP
