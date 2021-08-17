#include "window.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/image_view.h"
#include "vulkan_handle/instance.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/swapchain.h"
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

void createSurface(Window *window, Vulkan *vulkan) {
    if (!SDL_Vulkan_CreateSurface(window->win, vulkan->instance.instance,
                                  &window->surface)) {
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

void transitionImageLayout(Vulkan *vulkan, VkImage image,
                           VkFormat format __unused, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        THROW_ERROR("unsupported layout transition!\n");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         NULL, 0, NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
}

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
    cleanUpVulkan(&window, &vulkan);

    cleanupSDL(window.win);
}
