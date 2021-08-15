#ifndef INCLUDE_VULKAN_HANDLE_IMAGE_VIEW
#define INCLUDE_VULKAN_HANDLE_IMAGE_VIEW

#include <vulkan/vulkan.h>

typedef struct ImageView ImageView;

typedef struct Vulkan Vulkan;

struct ImageView {};

void createImageViews(Vulkan *);

VkImageView createImageView(VkDevice, VkImage, VkFormat, VkImageAspectFlags,
                            uint32_t);

void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat,
                 VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags,
                 Vulkan *, VkImage *, VkDeviceMemory *);

#endif /* INCLUDE_VULKAN_HANDLE_IMAGE_VIEW */
