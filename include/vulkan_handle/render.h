#ifndef INCLUDE_VULKAN_HANDLE_RENDER
#define INCLUDE_VULKAN_HANDLE_RENDER

typedef struct VkFramebuffer_T *VkFramebuffer;
typedef struct VkCommandPool_T *VkCommandPool;
typedef struct VkCommandBuffer_T *VkCommandBuffer;
typedef struct VkSemaphore_T *VkSemaphore;
typedef struct VkFence_T *VkFence;

typedef struct RenderBuffers {
    VkFramebuffer *swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
} RenderBuffers;

typedef struct Semaphores {
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkFence *imagesInFlight;
} Semaphores;

typedef struct Vulkan Vulkan;
typedef struct Window Window;

void createCommandPool(Window *, Vulkan *);

void createCommandBuffers(Vulkan *, Window *);

void drawFrame(Window *, Vulkan *);

void createSyncObjects(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_RENDER */
