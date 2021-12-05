#ifndef INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE
#define INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE

#include "device.h"
#include "geometry/geometry.h"
#include "render.h"
#include "swapchain.h"
#include "uniforms.h"
#include "validation.h"
#include "window/window.h"
#include <vulkan/vulkan.h>

typedef struct Vulkan {
    VkInstance instance;
    Validation validation;
    Device device;
    VkSampleCountFlagBits msaaSamples;
    SwapChain swapchain;
    VkRenderPass renderPass;
    size_t currentFrame;
    RenderBuffers renderBuffers;
    Semaphores semaphores;
    ShapeBuffers shapeBuffers;
    Shape *shapes;
    uint32_t shapeCount;
    UniformBufferObject ubo;
    Camera camera;
    Window window;

    Resource colour;
    Resource depth;
} Vulkan;

extern Vulkan initialise();

extern void terminate(Vulkan *);

extern void mainLoop(Vulkan *);

extern void generateShape(Vulkan *, ShapeType, const char *);

#endif /* INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE */
