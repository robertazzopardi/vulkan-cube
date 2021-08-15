#include "vulkan_handle/image_view.h"
#include "error_handle.h"
#include "vulkan_handle/vulkan_handle.h"

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    // viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        THROW_ERROR("failed to create texture image view!\n");
    }

    return imageView;
}

void createImageViews(Vulkan *vulkan) {
    vulkan->swapChainImageViews = malloc(vulkan->swapChainImagesCount *
                                         sizeof(*vulkan->swapChainImageViews));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vulkan->swapChainImageViews[i] = createImageView(
            vulkan->device.device, vulkan->swapChainImages[i],
            vulkan->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}
