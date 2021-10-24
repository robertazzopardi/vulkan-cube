#ifndef INCLUDE_VULKAN_HANDLE_UNIFORMS
#define INCLUDE_VULKAN_HANDLE_UNIFORMS

#include <cglm/types.h>

typedef struct VkDescriptorSetLayout_T *VkDescriptorSetLayout;
typedef struct VkBuffer_T *VkBuffer;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkDescriptorPool_T *VkDescriptorPool;
typedef struct VkDescriptorSet_T *VkDescriptorSet;

typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct DescriptorSet {
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
    VkDescriptorSetLayout descriptorSetLayout;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;
} DescriptorSet;

typedef struct Vulkan Vulkan;

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint64_t VkDeviceSize;

void updateUniformBuffer(Vulkan *, uint32_t);

void createDescriptorSetLayout(Vulkan *);

void createUniformBuffers(Vulkan *);

void createDescriptorPool(Vulkan *);

void createDescriptorSets(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_UNIFORMS */
