
SET(IMGUI_IMPL 
    "../../third_party/imgui/imgui.h"
    "../../third_party/imgui/imconfig.h"
    "../../third_party/imgui/imgui_draw.cpp"
    "../../third_party/imgui/imgui.cpp"
    "../../third_party/imgui/imgui_demo.cpp"
)

ADD_PLUGIN(gui_plugin
    "include/guiPluginAPI.hpp"
    "include/ImGuiWrapper.hpp"
    "src/guiPluginAPI.cpp"
    "src/ImGuiWrapper.cpp"
    ${IMGUI_SOURCES}
)

SOURCE_GROUP("imgui" FILES ${IMGUI_SOURCES})

TARGET_INCLUDE_DIRECTORIES(gui_plugin PRIVATE
    "../../third_party/imgui"
    "${Vulkan_INCLUDE_DIR}"
    "../../plugins"
    "../../third_party"
    "../../ext/include"
)

TARGET_LINK_LIBRARIES(gui_plugin PRIVATE vpr_core vpr_resource ${Vulkan_LIBRARY})
