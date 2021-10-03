#include "geometry/geometry.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec3.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

typedef Vertex Plane[4];
typedef Vertex Triangle[3];
typedef Triangle **Sphere;

#define Y (1.0 / M_SQRT2)
#define X .525731112119133606
#define Z .850650808352039932

typedef Triangle Icosahedron[20];
Icosahedron icosahedron1 = {
    {
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT},
    },
};

typedef Triangle Octahedron[8];
Octahedron octahedron1 = {
    {
        {{0.0f, Y, 0.0f}, RED, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, Y}, RED, GLM_VEC3_ZERO_INIT},
        {{Y, 0.0f, 0.0f}, RED, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, -Y, 0.0f}, BLUE, GLM_VEC3_ZERO_INIT},
        {{Y, 0.0f, 0.0f}, BLUE, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, Y}, BLUE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, -Y, 0.0f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, Y}, GREEN, GLM_VEC3_ZERO_INIT},
        {{-Y, 0.0f, 0.0f}, GREEN, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, Y, 0.0f}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-Y, 0.0f, 0.0f}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, Y}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, -Y, 0.0f}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, -Y}, WHITE, GLM_VEC3_ZERO_INIT},
        {{Y, 0.0f, 0.0f}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{Y, 0.0f, 0.0f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, -Y}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.0f, Y, 0.0f}, GREEN, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, -Y, 0.0f}, RED, GLM_VEC3_ZERO_INIT},
        {{-Y, 0.0f, 0.0f}, RED, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, -Y}, RED, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.0f, Y, 0.0f}, BLUE, GLM_VEC3_ZERO_INIT},
        {{0.0f, 0.0f, -Y}, BLUE, GLM_VEC3_ZERO_INIT},
        {{-Y, 0.0f, 0.0f}, BLUE, GLM_VEC3_ZERO_INIT},
    },
};

