#ifndef INCLUDE_VULKAN_HANDLE_SHAPE
#define INCLUDE_VULKAN_HANDLE_SHAPE

#include <cglm/cglm.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct Vertex Vertex;

typedef struct Shape Shape;

typedef struct Depth Depth;

struct Vertex {
    vec3 pos;
    vec3 colour;
    vec2 texCoord;
};

struct Depth {
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};

struct Shape {
    Vertex *vertices;
    uint32_t verticesCount;

    uint16_t *indices;
    uint16_t indicesCount;

    uint32_t index;
    // uint32_t shapeCount;
};

typedef struct ShapeBuffers ShapeBuffers;

struct ShapeBuffers {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
};

static Vertex shape1[] = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};

static Vertex shape2[] = {
    {{-0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};

VkVertexInputBindingDescription getBindingDescription();

VkVertexInputAttributeDescription *getAttributeDescriptions();

void combineVerticesAndIndices(Vulkan *, uint32_t, ...);

void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                  Vulkan *, VkBuffer *, VkDeviceMemory *);

void createVertexBuffer(Vulkan *);

void createIndexBuffer(Vulkan *);

void createDepthResources(Vulkan *);

void createFramebuffers(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_SHAPE */
