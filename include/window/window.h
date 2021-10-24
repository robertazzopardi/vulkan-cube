#ifndef INCLUDE_WINDOW_WINDOW
#define INCLUDE_WINDOW_WINDOW

#include <stdbool.h>

#define APP_NAME "Vulkan App"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct SDL_Window SDL_Window;
typedef union SDL_Event SDL_Event;
typedef struct Vulkan Vulkan;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

typedef unsigned int uint32_t;

typedef struct Window {
    SDL_Window *win;
    SDL_Event *event;
    bool windowResized;
    bool running;
    VkSurfaceKHR surface;
    float dt;
    int mX;
    int mY;
    int wW;
    int wH;
    bool lmbDown;
    uint32_t lastUpdate;
} Window;

void initSDL();

Window createWindow();

void createSurface(Vulkan *);

#endif /* INCLUDE_WINDOW_WINDOW */
