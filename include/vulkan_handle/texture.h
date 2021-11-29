#ifndef INCLUDE_VULKAN_HANDLE_TEXTURE
#define INCLUDE_VULKAN_HANDLE_TEXTURE

typedef unsigned int uint32_t;
typedef enum VkSampleCountFlagBits VkSampleCountFlagBits;
typedef enum VkImageUsageFlagBits VkImageUsageFlagBits;
typedef enum VkImageAspectFlagBits VkImageAspectFlagBits;
typedef struct VkImage_T *VkImage;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkImageView_T *VkImageView;
typedef struct VkSampler_T *VkSampler;

typedef struct Texture {
    uint32_t mipLevels;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
} Texture;

typedef struct Vulkan Vulkan;

void createTextureImage(Vulkan *, const char *);

void createTextureImageView(Vulkan *);

void createTextureSampler(Vulkan *);

void createImageViews(Vulkan *);

void createResourceFormat(Vulkan *, VkImageUsageFlagBits, VkImageAspectFlagBits,
                          VkImage *, VkDeviceMemory *, VkImageView *);

typedef enum VkFormat VkFormat;
typedef struct VkDevice_T *VkDevice;
typedef uint32_t VkFlags;
typedef VkFlags VkImageAspectFlags;
VkImageView createImageView(VkDevice, VkImage, VkFormat, VkImageAspectFlags,
                            uint32_t);

typedef enum VkSampleCountFlagBits VkSampleCountFlagBits;
typedef enum VkImageTiling VkImageTiling;
typedef VkFlags VkImageUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;
void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat,
                 VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags,
                 Vulkan *, VkImage *, VkDeviceMemory *);

typedef struct VkCommandBuffer_T *VkCommandBuffer;

VkCommandBuffer beginSingleTimeCommands(Vulkan *);

void endSingleTimeCommands(Vulkan *, VkCommandBuffer);

#endif /* INCLUDE_VULKAN_HANDLE_TEXTURE */
