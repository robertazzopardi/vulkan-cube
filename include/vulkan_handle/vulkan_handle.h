#ifndef INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE
#define INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE

#include "device.h"
#include "instance.h"
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct {
    vec3 pos;
    vec3 colour;
    vec2 texCoord;
} Vertex;

typedef struct {
    Vertex *vertices;
    uint32_t verticesCount;
    uint16_t *indices;
    uint16_t indicesCount;
    uint32_t index;
} Shape;

typedef struct Vulkan Vulkan;

typedef struct Validation Validation;

// typedef struct Device Device;

struct Vulkan {
    // VkInstance instance;
    Instance instance;

    // VkDebugUtilsMessengerEXT debugMessenger;
    Validation *validation;

    Device device;

    VkSurfaceKHR surface;

    VkSampleCountFlagBits msaaSamples;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkImageView *swapChainImageViews;

    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    Shape shapes;
    uint32_t shapeCount;

    UniformBufferObject ubo;

    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkCommandPool commandPool;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkCommandBuffer *commandBuffers;

    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkFence *imagesInFlight;

    bool framebufferResized;
};

typedef struct Window Window;

void initVulkan(Window *, Vulkan *);

void cleanUpVulkan(Vulkan *);

void createGraphicsPipeline(Vulkan *);

void createColorResources(Vulkan *);

void createDepthResources(Vulkan *);

void createFramebuffers(Vulkan *);

void createUniformBuffers(Vulkan *);

void updateUniformBuffer(Vulkan *, uint32_t, float);

void createDescriptorPool(Vulkan *);

void createDescriptorSets(Vulkan *);

void createRenderPass(Vulkan *);

void createImageViews(Vulkan *);

typedef struct SDL_Window SDL_Window;
void createSwapChain(SDL_Window *, Vulkan *);

void cleanupSwapChain(Vulkan *);

void generateMipmaps(Vulkan *, VkImage, VkFormat, int32_t, int32_t, uint32_t);

void copyBufferToImage(Vulkan *, VkBuffer, VkImage, uint32_t, uint32_t);

void transitionImageLayout(Vulkan *, VkImage, VkFormat __unused, VkImageLayout,
                           VkImageLayout, uint32_t);

void createImage(uint32_t, uint32_t, uint32_t, VkSampleCountFlagBits, VkFormat,
                 VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags,
                 Vulkan *, VkImage *, VkDeviceMemory *);

void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                  Vulkan *, VkBuffer *, VkDeviceMemory *);

#endif /* INCLUDE_VULKAN_HANDLE_VULKAN_HANDLE */
