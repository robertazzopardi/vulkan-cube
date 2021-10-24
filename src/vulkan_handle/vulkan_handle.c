#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "vulkan_handle/vulkan_handle.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/swapchain.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/validation.h"
#include "window/window.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

void createInstance(Vulkan *vulkan) {
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
    if (!SDL_Vulkan_GetInstanceExtensions(vulkan->window.win, &extensionCount,
                                          NULL)) {
        THROW_ERROR("Could not get instance extensions count\n");
    }

    const char *extensionNames[extensionCount + 1];
    if (!SDL_Vulkan_GetInstanceExtensions(vulkan->window.win, &extensionCount,
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

void initVulkan(Vulkan *vulkan) {
    createInstance(vulkan);

    setupDebugMessenger(vulkan);

    createSurface(vulkan);

    pickPhysicalDevice(vulkan);

    createLogicalDevice(vulkan);

    createSwapChain(vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createColorResources(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(vulkan);

    createTextureImage(vulkan);

    createTextureImageView(vulkan);

    createTextureSampler(vulkan);

    generateShape(vulkan, SPHERE);
    createVertexIndexBuffer(vulkan, vulkan->shapes.vertices,
                            sizeof(*vulkan->shapes.vertices) *
                                vulkan->shapes.verticesCount,
                            &vulkan->shapeBuffers.vertexBuffer,
                            &vulkan->shapeBuffers.vertexBufferMemory);
    createVertexIndexBuffer(vulkan, vulkan->shapes.indices,
                            sizeof(*vulkan->shapes.indices) *
                                vulkan->shapes.indicesCount,
                            &vulkan->shapeBuffers.indexBuffer,
                            &vulkan->shapeBuffers.indexBufferMemory);

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Vulkan *vulkan) {
    vkDeviceWaitIdle(vulkan->device.device);

    cleanupSwapChain(vulkan);

    vkDestroySampler(vulkan->device.device, vulkan->texture.textureSampler,
                     NULL);
    vkDestroyImageView(vulkan->device.device, vulkan->texture.textureImageView,
                       NULL);

    vkDestroyImage(vulkan->device.device, vulkan->texture.textureImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->texture.textureImageMemory,
                 NULL);

    vkDestroyDescriptorSetLayout(
        vulkan->device.device, vulkan->descriptorSet.descriptorSetLayout, NULL);

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

    freeMem(6, vulkan->descriptorSet.descriptorSets,
            vulkan->semaphores.renderFinishedSemaphores,
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

    vkDestroySurfaceKHR(vulkan->instance, vulkan->window.surface, NULL);
    vkDestroyInstance(vulkan->instance, NULL);
}

inline Vulkan initialise() {
    initSDL();

    Vulkan vulkan = {0};
    vulkan.window = createWindow();

    initVulkan(&vulkan);

    return vulkan;
}

inline void terminate(Vulkan *vulkan) {
    cleanUpVulkan(vulkan);

    freeMem(1, vulkan->window.event);

    SDL_DestroyWindow(vulkan->window.win);

    IMG_Quit();

    SDL_Quit();
}
