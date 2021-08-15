#include "window.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/instance.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_pixels.h>
#include <SDL_vulkan.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

const float FRAME_DELAY = 1000.0f / 60.0f;
#define WIDTH_INIT 1280
#define HEIGHT_INIT 720

void createSurface(SDL_Window *window, Vulkan *vulkan) {
    if (!SDL_Vulkan_CreateSurface(window, vulkan->instance.instance,
                                  &vulkan->surface)) {
        // failed to create a surface!
        THROW_ERROR("failed to create window surface!\n");
    }
}

static int resizingEventCallback(void *data, SDL_Event *event) {
    Window *mainWindow = (Window *)data;

    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window *win = SDL_GetWindowFromID(event->window.windowID);

        if (win == mainWindow->win) {
            // printf("resizing.....\n");
            *mainWindow->framebufferResized = true;

            // SDL_GetWindowSize(win, &mainWindow->width, &mainWindow->height);
        }
    }

    return 0;
}

// void createInstance(SDL_Window *window, Vulkan *vulkan) {
//     vulkan->physicalDevice = VK_NULL_HANDLE;
//     vulkan->msaaSamples = VK_SAMPLE_COUNT_1_BIT;

//     if (enableValidationLayers && !checkValidationLayerSupport()) {
//         THROW_ERROR("validation layers requested, but not available!\n");
//     }

//     VkApplicationInfo appInfo = {};
//     appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//     appInfo.pApplicationName = APP_NAME;
//     appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//     appInfo.pEngineName = "No Engine";
//     appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//     appInfo.apiVersion = VK_API_VERSION_1_0;

//     // Get the required extension count
//     uint32_t extensionCount;
//     if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL)) {
//         THROW_ERROR("Could not get instance extensions count\n");
//     }

//     const char *extensionNames[extensionCount + 1];
//     if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount,
//                                           extensionNames)) {
//         THROW_ERROR("Could not get instance extensions names\n");
//     }
//     extensionNames[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

//     VkInstanceCreateInfo createInfo = {};
//     createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//     createInfo.pApplicationInfo = &appInfo;
//     createInfo.enabledExtensionCount = SIZEOF(extensionNames);
//     createInfo.ppEnabledExtensionNames = extensionNames;

//     VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
//     if (enableValidationLayers) {
//         createInfo.enabledLayerCount = SIZEOF(validationLayers);
//         createInfo.ppEnabledLayerNames = validationLayers;

//         populateDebugMessengerCreateInfo(&debugCreateInfo);
//         createInfo.pNext =
//             (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
//     } else {
//         createInfo.enabledLayerCount = 0;

//         createInfo.pNext = NULL;
//     }

//     if (vkCreateInstance(&createInfo, NULL, &vulkan->instance) != VK_SUCCESS)
//     {
//         THROW_ERROR("failed to create instance!\n");
//     }
// }

void createTextureImage(Vulkan *vulkan) {
    SDL_Surface *image =
        IMG_Load("/Users/rob/Pictures/Affs/20160312_211430_Original.jpg");

    // convert to desired format
    image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ABGR8888, 0);

    VkDeviceSize imageSize = image->w * image->h * image->format->BytesPerPixel;

    vulkan->mipLevels =
        (uint32_t)(floor(log2(glm_max(image->w, image->h)))) + 1;

    if (!image) {
        printf("Could not load texture: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device.device, stagingBufferMemory, 0, imageSize, 0,
                &data);
    memcpy(data, image->pixels, (size_t)imageSize);
    vkUnmapMemory(vulkan->device.device, stagingBufferMemory);

    createImage(image->w, image->h, vulkan->mipLevels, VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->textureImage, &vulkan->textureImageMemory);

    transitionImageLayout(vulkan, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          vulkan->mipLevels);
    copyBufferToImage(vulkan, stagingBuffer, vulkan->textureImage,
                      (uint32_t)image->w, (uint32_t)image->h);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);

    generateMipmaps(vulkan, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                    image->w, image->h, vulkan->mipLevels);

    SDL_FreeSurface(image);
}

