#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "vulkan_handle/vulkan_handle.h"
#include "error_handle.h"
#include "memory.h"
#include "swapchain.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/image_view.h"
#include "vulkan_handle/uniforms.h"
#include "vulkan_handle/validation.h"
#include "window/window.h"
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/io.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTimeCommands(Vulkan *vulkan) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkan->renderBuffers.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkan->device.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(Vulkan *vulkan, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkan->device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->device.graphicsQueue);

    vkFreeCommandBuffers(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, 1, &commandBuffer);
}

void copyBuffer(Vulkan *vulkan, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, Vulkan *vulkan,
                 VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vulkan->device.device, &imageInfo, NULL, image) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create image!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkan->device.device, *image,
                                 &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkan, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkan->device.device, &allocInfo, NULL,
                         imageMemory) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate image memory!\n");
    }

    vkBindImageMemory(vulkan->device.device, *image, *imageMemory, 0);
}

void copyBufferToImage(Vulkan *vulkan, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createTextureImageView(Vulkan *vulkan) {
    vulkan->textureImageView = createImageView(
        vulkan->device.device, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT, vulkan->mipLevels);
}

void createTextureSampler(Vulkan *vulkan) {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(vulkan->device.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = (float)vulkan->mipLevels;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(vulkan->device.device, &samplerInfo, NULL,
                        &vulkan->textureSampler) != VK_SUCCESS) {
        THROW_ERROR("failed to create texture sampler!\n");
    }
}

VkFormat findSupportedFormat(const VkFormat *candidates, size_t length,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features,
                             VkPhysicalDevice physicalDevice) {
    // for (VkFormat format : candidates) {
    for (size_t i = 0; i < length; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i],
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    THROW_ERROR("Failed to find supported format!\n");
}

VkFormat findDepthFormat(Vulkan *vulkan) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(candidates, SIZEOF(candidates),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               vulkan->device.physicalDevice);
}

void createDepthResources(Vulkan *vulkan) {
    VkFormat depthFormat = findDepthFormat(vulkan);

    createImage(vulkan->swapchain.swapChainExtent.width,
                vulkan->swapchain.swapChainExtent.height, 1,
                vulkan->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->depthImage, &vulkan->depthImageMemory);

    vulkan->depthImageView =
        createImageView(vulkan->device.device, vulkan->depthImage, depthFormat,
                        VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void createColorResources(Vulkan *vulkan) {
    VkFormat colorFormat = vulkan->swapchain.swapChainImageFormat;

    createImage(vulkan->swapchain.swapChainExtent.width,
                vulkan->swapchain.swapChainExtent.height, 1,
                vulkan->msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->colorImage, &vulkan->colorImageMemory);

    vulkan->colorImageView =
        createImageView(vulkan->device.device, vulkan->colorImage, colorFormat,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void generateMipmaps(Vulkan *vulkan, VkImage image, VkFormat imageFormat,
                     int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(vulkan->device.physicalDevice,
                                        imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        THROW_ERROR("Texture image format does not support linear blitting!\n");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &barrier);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.dstOffsets[1] = (VkOffset3D){mipWidth > 1 ? mipWidth / 2 : 1,
                                          mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL,
                             0, NULL, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                         NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void initVulkan(Window *window, Vulkan *vulkan) {
    createInstance(window->win, vulkan);

    setupDebugMessenger(vulkan);

    createSurface(window, vulkan);

    pickPhysicalDevice(window, vulkan);

    createLogicalDevice(window, vulkan);

    createSwapChain(window, vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createColorResources(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(window, vulkan);

    createTextureImage(vulkan);

    createTextureImageView(vulkan);

    createTextureSampler(vulkan);

    combineVerticesAndIndices(vulkan, 2, (Shape){shape1, 4, NULL, 6, 0},
                              (Shape){shape2, 4, NULL, 6, 0});
    createVertexBuffer(vulkan);
    createIndexBuffer(vulkan);

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan, window);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Window *window, Vulkan *vulkan) {
    cleanupSwapChain(vulkan);

    vkDestroySampler(vulkan->device.device, vulkan->textureSampler, NULL);
    vkDestroyImageView(vulkan->device.device, vulkan->textureImageView, NULL);

    vkDestroyImage(vulkan->device.device, vulkan->textureImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->textureImageMemory, NULL);

    vkDestroyDescriptorSetLayout(vulkan->device.device,
                                 vulkan->ubo.descriptorSetLayout, NULL);

    if (vulkan->graphicsPipeline.graphicsPipeline == VK_NULL_HANDLE) {
        vkDestroyPipeline(vulkan->device.device,
                          vulkan->graphicsPipeline.graphicsPipeline, NULL);
    }
    if (vulkan->graphicsPipeline.pipelineLayout == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vulkan->device.device,
                                vulkan->graphicsPipeline.pipelineLayout, NULL);
    }

    vkDestroyBuffer(vulkan->device.device, vulkan->shapeBuffers.indexBuffer,
                    NULL);
    vkFreeMemory(vulkan->device.device, vulkan->shapeBuffers.indexBufferMemory,
                 NULL);

    vkDestroyBuffer(vulkan->device.device, vulkan->shapeBuffers.vertexBuffer,
                    NULL);
    vkFreeMemory(vulkan->device.device, vulkan->shapeBuffers.vertexBufferMemory,
                 NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.renderFinishedSemaphores[i],
                           NULL);
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.imageAvailableSemaphores[i],
                           NULL);
        vkDestroyFence(vulkan->device.device,
                       vulkan->semaphores.inFlightFences[i], NULL);
    }

    freeMem(8, vulkan->ubo.descriptorSets, vulkan->shapes.vertices,
            vulkan->shapes.indices, vulkan->semaphores.renderFinishedSemaphores,
            vulkan->semaphores.imageAvailableSemaphores,
            vulkan->semaphores.inFlightFences,
            vulkan->semaphores.imagesInFlight,
            vulkan->renderBuffers.commandBuffers);

    vkDestroyCommandPool(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, NULL);

    vkDestroyDevice(vulkan->device.device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan->instance.instance,
                                      vulkan->validation->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(vulkan->instance.instance, window->surface, NULL);
    vkDestroyInstance(vulkan->instance.instance, NULL);
}
