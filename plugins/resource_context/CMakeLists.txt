
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

IF(APPLE)
# Needed to support Boost filesystem
SET(boost_min_ver 1.67.0)
SET(boost_libs system filesystem)
FIND_PACKAGE(Boost ${boost_min_ver})
IF(Boost_FOUND)
    FIND_PACKAGE(Boost ${boost_min_ver} COMPONENTS ${boost_libs})
ENDIF()
ENDIF()

TARGET_LINK_LIBRARIES(resource_context PRIVATE vpr_alloc vpr_core vpr_command vpr_sync ${Vulkan_LIBRARY} easyloggingpp)
TARGET_INCLUDE_DIRECTORIES(resource_context PRIVATE
    "${Vulkan_INCLUDE_DIR}"
    "../../ext/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "../../plugins"
    "../../third_party/easyloggingpp/src"
)

IF(APPLE)
TARGET_LINK_LIBRARIES(resource_context PRIVATE ${Boost_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(resource_context PRIVATE ${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
ENDIF()
