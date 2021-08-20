#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include "vulkan_handle/vulkan_handle.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/swapchain.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/validation.h"
#include "window/window.h"
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleTimeCommands(Vulkan *vulkan) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkan->renderBuffers.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkan->device.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(Vulkan *vulkan, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkan->device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->device.graphicsQueue);

    vkFreeCommandBuffers(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, 1, &commandBuffer);
}

void copyBuffer(Vulkan *vulkan, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkan, commandBuffer);
}

VkFormat findSupportedFormat(const VkFormat *candidates, size_t length,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features,
                             VkPhysicalDevice physicalDevice) {
    // for (VkFormat format : candidates) {
    for (size_t i = 0; i < length; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i],
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    THROW_ERROR("Failed to find supported format!\n");
}

VkFormat findDepthFormat(Vulkan *vulkan) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(candidates, SIZEOF(candidates),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               vulkan->device.physicalDevice);
}

void createInstance(SDL_Window *window, Vulkan *vulkan) {
    vulkan->device.physicalDevice = VK_NULL_HANDLE;
    vulkan->msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        THROW_ERROR("validation layers requested, but not available!\n");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Get the required extension count
    uint32_t extensionCount;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL)) {
        THROW_ERROR("Could not get instance extensions count\n");
    }

    const char *extensionNames[extensionCount + 1];
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount,
                                          extensionNames)) {
        THROW_ERROR("Could not get instance extensions names\n");
    }
    extensionNames[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = SIZEOF(extensionNames);
    createInfo.ppEnabledExtensionNames = extensionNames;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &vulkan->instance) != VK_SUCCESS) {
        THROW_ERROR("failed to create instance!\n");
    }
}

