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

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,
        .imageOffset = (VkOffset3D){0, 0, 0},
        .imageExtent = (VkExtent3D){width, height, 1},
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(vulkan, commandBuffer);
}

static void transitionImageLayout(Vulkan *vulkan, VkImage image,
                                  VkFormat format __unused,
                                  VkImageLayout oldLayout,
                                  VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = mipLevels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

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

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = image,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
        .subresourceRange.levelCount = 1,
    };

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

        VkImageBlit blit = {
            .srcOffsets[0] = (VkOffset3D){0, 0, 0},
            .srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1},
            .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .srcSubresource.mipLevel = i - 1,
            .srcSubresource.baseArrayLayer = 0,
            .srcSubresource.layerCount = 1,
            .dstOffsets[0] = (VkOffset3D){0, 0, 0},
            .dstOffsets[1] = (VkOffset3D){mipWidth > 1 ? mipWidth / 2 : 1,
                                          mipHeight > 1 ? mipHeight / 2 : 1, 1},
            .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .dstSubresource.mipLevel = i,
            .dstSubresource.baseArrayLayer = 0,
            .dstSubresource.layerCount = 1,
        };

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
    vulkan->shapes[vulkan->shapeCount].texture.textureImageView =
        createImageView(vulkan->device.device,
                        vulkan->shapes[vulkan->shapeCount].texture.textureImage,
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,
                        vulkan->shapes[vulkan->shapeCount].texture.mipLevels);
}

void createTextureSampler(Vulkan *vulkan) {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(vulkan->device.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .minLod = 0.0f, // Optional
        .maxLod = (float)vulkan->shapes[vulkan->shapeCount].texture.mipLevels,
        .mipLodBias = 0.0f, // Optional
    };

    if (vkCreateSampler(
            vulkan->device.device, &samplerInfo, NULL,
            &vulkan->shapes[vulkan->shapeCount].texture.textureSampler) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create texture sampler!\n");
    }
}

void createTextureImage(Vulkan *vulkan, const char *fileName) {
    // SDL_Surface *image = IMG_Load("/Users/rob/Downloads/2k_saturn.jpg");
    // SDL_Surface *image = IMG_Load("/Users/rob/Downloads/2k_jupiter.jpg");
    // SDL_Surface *image =
    //     IMG_Load("/Users/rob/Downloads/2k_saturn_ring_alpha.png");
    SDL_Surface *image = IMG_Load(fileName);

    // convert to desired format
    image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ABGR8888, 0);

    VkDeviceSize imageSize = image->w * image->h * image->format->BytesPerPixel;

    vulkan->shapes[vulkan->shapeCount].texture.mipLevels =
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
        image->w, image->h,
        vulkan->shapes[vulkan->shapeCount].texture.mipLevels,
        VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
        &vulkan->shapes[vulkan->shapeCount].texture.textureImage,
        &vulkan->shapes[vulkan->shapeCount].texture.textureImageMemory);

    transitionImageLayout(
        vulkan, vulkan->shapes[vulkan->shapeCount].texture.textureImage,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        vulkan->shapes[vulkan->shapeCount].texture.mipLevels);
    copyBufferToImage(vulkan, stagingBuffer,
                      vulkan->shapes[vulkan->shapeCount].texture.textureImage,
                      (uint32_t)image->w, (uint32_t)image->h);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);

    generateMipmaps(vulkan,
                    vulkan->shapes[vulkan->shapeCount].texture.textureImage,
                    VK_FORMAT_R8G8B8A8_SRGB, image->w, image->h,
                    vulkan->shapes[vulkan->shapeCount].texture.mipLevels);

    SDL_FreeSurface(image);
}

void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, Vulkan *vulkan,
                 VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = numSamples,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

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
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = mipLevels,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

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
                vulkan, &vulkan->colorImage, &vulkan->colorImageMemory);

    vulkan->colorImageView =
        createImageView(vulkan->device.device, vulkan->colorImage, colorFormat,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VkCommandBuffer beginSingleTimeCommands(Vulkan *vulkan) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = vulkan->renderBuffers.commandPool,
        .commandBufferCount = 1,
    };

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

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(vulkan->device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->device.graphicsQueue);

    vkFreeCommandBuffers(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, 1, &commandBuffer);
}
