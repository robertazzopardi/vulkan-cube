#ifndef INCLUDE_VULKAN_HANDLE_UNIFORMS
#define INCLUDE_VULKAN_HANDLE_UNIFORMS

typedef float vec4[4];
typedef vec4 mat4[4];

typedef struct VkDescriptorSetLayout_T *VkDescriptorSetLayout;
typedef struct VkBuffer_T *VkBuffer;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkDescriptorPool_T *VkDescriptorPool;
typedef struct VkDescriptorSet_T *VkDescriptorSet;

typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;

    VkDescriptorSetLayout descriptorSetLayout;

    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
} UniformBufferObject;

typedef struct Vulkan Vulkan;

typedef unsigned int uint32_t;

void updateUniformBuffer(Vulkan *, uint32_t, float);

void createDescriptorSetLayout(Vulkan *);

void createUniformBuffers(Vulkan *);

void createDescriptorPool(Vulkan *);

void createDescriptorSets(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_UNIFORMS */
