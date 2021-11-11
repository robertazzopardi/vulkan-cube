#include "window/window.h"
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

#define WIDTH_INIT 1280
#define HEIGHT_INIT 720

inline void createSurface(Vulkan *vulkan) {
    if (!SDL_Vulkan_CreateSurface(vulkan->window.win, vulkan->instance,
                                  &vulkan->window.surface)) {
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

inline void initSDL() {
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

inline Window createWindow() {
    Window window = {0};
    window.win = SDL_CreateWindow(
        APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH_INIT,
        HEIGHT_INIT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    window.running = true;
    window.event = malloc(1 * sizeof(*window.event));
    window.lastUpdate = SDL_GetTicks();

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    return window;
}

static inline void handleUserInput(Window *window) {
    while (SDL_PollEvent(window->event)) {
        switch (window->event->type) {
        case SDL_QUIT:
            window->running = false;
            break;
        case SDL_MOUSEMOTION | SDL_MOUSEBUTTONDOWN: {
            int wx, wy;
            SDL_GetWindowSize(window->win, &wx, &wy);
            int x, y;
            SDL_GetMouseState(&x, &y);

            window->mX = x;
            window->mY = y;
            window->mX -= wx / 2;
            window->mY -= wy / 2;

            // printf("%d %d\n", window->mX, window->mY);
            break;
        }
        case SDL_MOUSEBUTTONUP:
            window->mX = 0;
            window->mY = 0;
            break;
        }
    }
}

inline void mainLoop(Vulkan *vulkan) {
    while (vulkan->window.running) {
        // Handle Events
        handleUserInput(&vulkan->window);

        // Physics
        uint32_t current = SDL_GetTicks();
        vulkan->window.dt = (current - vulkan->window.lastUpdate) / 1000.0f;
        // Update
        vulkan->window.lastUpdate = current;

        // Rendering
        drawFrame(vulkan);
    }
}
