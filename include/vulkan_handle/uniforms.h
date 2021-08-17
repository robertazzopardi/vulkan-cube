#ifndef INCLUDE_VULKAN_HANDLE_UNIFORMS
#define INCLUDE_VULKAN_HANDLE_UNIFORMS

#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct UniformBufferObject UniformBufferObject;

struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;

    VkDescriptorSetLayout descriptorSetLayout;

    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
};

void createDescriptorSetLayout(Vulkan *);

void createUniformBuffers(Vulkan *);

void updateUniformBuffer(Vulkan *, uint32_t, float);

void createDescriptorPool(Vulkan *);

void createDescriptorSets(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_UNIFORMS */
