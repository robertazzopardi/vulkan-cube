#ifndef INCLUDE_VULKAN_HANDLE_SWAPCHAIN
#define INCLUDE_VULKAN_HANDLE_SWAPCHAIN

#include <vulkan/vulkan.h>

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);

#endif /* INCLUDE_VULKAN_HANDLE_SWAPCHAIN */
