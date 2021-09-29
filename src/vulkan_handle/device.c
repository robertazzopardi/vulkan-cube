#include "vulkan_handle/device.h"
#include "error_handle.h"
#include "utility/error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/swapchain.h"
#include "vulkan_handle/validation.h"
#include "vulkan_handle/vulkan_handle.h"
#include "window/window.h"
#include <string.h>
#include <vulkan/vulkan.h>

const uint32_t MAX_FAMILY = 1000;

static const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

bool isComplete(QueueFamilyIndices queueFamilyIndices) {
    return queueFamilyIndices.graphicsFamily != MAX_FAMILY &&
           queueFamilyIndices.presentFamily != MAX_FAMILY;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
    QueueFamilyIndices queueFamilyIndices = {.graphicsFamily = MAX_FAMILY,
                                             .presentFamily = MAX_FAMILY};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    int i = 0;
    for (uint32_t j = 0; j < queueFamilyCount; j++) {
        if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

        if (presentSupport) {
            queueFamilyIndices.presentFamily = i;
        }

        if (isComplete(queueFamilyIndices)) {
            break;
        }

        i++;
    }

    return queueFamilyIndices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
                                         availableExtensions);

    // for (uint32_t i = 0; i < extensionCount; i++) {
    //     printf("%s\n", availableExtensions[i].extensionName);
    // }

    const char *requiredExtensions[SIZEOF(deviceExtensions)];
    for (uint32_t i = 0; i < SIZEOF(deviceExtensions); i++) {
        requiredExtensions[i] = deviceExtensions[i];
    }

    for (uint32_t i = 0; i < extensionCount; i++) {
        for (uint32_t j = 0; j < SIZEOF(deviceExtensions); j++) {
            if (strcmp(availableExtensions[i].extensionName,
                       requiredExtensions[j]) == 0) {
                requiredExtensions[j] = "";
                break;
            }
        }
    }

    bool empty = false;
    for (uint32_t i = 0; i < SIZEOF(deviceExtensions); i++) {
        if (strcmp(requiredExtensions[i], "") == 0) {
            empty = true;
            break;
        }
    }

    return empty;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device, surface);
        swapChainAdequate = swapChainSupport.formats != NULL &&
                            swapChainSupport.presentModes != NULL;

        freeMem(3, swapChainSupport.formats, swapChainSupport.presentModes,
                swapChainSupport.capabilities);
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return isComplete(queueFamilyIndices) && extensionsSupported &&
           swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSampleCountFlagBits getMaxUsableSampleCount(Vulkan *vulkan) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(vulkan->device.physicalDevice,
                                  &physicalDeviceProperties);

    VkSampleCountFlags counts =
        physicalDeviceProperties.limits.framebufferColorSampleCounts &
        physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void pickPhysicalDevice(Window *window, Vulkan *vulkan) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        THROW_ERROR("failed to find GPUs with Vulkan support!\n");
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, devices);

    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], window->surface)) {
            vulkan->device.physicalDevice = devices[i];
            vulkan->msaaSamples = getMaxUsableSampleCount(vulkan);
            break;
        }
    }

    if (vulkan->device.physicalDevice == VK_NULL_HANDLE) {
        THROW_ERROR("failed to find a suitable GPU!\n");
    }
}

void createLogicalDevice(Window *window, Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, window->surface);

    uint32_t uniqueQueueFamilies[] = {queueFamilyIndices.graphicsFamily,
                                      queueFamilyIndices.presentFamily};
    VkDeviceQueueCreateInfo queueCreateInfos[SIZEOF(uniqueQueueFamilies)];

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < SIZEOF(uniqueQueueFamilies); i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    // enable sample shading feature for the device
    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = SIZEOF(queueCreateInfos);
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.enabledExtensionCount = SIZEOF(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vulkan->device.physicalDevice, &createInfo, NULL,
                       &vulkan->device.device) != VK_SUCCESS) {
        THROW_ERROR("failed to create logical device!\n");
    }

    vkGetDeviceQueue(vulkan->device.device, queueFamilyIndices.graphicsFamily,
                     0, &vulkan->device.graphicsQueue);
    vkGetDeviceQueue(vulkan->device.device, queueFamilyIndices.presentFamily, 0,
                     &vulkan->device.presentQueue);
}
