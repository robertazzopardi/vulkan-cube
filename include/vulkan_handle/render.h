#ifndef INCLUDE_VULKAN_HANDLE_RENDER
#define INCLUDE_VULKAN_HANDLE_RENDER

#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct Window Window;

typedef union SDL_Event SDL_Event;

typedef struct Buffers Buffers;

struct Buffers {
    VkFramebuffer *swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
};

typedef struct Semaphores Semaphores;

struct Semaphores {
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkFence *imagesInFlight;
};

void createFramebuffers(Vulkan *);

void createCommandPool(Window *, Vulkan *);

void createCommandBuffers(Vulkan *, Window *);

void drawFrame(Window *, SDL_Event, Vulkan *, float);

void createSyncObjects(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_RENDER */