typedef Plane Cube[6];
Cube cube1 = {
    {
        {{-0.5f, -0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT}, // top
        {{0.5f, -0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT},
        {{0.5f, 0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT},
        {{-0.5f, 0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-0.5f, 0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT}, // bottom
        {{0.5f, 0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.5f, -0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{-0.5f, -0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
    },
    {
        {{-0.5f, 0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT}, // left
        {{-0.5f, 0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT},
        {{-0.5f, -0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT},
        {{-0.5f, -0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.5f, -0.5f, 0.5f}, GREEN, GLM_VEC3_ZERO_INIT}, // Right
        {{0.5f, -0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.5f, 0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
        {{0.5f, 0.5f, 0.5f}, GREEN, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.5f, 0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT}, // front
        {{0.5f, 0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT},
        {{-0.5f, 0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT},
        {{-0.5f, 0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT},
    },
    {
        {{0.5f, -0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT}, // back
        {{0.5f, -0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT},
        {{-0.5f, -0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT},
        {{-0.5f, -0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT},
    },
};

void getNormal(vec3 p1, vec3 p2, vec3 p3, vec3 normal) {
    vec3 a;
    glm_vec3_sub(p3, p2, a);
    vec3 b;
    glm_vec3_sub(p1, p2, b);

    glm_vec3_cross(a, b, normal);
}

void normalize(vec3 a, Vertex *b, float length) {
    // get the distance between a and b along the x and y axes
    vec3 d;
    glm_vec3_sub((*b).pos, a, d);

    // right now, sqrt(dx^2 + dy^2) = distance(a,b).
    // we want to modify them so that sqrt(dx^2 + dy^2) = the given length.
    float norm = length / glm_vec3_distance(a, (*b).pos);

    glm_vec3_scale(d, norm, d);

    glm_vec3_add(a, d, (*b).pos);
}

static inline void getMiddlePoint(vec3 point1, vec3 point2, vec3 res) {
    res[0] = (point1[0] + point2[0]) / 2.0f;
    res[1] = (point1[1] + point2[1]) / 2.0f;
    res[2] = (point1[2] + point2[2]) / 2.0f;
}

void SplitTriangle(Triangle tri, Triangle *split) {
    vec3 a, b, c;
    getMiddlePoint(tri[0].pos, tri[1].pos, a);
    getMiddlePoint(tri[1].pos, tri[2].pos, b);
    getMiddlePoint(tri[2].pos, tri[0].pos, c);

    glm_vec3_copy(tri[0].pos, split[0][0].pos);
    glm_vec3_copy(a, split[0][1].pos);
    glm_vec3_copy(c, split[0][2].pos);

    glm_vec3_copy(tri[1].pos, split[1][0].pos);
    glm_vec3_copy(b, split[1][1].pos);
    glm_vec3_copy(a, split[1][2].pos);

    glm_vec3_copy(tri[2].pos, split[2][0].pos);
    glm_vec3_copy(c, split[2][1].pos);
    glm_vec3_copy(b, split[2][2].pos);

    glm_vec3_copy(a, split[3][0].pos);
    glm_vec3_copy(b, split[3][1].pos);
    glm_vec3_copy(c, split[3][2].pos);
}

VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

VkVertexInputAttributeDescription *getAttributeDescriptions() {

    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(3 * sizeof(*attributeDescriptions));

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, colour);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, normal);

    // attributeDescriptions[2].binding = 0;
    // attributeDescriptions[2].location = 2;
    // attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    // attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

bool isInVertexArray(Vertex vert, Vertex *arr, uint32_t vertIndex) {
    for (uint32_t j = 0; j < vertIndex; j++) {
        if (glm_vec3_eqv(vert.pos, arr[j].pos)) {
            return true;
        }
    }

    return false;
}

void calculateIndicesForSphere(Shape *vulkanShape, size_t indices) {
    for (size_t i = vulkanShape->index; i < vulkanShape->index + indices; i++) {
        vulkanShape->indices[i] = i;
    }
    vulkanShape->index += indices;
}

void calculateIndicesForCube(Shape *vulkanShape, Vertex *shape, size_t length) {
    Vertex arr[length];
    uint32_t vertIndex = 0;

    uint16_t *indis = malloc((length + 2) * sizeof(*indis));
    uint32_t indisIndex = 0;

    for (uint32_t i = 0; i < length; i++) {
        Vertex vert = shape[i];

        if (!isInVertexArray(vert, arr, vertIndex)) {
            indis[indisIndex++] = vulkanShape->index++;
            arr[vertIndex++] = vert;
        }

        if (vertIndex % 3 == 0) {
            indis[indisIndex] = vulkanShape->index - 1;
            indisIndex++;
        }
    }

    indis[indisIndex++] = indis[0];

    memcpy(vulkanShape->indices + vulkanShape->indicesCount, indis,
           indisIndex * sizeof(*indis));

    freeMem(1, indis);
}

void findTriangles(Triangle triangle, int currentDepth, int depth,
                   Triangle *storage, size_t *index) {
    // Depth is reached.
    if (currentDepth == depth) {
        vec3 normal;
        getNormal(triangle[0].pos, triangle[1].pos, triangle[2].pos, normal);

        for (size_t i = 0; i < 3; i++) {
            glm_vec3_copy(normal, storage[(*index)]->normal);
            glm_vec3_copy(triangle[i].pos, storage[(*index)++]->pos);
        }
        return;
    }

    Triangle split[4];
    SplitTriangle(triangle, split);

    findTriangles(split[0], currentDepth + 1, depth, storage, index);
    findTriangles(split[1], currentDepth + 1, depth, storage, index);
    findTriangles(split[2], currentDepth + 1, depth, storage, index);
    findTriangles(split[3], currentDepth + 1, depth, storage, index);
}

static inline void allocateVerticesAndIndices(Vulkan *vulkan,
                                              size_t numVertices,
                                              size_t numIndices) {
    vulkan->shapes.vertices =
        malloc(numVertices * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(numIndices * sizeof(*vulkan->shapes.indices));
}

void combineVerticesAndIndicesForSphere(Vulkan *vulkan, Octahedron octahedron,
                                        size_t count, size_t depth) {
    size_t perFace = pow(4, depth);
    size_t verticesPerFace = perFace * 3;

    size_t numVertices = count * verticesPerFace;
    allocateVerticesAndIndices(vulkan, numVertices, numVertices);

    for (size_t j = 0; j < count; j++) {
        Vertex *face = octahedron[j];

        Triangle faceTriangles[verticesPerFace];

        size_t index = 0;
        findTriangles(face, 0, depth, faceTriangles, &index);

        Vertex quadrant[verticesPerFace];
        for (size_t i = 0; i < verticesPerFace; i++) {
            glm_vec3_copy(faceTriangles[i]->pos, quadrant[i].pos);

            vec3 a = {0.0f, 0.0f, 0.0f};
            normalize(a, &quadrant[i], 0.8);

            // glm_vec3_copy((vec3)WHITE, quadrant[i].colour);
            glm_vec3_copy(face->colour, quadrant[i].colour);
            // glm_vec3_copy(face[i].colour, quadrant[i].colour);
        }

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, quadrant,
               verticesPerFace * sizeof(*quadrant));

        vulkan->shapes.verticesCount += verticesPerFace;

        calculateIndicesForSphere(&vulkan->shapes, verticesPerFace);

        vulkan->shapes.indicesCount += verticesPerFace;
    }
}

void combineVerticesAndIndicesForCube(Vulkan *vulkan, Cube cube, size_t count) {
    allocateVerticesAndIndices(vulkan, count * 4, count * 6);

    for (uint32_t i = 0; i < count; i++) {
        Vertex *shape = cube[i];

        vec3 normal;
        getNormal(shape[0].pos, shape[1].pos, shape[2].pos, normal);
        for (size_t j = 0; j < 4; j++) {
            glm_vec3_copy(normal, shape[j].normal);
        }

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, shape,
               4 * sizeof(*shape));

        vulkan->shapes.verticesCount += 4;

        calculateIndicesForCube(&vulkan->shapes, shape, 4);

        vulkan->shapes.indicesCount += 6;
    }
}

void createDepthResources(Vulkan *vulkan) {
    const VkFormat depthFormat = findDepthFormat(vulkan);

    createImage(vulkan->swapchain.swapChainExtent->width,
                vulkan->swapchain.swapChainExtent->height, 1,
                vulkan->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
                // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                vulkan, &vulkan->depth.depthImage,
                &vulkan->depth.depthImageMemory);

    vulkan->depth.depthImageView =
        createImageView(vulkan->device.device, vulkan->depth.depthImage,
                        depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void createFramebuffers(Vulkan *vulkan) {
    vulkan->renderBuffers.swapChainFramebuffers =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->renderBuffers.swapChainFramebuffers));

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkImageView attachments[] = {vulkan->texture.colorImageView,
                                     vulkan->depth.depthImageView,
                                     vulkan->swapchain.swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan->graphicsPipeline.renderPass;
        framebufferInfo.attachmentCount = SIZEOF(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkan->swapchain.swapChainExtent->width;
        framebufferInfo.height = vulkan->swapchain.swapChainExtent->height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                vulkan->device.device, &framebufferInfo, NULL,
                &vulkan->renderBuffers.swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            THROW_ERROR("failed to create framebuffer!\n");
        }
    }
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, Vulkan *vulkan,
                  VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkan->device.device, &bufferInfo, NULL, buffer) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create buffer!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkan->device.device, *buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkan, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkan->device.device, &allocInfo, NULL,
                         bufferMemory) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate buffer memory!\n");
    }

    vkBindBufferMemory(vulkan->device.device, *buffer, *bufferMemory, 0);
}

void createVertexBuffer(Vulkan *vulkan) {

    // for (size_t i = 0; i < SIZEOF(cube1); i++) {
    //     vec3 normal;
    //     getNormal(octahedron1[i][0].pos, octahedron1[i][1].pos,
    //     octahedron1[i][2].pos, normal);

    //     for (size_t j = 0; j < 4; j++) {
    //         glm_vec3_copy(normal, octahedron1[i][j].normal);
    //     }
    // }

    combineVerticesAndIndicesForCube(vulkan, cube1, SIZEOF(cube1));
    // combineVerticesAndIndicesForSphere(vulkan, octahedron1, 8, 0);
    // combineVerticesAndIndicesForSphere(vulkan, icosahedron1,
    //    SIZEOF(icosahedron1), 4);

    VkDeviceSize bufferSize =
        sizeof(*vulkan->shapes.vertices) * vulkan->shapes.verticesCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    mapMemory(vulkan->device.device, stagingBufferMemory, bufferSize,
              vulkan->shapes.vertices);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                 &vulkan->shapeBuffers.vertexBuffer,
                 &vulkan->shapeBuffers.vertexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->shapeBuffers.vertexBuffer,
               bufferSize);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);
}

void createIndexBuffer(Vulkan *vulkan) {
    VkDeviceSize bufferSize =
        sizeof(*vulkan->shapes.indices) * vulkan->shapes.indicesCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    mapMemory(vulkan->device.device, stagingBufferMemory, bufferSize,
              vulkan->shapes.indices);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                 &vulkan->shapeBuffers.indexBuffer,
                 &vulkan->shapeBuffers.indexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->shapeBuffers.indexBuffer,
               bufferSize);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);
}
