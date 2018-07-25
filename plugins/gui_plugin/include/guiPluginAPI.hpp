#pragma once
#ifndef GUI_PLUGIN_API_HPP
#define GUI_PLUGIN_API_HPP
#include <cstdint>

constexpr static uint32_t GUI_PLUGIN_API_ID = 0x7603c270;

struct guiPluginApi {
    void (*Create)(uint64_t renderpass_handle, uint32_t vk_bool_sample_shading_enabled, uint32_t num_samples);
    // Call to update input handling systems: should be called AFTER the 
    // renderer context updates, or after a call to glfwPollEvents()
    void (*NewFrame)(void);
    // Ends current frame "recording" of draw commands: get any GUI
    // draw commands in before calling this, then proceed.
    void (*EndFrame)(void);
    // Call to draw the GUI. Must have an active renderpass.
    void (*DrawFrame)(uint32_t frame_idx, uint64_t cmd_buffer_handle);
};

#endif //GUI_PLUGIN_API_HPP
