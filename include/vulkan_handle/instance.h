#ifndef INCLUDE_VULKAN_HANDLE_INSTANCE
#define INCLUDE_VULKAN_HANDLE_INSTANCE

#include <vulkan/vulkan.h>

#define APP_NAME "Vulkan App"

typedef struct Instance Instance;

struct Instance {
    VkInstance instance;
};

typedef struct SDL_Window SDL_Window;
typedef struct Vulkan Vulkan;

void createInstance(SDL_Window *, Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_INSTANCE */
