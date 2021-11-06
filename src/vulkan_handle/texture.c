#include "vulkan_handle/texture.h"
#include "utility/error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cglm/util.h>
#include <vulkan/vulkan.h>

void copyBufferToImage(Vulkan *vulkan, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferImageCopy region = {0};
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

static void transitionImageLayout(Vulkan *vulkan, VkImage image,
                                  VkFormat format __unused,
                                  VkImageLayout oldLayout,
                                  VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        THROW_ERROR("unsupported layout transition!\n");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         NULL, 0, NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
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

void createTextureImageView(Vulkan *vulkan) {
    vulkan->texture.textureImageView =
        createImageView(vulkan->device.device, vulkan->texture.textureImage,
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,
                        vulkan->texture.mipLevels);
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
    samplerInfo.maxLod = (float)vulkan->texture.mipLevels;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(vulkan->device.device, &samplerInfo, NULL,
                        &vulkan->texture.textureSampler) != VK_SUCCESS) {
        THROW_ERROR("failed to create texture sampler!\n");
    }
}

void createTextureImage(Vulkan *vulkan) {
    // SDL_Surface *image = IMG_Load("/Users/rob/Downloads/2k_saturn.jpg");
    SDL_Surface *image = IMG_Load("/Users/rob/Downloads/2k_jupiter.jpg");
    // SDL_Surface *image =
    // IMG_Load("/Users/rob/Downloads/2k_saturn_ring_alpha.jpg");

    // convert to desired format
    image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ABGR8888, 0);

    VkDeviceSize imageSize = image->w * image->h * image->format->BytesPerPixel;

    vulkan->texture.mipLevels =
        (uint32_t)(floor(log2(glm_max(image->w, image->h)))) + 1;

    if (!image) {
        printf("Could not load texture: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device.device, stagingBufferMemory, 0, imageSize, 0,
                &data);
    memcpy(data, image->pixels, (size_t)imageSize);
    vkUnmapMemory(vulkan->device.device, stagingBufferMemory);
    mapMemory(vulkan->device.device, stagingBufferMemory, imageSize,
              image->pixels);

    createImage(
        image->w, image->h, vulkan->texture.mipLevels, VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
        &vulkan->texture.textureImage, &vulkan->texture.textureImageMemory);

    transitionImageLayout(vulkan, vulkan->texture.textureImage,
                          VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          vulkan->texture.mipLevels);
    copyBufferToImage(vulkan, stagingBuffer, vulkan->texture.textureImage,
                      (uint32_t)image->w, (uint32_t)image->h);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);

    generateMipmaps(vulkan, vulkan->texture.textureImage,
                    VK_FORMAT_R8G8B8A8_SRGB, image->w, image->h,
                    vulkan->texture.mipLevels);

    SDL_FreeSurface(image);
}

void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, Vulkan *vulkan,
                 VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {0};
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

    VkMemoryAllocateInfo allocInfo = {0};
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

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
    vulkan->swapchain.swapChainImageViews =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->swapchain.swapChainImageViews));

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        vulkan->swapchain.swapChainImageViews[i] = createImageView(
            vulkan->device.device, vulkan->swapchain.swapChainImages[i],
            *vulkan->swapchain.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,
            1);
    }
}

void createColorResources(Vulkan *vulkan) {
    VkFormat colorFormat = *vulkan->swapchain.swapChainImageFormat;

    createImage(vulkan->swapchain.swapChainExtent->width,
                vulkan->swapchain.swapChainExtent->height, 1,
                vulkan->msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
                // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                vulkan, &vulkan->texture.colorImage,
                &vulkan->texture.colorImageMemory);

    vulkan->texture.colorImageView =
        createImageView(vulkan->device.device, vulkan->texture.colorImage,
                        colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

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
