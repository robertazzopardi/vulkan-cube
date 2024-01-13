#ifndef INCLUDE_GEOMETRY_GEOMETRY
#define INCLUDE_GEOMETRY_GEOMETRY

#include "vulkan_handle/pipeline.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/uniforms.h"
#include <cglm/types.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum ShapeType {
    CUBE,
    SPHERE,
    ICOSPHERE,
    OCTASPHERE,
    CIRCLE,
    PLAIN,
    RING,
} ShapeType;

#define X .525731112119133606
#define Z .850650808352039932

#define RED                                                                    \
    { 1.f, 0.f, 0.f }
#define GREEN                                                                  \
    { 0.f, 1.f, 0.f }
#define BLUE                                                                   \
    { 0.f, 0.f, 1.f }
#define WHITE                                                                  \
    { 1.f, 1.f, 1.f }
#define BLACK                                                                  \
    { 0.f, 0.f, 0.f }

#define VEC_2(num)                                                             \
    { num, num }
#define VEC_3(num)                                                             \
    { num, num, num }
#define VEC_4(num)                                                             \
    { num, num, num, num }

#define MAT3_ROT_Z                                                             \
    ((vec3 *){                                                                 \
        {cos(GLM_PI_2f), -sin(GLM_PI_2f), 0.0f},                               \
        {sin(GLM_PI_2f), cos(GLM_PI_2f), 0.0f},                                \
        {0.0f, 0.0f, 1.0f},                                                    \
    })

#define MAT3_ROT_Y                                                             \
    ((vec3 *){                                                                 \
        {cos(GLM_PI_2f), 0.0f, sin(GLM_PI_2f)},                                \
        {0.0f, 1.0f, 0.0f},                                                    \
        {-sin(GLM_PI_2f), 0.0f, cos(GLM_PI_2f)},                               \
    })

#define MAT3_ROT_X                                                             \
    ((vec3 *){                                                                 \
        {1.0f, 0.0f, 0.0f},                                                    \
        {0.0f, cos(GLM_PI_2), -sin(GLM_PI_2)},                                 \
        {0.0f, sin(GLM_PI_2), cos(GLM_PI_2)},                                  \
    })

typedef struct Vertex {
    vec3 pos;
    vec3 colour;
    vec3 normal;
    vec2 texCoord;
} Vertex;

typedef Vertex Plane[4];
typedef Vertex Triangle[3];
typedef Triangle **Sphere;
typedef Triangle Icosahedron[20];
typedef Triangle Octahedron[8];
typedef Plane Cube[6];

typedef struct VkImage_T *VkImage;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkImageView_T *VkImageView;

typedef struct Resource {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
} Resource;

typedef struct ShapeBuffers {
    VkBuffer *vertexBuffer;
    VkDeviceMemory *vertexBufferMemory;
    VkBuffer *indexBuffer;
    VkDeviceMemory *indexBufferMemory;
} ShapeBuffers;

typedef struct Shape {
    Vertex *vertices;
    uint32_t verticesCount;

    uint16_t *indices;
    uint16_t indicesCount;

    uint32_t index;

    bool indexed;

    GraphicsPipeline graphicsPipeline;
    DescriptorSet descriptorSet;
    Texture texture;
    // ShapeBuffers buffers;
} Shape;

typedef struct VkBuffer_T *VkBuffer;

typedef struct Vulkan Vulkan;

typedef uint64_t VkDeviceSize;
typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;

void generateShape(Vulkan *, ShapeType, const char *);

void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                  Vulkan *, VkBuffer *, VkDeviceMemory *);

void createVertexBuffer(Vulkan *);

void createVertexIndexBuffer(Vulkan *, void *, uint64_t, VkBuffer *,
                             VkDeviceMemory *, VkBufferUsageFlags);

void createFramebuffers(Vulkan *);

typedef struct VkVertexInputBindingDescription VkVertexInputBindingDescription;
typedef struct VkVertexInputAttributeDescription
    VkVertexInputAttributeDescription;

VkVertexInputBindingDescription getBindingDescription();

VkVertexInputAttributeDescription *getAttributeDescriptions();

void createBufferAndMemory(Vulkan *, VkBuffer *, VkDeviceMemory *, Vertex *,
                           uint16_t);

void calculateNormals(Vertex *, uint32_t);

void normalize(vec3, Vertex *, float);

void getMiddlePoint(vec3, vec3, vec3);

// void bindVertexAndIndexBuffers(Vulkan *);

#endif /* INCLUDE_GEOMETRY_GEOMETRY */
