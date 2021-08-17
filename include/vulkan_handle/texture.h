#ifndef INCLUDE_VULKAN_HANDLE_TEXTURE
#define INCLUDE_VULKAN_HANDLE_TEXTURE

#include <stdint.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct Texture Texture;

struct Texture {
    uint32_t mipLevels;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
};

void createTextureImage(Vulkan *);

void createTextureImageView(Vulkan *);

void createTextureSampler(Vulkan *);

void createImageViews(Vulkan *);

void createColorResources(Vulkan *);

void copyBufferToImage(Vulkan *, VkBuffer, VkImage, uint32_t, uint32_t);

void generateMipmaps(Vulkan *, VkImage, VkFormat, int32_t, int32_t, uint32_t);

VkImageView createImageView(VkDevice, VkImage, VkFormat, VkImageAspectFlags,
                            uint32_t);

void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat,
                 VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags,
                 Vulkan *, VkImage *, VkDeviceMemory *);

#endif /* INCLUDE_VULKAN_HANDLE_TEXTURE */
