#include "resources/Texture.hpp"
#include "systems/TransferSystem.hpp"
#include "RendererCore.hpp"

namespace vpsk {

    Texture::Texture(const vpr::Device * dvc) noexcept : device(dvc), image(nullptr), sampler(nullptr) {}
    
    Texture::Texture(Texture && other) noexcept : device(other.device), imageInfo(std::move(other.imageInfo)), samplerInfo(std::move(other.samplerInfo)), viewInfo(std::move(other.viewInfo)),
        viewInfoSet(std::move(other.viewInfoSet)), descriptorInfo(std::move(other.descriptorInfo)), descriptorInfoSet(std::move(other.descriptorInfoSet)), image(std::move(other.image)),
        stagingBuffer(std::move(other.stagingBuffer)), copyInfo(std::move(other.copyInfo)), sampler(std::move(other.sampler)) {}

    Texture& Texture::operator=(Texture && other) noexcept {
        device = other.device;
        sampler = std::move(other.sampler);
        imageInfo = std::move(other.imageInfo);
        samplerInfo = std::move(other.samplerInfo);
        viewInfo = std::move(other.viewInfo);
        viewInfoSet = std::move(other.viewInfoSet);
        descriptorInfo = std::move(other.descriptorInfo);
        descriptorInfoSet = std::move(other.descriptorInfoSet);
        image = std::move(other.image);
        stagingBuffer = std::move(other.stagingBuffer);
        copyInfo = std::move(other.copyInfo);
        return *this;
    }
  
    void Texture::CreateFromFile(const char * fname) {
        loadTextureDataFromFile(fname);
    }
    
    void Texture::CreateFromBuffer(VkBuffer staging_buffer, VkBufferImageCopy * copies, const size_t num_copies) {
        copyInfo = std::vector<VkBufferImageCopy>{ copies, copies + num_copies };
        createImage();
        createView();
    }
  
    void Texture::TransferToDevice(VkCommandBuffer cmd) {
        auto barrier0 = vpr::Image::GetMemoryBarrier(image->vkHandle(), format(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        barrier0.subresourceRange = { aspect(), 0, mipLevels(), 0, arrayLayers() };
        auto barrier1 = vpr::Image::GetMemoryBarrier(image->vkHandle(), format(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout());
        barrier1.subresourceRange = barrier0.subresourceRange;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);
        vkCmdCopyBufferToImage(cmd, stagingBuffer->vkHandle(), image->vkHandle(), finalLayout(), static_cast<uint32_t>(copyInfo.size()), copyInfo.data());
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
    }

    void Texture::FreeStagingBuffer() {
        stagingBuffer.reset();
    }

    const VkImage & Texture::Image() const noexcept {
        return image->vkHandle();
    }

    const VkSampler & Texture::Sampler() const noexcept {
        return sampler->vkHandle();
    }

    const VkImageView& Texture::View() const noexcept {
        return image->View();
    }
   
    const VkDescriptorImageInfo& Texture::Descriptor() const noexcept {
        if (!descriptorInfoSet) {
            setDescriptorInfo();
        }
        return descriptorInfo;
    }
   
    void Texture::SetImageInfo(VkImageCreateInfo image_info) {
        imageInfo = std::move(image_info);
    }
  
    void Texture::SetSamplerInfo(VkSamplerCreateInfo sampler_info) {
        samplerInfo = std::move(sampler_info);
    }
   
    void Texture::SetViewInfo(VkImageViewCreateInfo view_info) {
        viewInfo = std::move(view_info);
    }

    void Texture::createImage() {
        image->Create(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
 
    void Texture::createView() {
        image->CreateView(viewInfo);
    }
 
    void Texture::setDescriptorInfo() const {
        descriptorInfo = VkDescriptorImageInfo{ Sampler(), View(), finalLayout() };
    }

    void Texture::finishSetup() {
        createCopyInformation();
        updateImageInfo();
        createImage();
        updateViewInfo();
        createView();
        scheduleDeviceTransfer();
    }

    void Texture::scheduleDeviceTransfer() {
        ResourceTransferSystem::TransferDelegate transfer_func = ResourceTransferSystem::TransferDelegate::create<Texture, &Texture::TransferToDevice>(this);
        ResourceTransferSystem::SignalDelegate signal_func = ResourceTransferSystem::SignalDelegate::create<Texture, &Texture::FreeStagingBuffer>(this);
        auto* loader = RendererCore::GetRenderer().TransferSystem();
        loader->AddTransferRequest(transfer_func, signal_func);
    }

}
