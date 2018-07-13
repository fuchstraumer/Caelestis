#pragma once
#ifndef VULKAN_SCENE_TEST_FIXTURE_HPP
#define VULKAN_SCENE_TEST_FIXTURE_HPP
#include <cstdint>
#include <chrono>
#include <memory>

namespace vpr {
    class Device;
    class PhysicalDevice;
    class Instance;
    class Swapchain;
    class Semaphore;
}

struct RequiredVprObjects {
    vpr::Device* device;
    vpr::PhysicalDevice* physicalDevice;
    vpr::Instance* instance;
    vpr::Swapchain* swapchain;
};

class VulkanScene {
    VulkanScene();
    virtual ~VulkanScene();
    VulkanScene(const VulkanScene&) = delete;
    VulkanScene& operator=(const VulkanScene&) = delete;
public:

    static VulkanScene& GetScene();

    virtual void Construct(RequiredVprObjects vpr_objects, void* user_data);
    virtual void Destroy();
    virtual void Render(void* user_data) = 0;

private:

    virtual void limitFrame();
    virtual void update();
    virtual void acquireImage();
    virtual void recordCommands();
    virtual void draw();
    virtual void present();

    std::chrono::system_clock::time_point limiterA;
    std::chrono::system_clock::time_point limiterB;
    uint32_t currentBuffer;
    RequiredVprObjects vprObjects;
    std::unique_ptr<vpr::Semaphore> imageAcquireSemaphore;
    std::unique_ptr<vpr::Semaphore> renderCompleteSemaphore;

};

#endif //!VULKAN_SCENE_TEST_FIXTURE_HPP
