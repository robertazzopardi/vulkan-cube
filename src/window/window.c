#include "window.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
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
    if (!SDL_Vulkan_CreateSurface(window->win, vulkan->instance,
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
            mainWindow->windowResized = true;

            // SDL_GetWindowSize(win, &mainWindow->width, &mainWindow->height);
        }
    }

    return 0;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,
                            SDL_Window *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

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

static Window createWindow() {
    Window window = {0};
    window.win = SDL_CreateWindow(
        APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH_INIT,
        HEIGHT_INIT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    window.running = true;
    window.windowResized = false;
    return window;
}

void initialise() {
    initSDL();

    // Set Up Window
    Window window = createWindow();

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    // INIT VULKAN
    Vulkan vulkan = {0};
    initVulkan(&window, &vulkan);

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
