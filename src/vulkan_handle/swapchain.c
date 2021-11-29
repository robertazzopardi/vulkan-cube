#include "vulkan_handle/swapchain.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/vulkan_handle.h"
#include "window/window.h"
#include <SDL_events.h>
#include <SDL_vulkan.h>
#include <cglm/util.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

static inline VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,
                 SDL_Window *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t)width,
            (uint32_t)height,
        };

        glm_clamp(actualExtent.width, capabilities.minImageExtent.width,
                  capabilities.maxImageExtent.width);
        glm_clamp(actualExtent.height, capabilities.minImageExtent.height,
                  capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
    SwapChainSupportDetails details = {
        .formats = NULL,
        .presentModes = NULL,
        .formatCount = 0,
        .presentModeCount = 0,
        .capabilities = malloc(1 * sizeof(*details.capabilities)),
    };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              details.capabilities);

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

void recreateSwapChain(Vulkan *vulkan) {
    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(vulkan->window.win, &width, &height);
    while (width == 0 || height == 0) {
        SDL_Vulkan_GetDrawableSize(vulkan->window.win, &width, &height);
        SDL_PollEvent(vulkan->window.event);
    }

    vkDeviceWaitIdle(vulkan->device.device);

    cleanupSwapChain(vulkan);

    createSwapChain(vulkan);
    createImageViews(vulkan);
    createRenderPass(vulkan);
    createResourceFormat(vulkan, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_ASPECT_DEPTH_BIT, &vulkan->depth.depthImage,
                         &vulkan->depth.depthImageMemory,
                         &vulkan->depth.depthImageView);
    createResourceFormat(vulkan, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                         VK_IMAGE_ASPECT_COLOR_BIT, &vulkan->colorImage,
                         &vulkan->colorImageMemory, &vulkan->colorImageView);
    createFramebuffers(vulkan);

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        createGraphicsPipeline(
            vulkan, &vulkan->shapes[i].descriptorSet.descriptorSetLayout,
            &vulkan->shapes[i].graphicsPipeline);
        createDescriptorPool(vulkan,
                             &vulkan->shapes[i].descriptorSet.descriptorPool);
        createUniformBuffers(vulkan, &vulkan->shapes[i].descriptorSet);
        createDescriptorSets(vulkan, &vulkan->shapes[i].descriptorSet,
                             &vulkan->shapes[i].texture);
    }

    createCommandBuffers(vulkan);

    freeMem(1, vulkan->semaphores.imagesInFlight);

    vulkan->semaphores.imagesInFlight =
        malloc(1 * sizeof(*vulkan->semaphores.imagesInFlight));
}

static inline VkPresentModeKHR
chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static inline VkSurfaceFormatKHR
chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

void createSwapChain(Vulkan *vulkan) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(
        vulkan->device.physicalDevice, vulkan->window.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent =
        chooseSwapExtent(*swapChainSupport.capabilities, vulkan->window.win);

    vulkan->swapchain.swapChainImagesCount =
        swapChainSupport.capabilities->minImageCount + 1;

    if (swapChainSupport.capabilities->maxImageCount > 0 &&
        vulkan->swapchain.swapChainImagesCount >
            swapChainSupport.capabilities->maxImageCount) {
        vulkan->swapchain.swapChainImagesCount =
            swapChainSupport.capabilities->maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkan->window.surface,
        .minImageCount = vulkan->swapchain.swapChainImagesCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapChainSupport.capabilities->currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(
        vulkan->device.physicalDevice, vulkan->window.surface);

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndicesArr[] = {
            queueFamilyIndices.graphicsFamily,
            queueFamilyIndices.presentFamily,
        };
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

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

    vulkan->swapchain.swapChainImageFormat =
        malloc(1 * sizeof(*vulkan->swapchain.swapChainImageFormat));
    *vulkan->swapchain.swapChainImageFormat = surfaceFormat.format;
    vulkan->swapchain.swapChainExtent =
        malloc(1 * sizeof(*vulkan->swapchain.swapChainExtent));
    *vulkan->swapchain.swapChainExtent = extent;
}

void cleanupSwapChain(Vulkan *vulkan) {
    vkDestroyImageView(vulkan->device.device, vulkan->colorImageView, NULL);
    vkDestroyImage(vulkan->device.device, vulkan->colorImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->colorImageMemory, NULL);

    vkDestroyImageView(vulkan->device.device, vulkan->depth.depthImageView,
                       NULL);
    vkDestroyImage(vulkan->device.device, vulkan->depth.depthImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->depth.depthImageMemory, NULL);

    vkFreeCommandBuffers(vulkan->device.device,
                         vulkan->renderBuffers.commandPool,
                         vulkan->swapchain.swapChainImagesCount,
                         vulkan->renderBuffers.commandBuffers);

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        vkDestroyPipeline(vulkan->device.device,
                          vulkan->shapes[i].graphicsPipeline.graphicsPipeline,
                          NULL);
        vkDestroyPipelineLayout(
            vulkan->device.device,
            vulkan->shapes[i].graphicsPipeline.pipelineLayout, NULL);

        vkDestroyDescriptorPool(vulkan->device.device,
                                vulkan->shapes[i].descriptorSet.descriptorPool,
                                NULL);
    }

    vkDestroyRenderPass(vulkan->device.device, vulkan->renderPass, NULL);

    vkDestroySwapchainKHR(vulkan->device.device, vulkan->swapchain.swapChain,
                          NULL);

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        for (uint32_t j = 0; j < vulkan->swapchain.swapChainImagesCount; j++) {
            vkDestroyBuffer(vulkan->device.device,
                            vulkan->shapes[i].descriptorSet.uniformBuffers[j],
                            NULL);

            vkFreeMemory(
                vulkan->device.device,
                vulkan->shapes[i].descriptorSet.uniformBuffersMemory[j], NULL);
        }
        freeMem(2, vulkan->shapes[i].descriptorSet.uniformBuffers,
                vulkan->shapes[i].descriptorSet.uniformBuffersMemory);
    }

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        vkDestroyFramebuffer(vulkan->device.device,
                             vulkan->renderBuffers.swapChainFramebuffers[i],
                             NULL);

        vkDestroyImageView(vulkan->device.device,
                           vulkan->swapchain.swapChainImageViews[i], NULL);
    }

    freeMem(4, vulkan->renderBuffers.swapChainFramebuffers,
            vulkan->swapchain.swapChainImageViews,
            vulkan->swapchain.swapChainImageFormat,
            vulkan->swapchain.swapChainExtent);
}