void initVulkan(Window *window, Vulkan *vulkan) {
    createInstance(window->win, vulkan);

    setupDebugMessenger(vulkan);

    createSurface(window, vulkan);

    pickPhysicalDevice(window, vulkan);

    createLogicalDevice(window, vulkan);

    createSwapChain(window, vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createColorResources(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(window, vulkan);

    // createTextureImage(vulkan);

    // createTextureImageView(vulkan);

    // createTextureSampler(vulkan);

    //

    Vertex top[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    Vertex bottom[] = {
        {{-0.5f, 0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}},
    };
    Vertex right[] = {
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Right
        {{0.5f, -0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    Vertex left[] = {
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // left
        {{-0.5f, 0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    Vertex front[] = {
        {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // front
        {{0.5f, 0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    Vertex back[] = {
        {{0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}}, // back
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}},
    };

    // combineVerticesAndIndices(vulkan, 2, (Shape){shape1, 4, NULL, 6, 0},
    //                           (Shape){shape2, 4, NULL, 6, 0});
    combineVerticesAndIndices(
        vulkan, 6, (Shape){top, 4, NULL, 6, 0}, (Shape){bottom, 4, NULL, 6, 0},
        (Shape){right, 4, NULL, 6, 0}, (Shape){left, 4, NULL, 6, 0},
        (Shape){front, 4, NULL, 6, 0}, (Shape){back, 4, NULL, 6, 0});

    // Vertex vertices[] = {
    //     {{+0.5f, +0.5f, +0.5f}, {+0.f, +1.f, +0.f}}, // Top
    //     {{-0.5f, +0.5f, +0.5f}, {+0.f, +1.f, +0.f}},
    //     {{+0.5f, +0.5f, -0.5f}, {+0.f, +1.f, +0.f}},
    //     {{-0.5f, +0.5f, -0.5f}, {+0.f, +1.f, +0.f}},
    //     {{+0.5f, -0.5f, +0.5f}, {+0.f, -1.f, +0.f}}, // Bottom
    //     {{-0.5f, -0.5f, +0.5f}, {+0.f, -1.f, +0.f}},
    //     {{+0.5f, -0.5f, -0.5f}, {+0.f, -1.f, +0.f}},
    //     {{-0.5f, -0.5f, -0.5f}, {+0.f, -1.f, +0.f}},
    //     {{+0.5f, +0.5f, +0.5f}, {+1.f, +0.f, +0.f}}, // Right
    //     {{+0.5f, +0.5f, -0.5f}, {+1.f, +0.f, +0.f}},
    //     {{+0.5f, -0.5f, -0.5f}, {+1.f, +0.f, +0.f}},
    //     {{+0.5f, -0.5f, +0.5f}, {+1.f, +0.f, +0.f}},
    //     {{-0.5f, +0.5f, +0.5f}, {-1.f, +0.f, +0.f}}, // left
    //     {{-0.5f, +0.5f, -0.5f}, {-1.f, +0.f, +0.f}},
    //     {{-0.5f, -0.5f, -0.5f}, {-1.f, +0.f, +0.f}},
    //     {{-0.5f, -0.5f, +0.5f}, {-1.f, +0.f, +0.f}},
    //     {{-0.5f, +0.5f, +0.5f}, {+0.f, +0.f, +1.f}}, // front
    //     {{+0.5f, +0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
    //     {{+0.5f, -0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
    //     {{-0.5f, -0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
    //     {{-0.5f, +0.5f, -0.5f}, {+0.f, +0.f, -1.f}}, // back
    //     {{+0.5f, +0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
    //     {{+0.5f, -0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
    //     {{-0.5f, -0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
    // };
    // uint32_t indices[] = {
    //     0,  1,  2,  1,  2,  3,  /*top*/
    //     4,  5,  6,  5,  6,  7,  /*bottom*/
    //     8,  9,  10, 8,  10, 11, /*right*/
    //     12, 13, 14, 12, 14, 15, /*left*/
    //     16, 17, 18, 16, 18, 19, /*front*/
    //     20, 21, 22, 20, 22, 23, /*back*/
    // };
    createVertexBuffer(vulkan);
    createIndexBuffer(vulkan);

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan, window);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Window *window, Vulkan *vulkan) {
    cleanupSwapChain(vulkan);

    // vkDestroySampler(vulkan->device.device, vulkan->texture.textureSampler,
    //                  NULL);
    // vkDestroyImageView(vulkan->device.device,
    // vulkan->texture.textureImageView,
    //                    NULL);

    // vkDestroyImage(vulkan->device.device, vulkan->texture.textureImage,
    // NULL); vkFreeMemory(vulkan->device.device,
    // vulkan->texture.textureImageMemory,
    //              NULL);

    vkDestroyDescriptorSetLayout(vulkan->device.device,
                                 vulkan->ubo.descriptorSetLayout, NULL);

    if (vulkan->graphicsPipeline.graphicsPipeline == VK_NULL_HANDLE) {
        vkDestroyPipeline(vulkan->device.device,
                          vulkan->graphicsPipeline.graphicsPipeline, NULL);
    }
    if (vulkan->graphicsPipeline.pipelineLayout == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vulkan->device.device,
                                vulkan->graphicsPipeline.pipelineLayout, NULL);
    }

    vkDestroyBuffer(vulkan->device.device, vulkan->shapeBuffers.indexBuffer,
                    NULL);
    vkFreeMemory(vulkan->device.device, vulkan->shapeBuffers.indexBufferMemory,
                 NULL);

    vkDestroyBuffer(vulkan->device.device, vulkan->shapeBuffers.vertexBuffer,
                    NULL);
    vkFreeMemory(vulkan->device.device, vulkan->shapeBuffers.vertexBufferMemory,
                 NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.renderFinishedSemaphores[i],
                           NULL);
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.imageAvailableSemaphores[i],
                           NULL);
        vkDestroyFence(vulkan->device.device,
                       vulkan->semaphores.inFlightFences[i], NULL);
    }

    freeMem(8, vulkan->ubo.descriptorSets, vulkan->shapes.vertices,
            vulkan->shapes.indices, vulkan->semaphores.renderFinishedSemaphores,
            vulkan->semaphores.imageAvailableSemaphores,
            vulkan->semaphores.inFlightFences,
            vulkan->semaphores.imagesInFlight,
            vulkan->renderBuffers.commandBuffers);

    vkDestroyCommandPool(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, NULL);

    vkDestroyDevice(vulkan->device.device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan->instance,
                                      vulkan->validation.debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(vulkan->instance, window->surface, NULL);
    vkDestroyInstance(vulkan->instance, NULL);
}
