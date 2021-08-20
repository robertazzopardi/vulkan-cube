#ifndef INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE
#define INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE

#include "device.h"
#include "graphics_pipeline.h"
#include "render.h"
#include "shape.h"
#include "swapchain.h"
#include "texture.h"
#include "uniforms.h"
#include <validation.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan {
    VkInstance instance;
    Validation validation;
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
} Vulkan;

typedef struct Window Window;

void initVulkan(Window *, Vulkan *);

void cleanUpVulkan(Window *, Vulkan *);

VkFormat findDepthFormat(Vulkan *);

VkCommandBuffer beginSingleTimeCommands(Vulkan *);

void endSingleTimeCommands(Vulkan *, VkCommandBuffer);

void copyBuffer(Vulkan *, VkBuffer, VkBuffer, VkDeviceSize);

#endif /* INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE */
