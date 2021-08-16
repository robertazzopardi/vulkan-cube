#ifndef INCLUDE_WINDOW_WINDOW
#define INCLUDE_WINDOW_WINDOW

#include <stdbool.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

static const int MAX_FRAMES_IN_FLIGHT = 2;

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;

typedef struct Vulkan Vulkan;

typedef struct Window Window;

struct Window {
    SDL_Window *win;
    SDL_Event *event;
    size_t currentFrame;
    bool *framebufferResized;
    bool running;

    VkSurfaceKHR surface;
};

void start();

// void createInstance(SDL_Window *, Vulkan *);

void createTextureImage(Vulkan *);

void createSurface(Window *, Vulkan *);

typedef struct VkSurfaceCapabilitiesKHR VkSurfaceCapabilitiesKHR;
struct VkExtent2d;

struct VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR,
                                   SDL_Window *);

#endif /* INCLUDE_WINDOW_WINDOW */
