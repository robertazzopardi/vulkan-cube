#include "vulkan_handle/swapchain.h"
#include "error_handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.h>

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
    SwapChainSupportDetails details = {};
    details.formats = NULL;
    details.presentModes = NULL;
    details.formatCount = 0;
    details.presentModeCount = 0;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount,
                                         NULL);

    if (details.formatCount != 0) {
        details.formats =
            malloc(details.formatCount * sizeof(*details.formats));
        if (details.formats == NULL) {
            THROW_ERROR("Could not init SwapChainSupportDetails formats\n");
        }
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &details.formatCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &details.presentModeCount, NULL);

    if (details.presentModeCount != 0) {
        details.presentModes =
            malloc(details.presentModeCount * sizeof(*details.presentModes));
        if (details.presentModes == NULL) {
            THROW_ERROR(
                "Could not init SwapChainSupportDetails presentModes\n");
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &details.presentModeCount, details.presentModes);
    }

    return details;
}
