#ifndef INCLUDE_WINDOW_WINDOW
#define INCLUDE_WINDOW_WINDOW

#include <stdbool.h>
#include <stddef.h>
#include <vulkan/vulkan.h>

#define APP_NAME "Vulkan App"

static const int MAX_FRAMES_IN_FLIGHT = 2;

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef struct Vulkan Vulkan;
typedef struct Window Window;
typedef struct VkSurfaceCapabilitiesKHR VkSurfaceCapabilitiesKHR;
typedef struct VkExtent2d VkExtent2d;

struct Window {
    SDL_Window *win;
    SDL_Event *event;
    size_t currentFrame;
    bool windowResized;
    bool running;

    VkSurfaceKHR surface;
};

void initialise();

void createSurface(Window *, Vulkan *);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR, SDL_Window *);

#endif /* INCLUDE_WINDOW_WINDOW */