void recreateSwapChain(Window *window, SDL_Event event, Vulkan *vulkan) {
    int width = 0, height = 0;
    SDL_Vulkan_GetDrawableSize(window->win, &width, &height);
    while (width == 0 || height == 0) {
        SDL_Vulkan_GetDrawableSize(window->win, &width, &height);
        SDL_PollEvent(&event);
    }

    vkDeviceWaitIdle(vulkan->device.device);

    cleanupSwapChain(vulkan);

    createSwapChain(window->win, vulkan);
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

    freeMem(1, vulkan->imagesInFlight);

    vulkan->imagesInFlight =
        calloc(vulkan->swapChainImagesCount, sizeof(*vulkan->imagesInFlight));
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,
                            SDL_Window *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

        // clamp(&actualExtent.width, capabilities.minImageExtent.width,
        //       capabilities.maxImageExtent.width);
        // clamp(&actualExtent.height, capabilities.minImageExtent.height,
        //       capabilities.maxImageExtent.height);
        glm_clamp(actualExtent.width, capabilities.minImageExtent.width,
                  capabilities.maxImageExtent.width);
        glm_clamp(actualExtent.height, capabilities.minImageExtent.height,
                  capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void createCommandBuffers(Vulkan *vulkan, Window *window) {
    vulkan->commandBuffers =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->commandBuffers));

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vulkan->swapChainImagesCount;

    if (vkAllocateCommandBuffers(vulkan->device.device, &allocInfo,
                                 vulkan->commandBuffers) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate command buffers!\n");
    }

    int width, height;
    SDL_GetWindowSize(window->win, &width, &height);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // viewport.width = window.width;
    // viewport.height = window.height;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = (VkOffset2D){0, 0};
    // scissor.extent = (VkExtent2D){window.width, window.height};
    scissor.extent = (VkExtent2D){width, height};

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(vulkan->commandBuffers[i], &beginInfo) !=
            VK_SUCCESS) {
            THROW_ERROR("failed to begin recording command buffer!\n");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkan->renderPass;
        renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
        renderPassInfo.renderArea.extent = vulkan->swapChainExtent;
        renderPassInfo.framebuffer = vulkan->swapChainFramebuffers[i];

        VkClearValue clearValues[] = {{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
                                      {{{1.0f, 0}}}};

        renderPassInfo.clearValueCount = SIZEOF(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(vulkan->commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(vulkan->commandBuffers[i], 0, 1, &viewport);

        vkCmdSetScissor(vulkan->commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(vulkan->commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vulkan->graphicsPipeline);

        VkBuffer vertexBuffers[] = {vulkan->vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(vulkan->commandBuffers[i], 0, 1, vertexBuffers,
                               offsets);

        vkCmdBindIndexBuffer(vulkan->commandBuffers[i], vulkan->indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(
            vulkan->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
            vulkan->pipelineLayout, 0, 1, &vulkan->descriptorSets[i], 0, NULL);

        // printf("%lu %u\n", SIZEOF(indices), vulkan->indicesCount);
        vkCmdDrawIndexed(vulkan->commandBuffers[i],
                         vulkan->shapes.indicesCount
                         // SIZEOF(indices)
                         ,
                         1, 0, 0, 0);

        vkCmdEndRenderPass(vulkan->commandBuffers[i]);

        if (vkEndCommandBuffer(vulkan->commandBuffers[i]) != VK_SUCCESS) {
            THROW_ERROR("failed to record command buffer!\n");
        }
    }
}

void drawFrame(Window *window, SDL_Event event, Vulkan *vulkan, float dt) {
    vkWaitForFences(vulkan->device.device, 1,
                    &vulkan->inFlightFences[window->currentFrame], VK_TRUE,
                    UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vulkan->device.device, vulkan->swapChain, UINT64_MAX,
        vulkan->imageAvailableSemaphores[window->currentFrame], VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(window, event, vulkan);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_ERROR("failed to acquire swap chain image!\n");
    }

    updateUniformBuffer(vulkan, imageIndex, dt);

    if (vulkan->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vulkan->device.device, 1,
                        &vulkan->imagesInFlight[imageIndex], VK_TRUE,
                        UINT64_MAX);
    }
    vulkan->imagesInFlight[imageIndex] =
        vulkan->inFlightFences[window->currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        vulkan->imageAvailableSemaphores[window->currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan->commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {
        vulkan->renderFinishedSemaphores[window->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(vulkan->device.device, 1,
                  &vulkan->inFlightFences[window->currentFrame]);

    if (vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo,
                      vulkan->inFlightFences[window->currentFrame]) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to submit draw command buffer!\n");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkan->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkan->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        vulkan->framebufferResized) {
        vulkan->framebufferResized = false;
        recreateSwapChain(window, event, vulkan);
    } else if (result != VK_SUCCESS) {
        THROW_ERROR("failed to present swap chain image!\n");
    }

    window->currentFrame = (window->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void initSDL() {
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // init SDL Image
    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
        SDL_Log("Unable to initialize SDL_Image: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void cleanupSDL(SDL_Window *win) {
    SDL_DestroyWindow(win);

    IMG_Quit();

    SDL_Quit();
}

void start() {
    initSDL();

    // Set Up Window
    Window window = {0};
    window.win = SDL_CreateWindow(
        APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH_INIT,
        HEIGHT_INIT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    window.running = true;

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    // INIT VULKAN
    Vulkan vulkan = {0};
    initVulkan(&window, &vulkan);

    window.framebufferResized = &vulkan.framebufferResized;

    // MAIN LOOP
    SDL_Event event;

    uint64_t start = SDL_GetPerformanceCounter();
    float dt;

    while (window.running) {
        // Delta Time
        uint64_t last = start;
        start = SDL_GetPerformanceCounter();
        dt = (((start - last) * 1000) / SDL_GetPerformanceFrequency()) * 0.001;

        // Handle Events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                window.running = false;
                break;
            }
        }

        // Update

        // Drawing
        drawFrame(&window, event, &vulkan, dt);

        SDL_Delay(floor(FRAME_DELAY - dt));
    }

    vkDeviceWaitIdle(vulkan.device.device);

    // CLEAN UP
    cleanUpVulkan(&vulkan);

    cleanupSDL(window.win);
}
