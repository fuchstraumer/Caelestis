#include "objects/DepthTarget.hpp"
#include "RendererCore.hpp"
#include <vulkan/vulkan.h>
#include "resource/Image.hpp"
#include "core/LogicalDevice.hpp"
#include "doctest/doctest.h"

namespace vpsk {

    DepthTarget::DepthTarget() : image(nullptr), imageMSAA(nullptr) {}

    DepthTarget::~DepthTarget() {
        image.reset();
        imageMSAA.reset();
    }

    void DepthTarget::Create(uint32_t width, uint32_t height, uint32_t sample_count_flags) {
        const VkSampleCountFlagBits sample_count = VkSampleCountFlagBits(sample_count_flags);
        VkImageCreateInfo image_info = vpr::vk_image_create_info_base;

        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;

        auto& renderer = RendererCore::GetRenderer();

        image_info.format = renderer.Device()->FindDepthFormat();
        image_info.samples = sample_count;
        image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_info.tiling = renderer.Device()->GetFormatTiling(image_info.format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        image = std::make_unique<vpr::Image>(renderer.Device());
        image->Create(image_info);
        image->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

        if (sample_count_flags > 1) {
            image_info.samples = VkSampleCountFlagBits(1);
            imageMSAA = std::make_unique<vpr::Image>(renderer.Device());
            image->Create(image_info);
            image->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
        }
    }

    void DepthTarget::CreateAsCube(uint32_t size, bool independent_faces)
    {
    }

    const vpr::Image * DepthTarget::GetImage() const noexcept {
        return image.get();
    }

    const vpr::Image * DepthTarget::GetImageMSAA() const noexcept {
        return imageMSAA.get();
    }

}

#ifdef VPSK_TESTING_ENABLED
TEST_SUITE("DepthTarget") {
    TEST_CASE("CreateDepthTarget") {
        std::unique_ptr<vpsk::DepthTarget> target = std::make_unique<vpsk::DepthTarget>();
        target->Create(1440, 900, VK_SAMPLE_COUNT_1_BIT);
    }
    TEST_CASE("CreateDepthTargetMultisampled") {
        std::unique_ptr<vpsk::DepthTarget> target = std::make_unique<vpsk::DepthTarget>();
        target->Create(1440, 900, VK_SAMPLE_COUNT_4_BIT);
    }
}
#endif // VPSK_UNIT_TESTING
