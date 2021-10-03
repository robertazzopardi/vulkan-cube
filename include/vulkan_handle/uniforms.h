#ifndef INCLUDE_VULKAN_HANDLE_UNIFORMS
#define INCLUDE_VULKAN_HANDLE_UNIFORMS

typedef float vec3[3];
typedef float vec4[4];
typedef vec4 mat4[4];

typedef struct VkDescriptorSetLayout_T *VkDescriptorSetLayout;
typedef struct VkBuffer_T *VkBuffer;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkDescriptorPool_T *VkDescriptorPool;
typedef struct VkDescriptorSet_T *VkDescriptorSet;

typedef struct UniformMVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformMVP;

typedef struct UniformLight {
    vec3 pos;
    vec3 colour;
} UniformLight;

typedef struct UniformBufferObject {
    UniformMVP mvp;
    UniformLight light;
} UniformBufferObject;

typedef struct DescriptorSet {
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;
    VkDescriptorSetLayout descriptorSetLayout;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;
} DescriptorSet;

typedef struct Vulkan Vulkan;
typedef struct Window Window;

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint64_t VkDeviceSize;

void updateUniformBuffer(Vulkan *, Window *, uint32_t, float);

void createDescriptorSetLayout(Vulkan *);

void createUniformBuffers(Vulkan *);

void createDescriptorPool(Vulkan *);

void createDescriptorSets(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_UNIFORMS */
