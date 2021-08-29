#ifndef INCLUDE_WINDOW_WINDOW
#define INCLUDE_WINDOW_WINDOW

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define APP_NAME "Vulkan App"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef struct Vulkan Vulkan;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

typedef struct Window {
    SDL_Window *win;
    SDL_Event *event;
    bool windowResized;
    bool running;
    VkSurfaceKHR surface;
    float dt;
} Window;

void initialise();

void createSurface(Window *, Vulkan *);

#endif /* INCLUDE_WINDOW_WINDOW */
