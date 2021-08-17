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

// typedef struct Device Device;

struct Vulkan {
    // VkInstance instance;
    Instance instance;

    // VkDebugUtilsMessengerEXT debugMessenger;
    Validation *validation;

    Device device;

    // VkSurfaceKHR surface;

    VkSampleCountFlagBits msaaSamples;

    // VkQueue graphicsQueue;
    // VkQueue presentQueue;

    // VkSwapchainKHR swapChain;
    // VkImage *swapChainImages;
    // uint32_t swapChainImagesCount;
    // VkFormat swapChainImageFormat;
    // VkExtent2D swapChainExtent;
    // VkImageView *swapChainImageViews;
    // VkFramebuffer *swapChainFramebuffers;
    SwapChain swapchain;

    // VkRenderPass renderPass;

    // VkDescriptorSetLayout descriptorSetLayout;

    // VkPipelineLayout pipelineLayout;
    // VkPipeline graphicsPipeline;
    GraphicsPipeline graphicsPipeline;

    RenderBuffers renderBuffers;

    Semaphores semaphores;

    // VkBuffer vertexBuffer;
    // VkDeviceMemory vertexBufferMemory;
    // VkBuffer indexBuffer;
    // VkDeviceMemory indexBufferMemory;
    ShapeBuffers shapeBuffers;

    Shape shapes;

    Depth depth;

    UniformBufferObject ubo;

    Texture texture;

    // VkCommandPool commandPool;

    // VkImage colorImage;
    // VkDeviceMemory colorImageMemory;
    // VkImageView colorImageView;

    // VkImage depthImage;
    // VkDeviceMemory depthImageMemory;
    // VkImageView depthImageView;

    // VkCommandBuffer *commandBuffers;

    // VkSemaphore *imageAvailableSemaphores;
    // VkSemaphore *renderFinishedSemaphores;
    // VkFence *inFlightFences;
    // VkFence *imagesInFlight;

    bool framebufferResized;
};

void initVulkan(Window *, Vulkan *);

void cleanUpVulkan(Window *, Vulkan *);

VkFormat findDepthFormat(Vulkan *);

void copyBuffer(Vulkan *, VkBuffer, VkBuffer, VkDeviceSize);

VkCommandBuffer beginSingleTimeCommands(Vulkan *);

void endSingleTimeCommands(Vulkan *, VkCommandBuffer);

#endif /* INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE */
