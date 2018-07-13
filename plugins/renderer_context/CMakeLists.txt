ADD_SUBDIRECTORY("../../third_party/vulpesrender" "${CMAKE_CURRENT_BINARY_DIR}/vulpesrender")

ADD_MODULE(renderer_context
    "include/RendererContextAPI.hpp"
    "include/core/RendererContext.hpp"
    "include/core/PlatformWindow.hpp"
    "include/core/WindowInput.hpp"
    "src/RendererContextAPI.cpp"
    "src/core/RendererContext.cpp"
    "src/core/PlatformWindow.cpp"
    "src/core/WindowInput.cpp"
)

TARGET_INCLUDE_DIRECTORIES(renderer_context PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../ext/include/" "../../third_party/json/include" "${Vulkan_INCLUDE_DIR}")
TARGET_LINK_LIBRARIES(renderer_context PRIVATE vpr_core glfw ${Vulkan_LIBRARY})
TARGET_COMPILE_DEFINITIONS(renderer_context PRIVATE "D_SCL_SECURE_NO_WARNINGS" "SCL_SECURE_NO_WARNINGS")
