
#include "core/LogicalDevice.hpp"
#include "resource/Buffer.hpp"
namespace vpsk {

    struct vpskBuffer {
        std::unique_ptr<vpr::Buffer> Host;
    };
}