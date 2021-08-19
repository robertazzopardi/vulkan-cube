#ifndef INCLUDE_WINDOW_WINDOW
#define INCLUDE_WINDOW_WINDOW

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define APP_NAME "Vulkan App"

static const int MAX_FRAMES_IN_FLIGHT = 2;

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef struct Vulkan Vulkan;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

typedef struct Time {
    uint64_t now;
    uint64_t last;
    double dt;
} Time;

typedef struct Window {
    SDL_Window *win;
    SDL_Event *event;
    Time time;
    size_t currentFrame;
    bool windowResized;
    bool running;
    VkSurfaceKHR surface;
} Window;

void initialise();

void createSurface(Window *, Vulkan *);

typedef struct VkSurfaceCapabilitiesKHR VkSurfaceCapabilitiesKHR;
typedef struct VkExtent2D VkExtent2D;

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR, SDL_Window *);

#endif /* INCLUDE_WINDOW_WINDOW */
