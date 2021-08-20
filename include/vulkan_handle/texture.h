#ifndef INCLUDE_VULKAN_HANDLE_TEXTURE
#define INCLUDE_VULKAN_HANDLE_TEXTURE

typedef unsigned int uint32_t;
typedef enum VkSampleCountFlagBits VkSampleCountFlagBits;
typedef struct VkImage_T *VkImage;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkImageView_T *VkImageView;
typedef struct VkSampler_T *VkSampler;

typedef struct Texture {
    uint32_t mipLevels;

    // VkImage textureImage;
    // VkDeviceMemory textureImageMemory;
    // VkImageView textureImageView;
    // VkSampler textureSampler;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
} Texture;

typedef struct Vulkan Vulkan;

void createTextureImage(Vulkan *);

void createTextureImageView(Vulkan *);

void createTextureSampler(Vulkan *);

void createImageViews(Vulkan *);

void createColorResources(Vulkan *);

typedef struct VkBuffer_T *VkBuffer;
void copyBufferToImage(Vulkan *, VkBuffer, VkImage, uint32_t, uint32_t);

typedef enum VkFormat VkFormat;
void generateMipmaps(Vulkan *, VkImage, VkFormat, int, int, uint32_t);

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

#endif /* INCLUDE_VULKAN_HANDLE_TEXTURE */
