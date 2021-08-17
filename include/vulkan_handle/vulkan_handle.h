#ifndef INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE
#define INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE

#include "device.h"
#include "graphics_pipeline.h"
#include "instance.h"
#include "render.h"
#include "shape.h"
#include "swapchain.h"
#include "texture.h"
#include "uniforms.h"
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct Validation Validation;

typedef struct Window Window;

struct Vulkan {
    Instance instance;
    Validation *validation;
    Device device;
    VkSampleCountFlagBits msaaSamples;
    SwapChain swapchain;
    GraphicsPipeline graphicsPipeline;
    RenderBuffers renderBuffers;
    Semaphores semaphores;
    ShapeBuffers shapeBuffers;
    Shape shapes;
    Depth depth;
    UniformBufferObject ubo;
    Texture texture;
    bool framebufferResized;
};

void initVulkan(Window *, Vulkan *);

void cleanUpVulkan(Window *, Vulkan *);

VkFormat findDepthFormat(Vulkan *);

void copyBuffer(Vulkan *, VkBuffer, VkBuffer, VkDeviceSize);

VkCommandBuffer beginSingleTimeCommands(Vulkan *);

void endSingleTimeCommands(Vulkan *, VkCommandBuffer);

#endif /* INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE */
