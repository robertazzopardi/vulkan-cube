#ifndef INCLUDE_VULKAN_HANDLE_DEVICE
#define INCLUDE_VULKAN_HANDLE_DEVICE

#include <vulkan/vulkan.h>

#include <stdint.h>

static const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

typedef struct Device Device;

typedef struct Window Window;

// typedef struct VkPhysicalDevice VkPhysicalDevice;
// typedef struct VkDevice VkDevice;

struct Device {
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

typedef struct Vulkan Vulkan;

void pickPhysicalDevice(Window *, Vulkan *);

typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);

void createLogicalDevice(Window *, Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_DEVICE */
