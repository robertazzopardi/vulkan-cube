#include "vulkan_handle/swapchain.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include "window/window.h"
#include <SDL_events.h>
#include <SDL_vulkan.h>
#include <stdio.h>
#include <stdlib.h>
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

void recreateSwapChain(Window *window, Vulkan *vulkan) {
    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(window->win, &width, &height);
    while (width == 0 || height == 0) {
        SDL_Vulkan_GetDrawableSize(window->win, &width, &height);
        SDL_PollEvent(window->event);
    }

    vkDeviceWaitIdle(vulkan->device.device);

    cleanupSwapChain(vulkan);

    createSwapChain(window, vulkan);
    createImageViews(vulkan);
    createRenderPass(vulkan);
    createGraphicsPipeline(vulkan);
    createColorResources(vulkan);
    createDepthResources(vulkan);
    createFramebuffers(vulkan);
    createUniformBuffers(vulkan);
    createDescriptorPool(vulkan);
    createDescriptorSets(vulkan);
    createCommandBuffers(vulkan, window);

    freeMem(1, vulkan->semaphores.imagesInFlight);

    vulkan->semaphores.imagesInFlight =
        calloc(vulkan->swapchain.swapChainImagesCount,
               sizeof(*vulkan->semaphores.imagesInFlight));
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,
                                       uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats,
                                           uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

void createSwapChain(Window *window, Vulkan *vulkan) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(vulkan->device.physicalDevice, window->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent =
        chooseSwapExtent(swapChainSupport.capabilities, window->win);

    vulkan->swapchain.swapChainImagesCount =
        swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        vulkan->swapchain.swapChainImagesCount >
            swapChainSupport.capabilities.maxImageCount) {
        vulkan->swapchain.swapChainImagesCount =
            swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = window->surface;
    createInfo.minImageCount = vulkan->swapchain.swapChainImagesCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, window->surface);

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndicesArr[] = {queueFamilyIndices.graphicsFamily,
                                            queueFamilyIndices.presentFamily};
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vulkan->device.device, &createInfo, NULL,
                             &vulkan->swapchain.swapChain) != VK_SUCCESS) {
        THROW_ERROR("failed to create swap chain!\n");
    }

    vkGetSwapchainImagesKHR(vulkan->device.device, vulkan->swapchain.swapChain,
                            &vulkan->swapchain.swapChainImagesCount, NULL);

    vulkan->swapchain.swapChainImages =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->swapchain.swapChainImages));

    if (vulkan->swapchain.swapChainImages == NULL) {
        THROW_ERROR("Could not init swap chain images\n");
    }
    vkGetSwapchainImagesKHR(vulkan->device.device, vulkan->swapchain.swapChain,
                            &vulkan->swapchain.swapChainImagesCount,
                            vulkan->swapchain.swapChainImages);

    vulkan->swapchain.swapChainImageFormat = surfaceFormat.format;
    vulkan->swapchain.swapChainExtent = extent;
}

void cleanupSwapChain(Vulkan *vulkan) {
    vkDestroyImageView(vulkan->device.device, vulkan->texture.colorImageView,
                       NULL);
    vkDestroyImage(vulkan->device.device, vulkan->texture.colorImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->texture.colorImageMemory, NULL);

    vkDestroyImageView(vulkan->device.device, vulkan->depth.depthImageView,
                       NULL);
    vkDestroyImage(vulkan->device.device, vulkan->depth.depthImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->depth.depthImageMemory, NULL);

    vkFreeCommandBuffers(vulkan->device.device,
                         vulkan->renderBuffers.commandPool,
                         vulkan->swapchain.swapChainImagesCount,
                         vulkan->renderBuffers.commandBuffers);

    vkDestroyPipeline(vulkan->device.device,
                      vulkan->graphicsPipeline.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vulkan->device.device,
                            vulkan->graphicsPipeline.pipelineLayout, NULL);
    vkDestroyRenderPass(vulkan->device.device,
                        vulkan->graphicsPipeline.renderPass, NULL);

    vkDestroySwapchainKHR(vulkan->device.device, vulkan->swapchain.swapChain,
                          NULL);

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        vkDestroyFramebuffer(vulkan->device.device,
                             vulkan->renderBuffers.swapChainFramebuffers[i],
                             NULL);

        vkDestroyImageView(vulkan->device.device,
                           vulkan->swapchain.swapChainImageViews[i], NULL);

        vkDestroyBuffer(vulkan->device.device, vulkan->ubo.uniformBuffers[i],
                        NULL);

        vkFreeMemory(vulkan->device.device, vulkan->ubo.uniformBuffersMemory[i],
                     NULL);
    }

    freeMem(4, vulkan->renderBuffers.swapChainFramebuffers,
            vulkan->swapchain.swapChainImageViews, vulkan->ubo.uniformBuffers,
            vulkan->ubo.uniformBuffersMemory);

    vkDestroyDescriptorPool(vulkan->device.device, vulkan->ubo.descriptorPool,
                            NULL);
}
