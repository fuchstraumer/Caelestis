#pragma once
#ifndef VPSK_ENUM_STRING_CONVERTERS_HPP
#define VPSK_ENUM_STRING_CONVERTERS_HPP
#include <vulkan/vulkan.h>
#include <string>

namespace vpsk {


    VkImageLayout LayoutFromStr(const std::string& str, bool shortform);
    VkFormat FormatFromStr(const std::string& str, bool shortform);
    std::string LayoutToStr(const VkImageLayout layout, bool shortform);
    std::string FormatToStr(const VkFormat format, bool shortform);
    VkAttachmentLoadOp LoadOpFromStr(const std::string& str, bool shortform);
    VkAttachmentStoreOp StoreOpFromStr(const std::string& str, bool shortform);


}
#endif // !VPSK_ENUM_STRING_CONVERTERS_HPP
