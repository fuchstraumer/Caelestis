
ADD_PLUGIN(resource_context
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ResourceContextAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ResourceTypes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ResourceContext.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/TransferSystem.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ResourceLoader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceContextAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceContext.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TransferSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UploadBuffer.hpp"
)

TARGET_LINK_LIBRARIES(resource_context PRIVATE vpr_alloc vpr_core vpr_command vpr_sync ${Vulkan_LIBRARY} easyloggingpp)
TARGET_INCLUDE_DIRECTORIES(resource_context PRIVATE
    "${Vulkan_INCLUDE_DIR}"
    "../../ext/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../plugins"
    "../../third_party/easyloggingpp/src"
)