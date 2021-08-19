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

static inline void initSDL() {
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

static inline void cleanup(Window *window) {
    freeMem(1, window->event);

    SDL_DestroyWindow(window->win);

    IMG_Quit();

    SDL_Quit();
}

static inline Window createWindow() {
    Window window = {0};
    window.win = SDL_CreateWindow(
        APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH_INIT,
        HEIGHT_INIT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    window.running = true;
    window.event = malloc(1 * sizeof(*window.event));
    window.time.now = SDL_GetPerformanceCounter();

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    return window;
}

static inline void calculateDeltaTime(Time *time) {
    time->last = time->now;
    time->now = SDL_GetPerformanceCounter();
    time->dt = (time->now - time->last) / (double)SDL_GetPerformanceFrequency();
}

static inline void handleUserInput(Window *window) {
    while (SDL_PollEvent(window->event)) {
        switch (window->event->type) {
        case SDL_QUIT:
            window->running = false;
            break;
        }
    }
}

void initialise() {
    initSDL();

    // Set Up Window
    Window window = createWindow();

    // Init Vulkan
    Vulkan vulkan = {0};
    initVulkan(&window, &vulkan);

    // Main Loop
    while (window.running) {
        // Calculate frame delay
        calculateDeltaTime(&window.time);

        // Frame rate limiting
        uint32_t timeout = SDL_GetTicks() + FRAME_DELAY;
        while (!SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) {
            // Handle Input
            handleUserInput(&window);

            // Update
            // printf("%.f\n", 1.0f / window.time.dt);

            // Rendering
            drawFrame(&window, &vulkan);
        }
    }

    vkDeviceWaitIdle(vulkan.device.device);

    // Clean Up
    cleanUpVulkan(&window, &vulkan);
    cleanup(&window);
}
