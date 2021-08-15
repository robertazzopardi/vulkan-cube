#ifndef INCLUDE_VULKAN_HANDLE_DEVICE
#define INCLUDE_VULKAN_HANDLE_DEVICE

#include <vulkan/vulkan.h>

static const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

typedef struct Device Device;

struct Device {
    VkPhysicalDevice physicalDevice;
    VkDevice device;
};

typedef struct Vulkan Vulkan;

void pickPhysicalDevice(Vulkan *);

typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);

#endif /* INCLUDE_VULKAN_HANDLE_DEVICE */
