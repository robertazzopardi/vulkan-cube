#include "vulkan_handle/shape.h"
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

typedef Vertex Square[4];
typedef Square Cube[6];
typedef Vertex Triangle[3];
typedef Triangle Octahedron[8];
typedef Triangle **Sphere;

#define Y (1.0 / M_SQRT2)

Octahedron octahedron1 = {
    {
        {{0.0f, Y, 0.0f}, RED},
        {{0.0f, 0.0f, Y}, RED},
        {{Y, 0.0f, 0.0f}, RED},
    },
    {
        {{0.0f, -Y, 0.0f}, BLUE},
        {{Y, 0.0f, 0.0f}, BLUE},
        {{0.0f, 0.0f, Y}, BLUE},
    },
    {
        {{0.0f, -Y, 0.0f}, GREEN},
        {{0.0f, 0.0f, Y}, GREEN},
        {{-Y, 0.0f, 0.0f}, GREEN},
    },
    {
        {{0.0f, Y, 0.0f}, WHITE},
        {{-Y, 0.0f, 0.0f}, WHITE},
        {{0.0f, 0.0f, Y}, WHITE},
    },

    {
        {{0.0f, -Y, 0.0f}, WHITE},
        {{0.0f, 0.0f, -Y}, WHITE},
        {{Y, 0.0f, 0.0f}, WHITE},
    },
    {
        {{Y, 0.0f, 0.0f}, GREEN},
        {{0.0f, 0.0f, -Y}, GREEN},
        {{0.0f, Y, 0.0f}, GREEN},
    },
    {
        {{0.0f, -Y, 0.0f}, RED},
        {{-Y, 0.0f, 0.0f}, RED},
        {{0.0f, 0.0f, -Y}, RED},
    },
    {
        {{0.0f, Y, 0.0f}, BLUE},
        {{0.0f, 0.0f, -Y}, BLUE},
        {{-Y, 0.0f, 0.0f}, BLUE},
    },
};

Cube cube1 = {
    {
        {{-0.5f, -0.5f, 0.5f}, WHITE}, // top
        {{0.5f, -0.5f, 0.5f}, WHITE},
        {{0.5f, 0.5f, 0.5f}, WHITE},
        {{-0.5f, 0.5f, 0.5f}, WHITE},
    },
    {
        {{-0.5f, 0.5f, -0.5f}, GREEN}, // bottom
        {{0.5f, 0.5f, -0.5f}, GREEN},
        {{0.5f, -0.5f, -0.5f}, GREEN},
        {{-0.5f, -0.5f, -0.5f}, GREEN},
    },
    {
        {{0.5f, -0.5f, 0.5f}, BLUE}, // Right
        {{0.5f, -0.5f, -0.5f}, BLUE},
        {{0.5f, 0.5f, -0.5f}, BLUE},
        {{0.5f, 0.5f, 0.5f}, BLUE},
    },
    {
        {{-0.5f, 0.5f, 0.5f}, RED}, // left
        {{-0.5f, 0.5f, -0.5f}, RED},
        {{-0.5f, -0.5f, -0.5f}, RED},
        {{-0.5f, -0.5f, 0.5f}, RED},
    },
    {
        {{0.5f, 0.5f, 0.5f}, RED}, // front
        {{0.5f, 0.5f, -0.5f}, RED},
        {{-0.5f, 0.5f, -0.5f}, RED},
        {{-0.5f, 0.5f, 0.5f}, RED},
    },
    {
        {{0.5f, -0.5f, -0.5f}, BLUE}, // back
        {{0.5f, -0.5f, 0.5f}, BLUE},
        {{-0.5f, -0.5f, 0.5f}, BLUE},
        {{-0.5f, -0.5f, -0.5f}, BLUE},
    },
};

void normalize(vec3 a, Vertex *b, float length) {
    //    #get the distance between a and b along the x and y axes
    float dx = (*b).pos[0] - a[0];
    float dy = (*b).pos[1] - a[1];
    float dz = (*b).pos[2] - a[2];
    // #right now, sqrt(dx^2 + dy^2) = distance(a,b).
    // #we want to modify them so that sqrt(dx^2 + dy^2) = the given length.
    dx = dx * length / glm_vec3_distance(a, (*b).pos);
    dy = dy * length / glm_vec3_distance(a, (*b).pos);
    dz = dz * length / glm_vec3_distance(a, (*b).pos);

    vec3 c = {a[0] + dx, a[1] + dy, a[2] + dz};
    // return c;
    memcpy((*b).pos, c, 3 * sizeof(*c));
}

void getMiddlePoint(vec3 point1, vec3 point2, vec3 res) {
    res[0] = (point1[0] + point2[0]) / 2.0f;
    res[1] = (point1[1] + point2[1]) / 2.0f;
    res[2] = (point1[2] + point2[2]) / 2.0f;
}

void SplitTriangle(Triangle tri, Triangle *split) {
    vec3 a;
    getMiddlePoint(tri[0].pos, tri[1].pos, a);
    vec3 b;
    getMiddlePoint(tri[1].pos, tri[2].pos, b);
    vec3 c;
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
    // VkVertexInputAttributeDescription *attributeDescriptions =
    //     malloc(3 * sizeof(*attributeDescriptions));

    // attributeDescriptions[0].binding = 0;
    // attributeDescriptions[0].location = 0;
    // attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // attributeDescriptions[1].binding = 0;
    // attributeDescriptions[1].location = 1;
    // attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDescriptions[1].offset = offsetof(Vertex, colour);

    // attributeDescriptions[2].binding = 0;
    // attributeDescriptions[2].location = 2;
    // attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    // attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(2 * sizeof(*attributeDescriptions));

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, colour);

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

void calculateIndicesForOctahedron(Shape *vulkanShape) {
    for (size_t i = vulkanShape->index; i < vulkanShape->index + 3; i++) {
        vulkanShape->indices[i] = i;
    }
    vulkanShape->index += 3;
}

void calculateIndicesForSquare(Shape *vulkanShape, Vertex *shape,
                               size_t length) {
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

    for (uint32_t i = 0; i < indisIndex; i++) {
        printf("%u ", indis[i]);
    }
    printf("\n");

    freeMem(1, indis);
}

void findTriangles(Triangle triangle, int currentDepth, int depth,
                   Triangle *storage, size_t *index) {
    // Depth is reached.
    if (currentDepth == depth) {

        for (size_t i = 0; i < 3; i++) {

            // glm_vec3_copy((vec3)WHITE, storage[*index][i].colour);
            memset(storage[*index][i].colour, 1,
                   3 * sizeof(*storage[*index][i].colour));

            // memcpy(storage[(*index)++][i].pos, triangle[i].pos,
            //        3 * sizeof(*triangle[i].pos));

            // glm_vec3_copy(triangle[i].pos, storage[(*index)++][i].pos);
            glm_vec3_copy(triangle[i].pos, storage[(*index)++]->pos);

            // printf("%f %f %f\n", storage[(*index) - 1][i].pos[0],
            //        storage[(*index) - 1][i].pos[1],
            //        storage[(*index) - 1][i].pos[2]);
        }

        return;
    }

    Triangle split[4];
    SplitTriangle(triangle, split);

    findTriangles(split[0], currentDepth + 1, depth, storage, index);
    findTriangles(split[1], currentDepth + 1, depth, storage, index);
    findTriangles(split[2], currentDepth + 1, depth, storage, index);
    findTriangles(split[3], currentDepth + 1, depth, storage, index);

    // printf("\n");
}

void combineVerticesAndIndicesForOctahedron(Vulkan *, Octahedron, size_t);

void combineVerticesAndIndicesForSphere(Vulkan *vulkan __unused,
                                        Octahedron octahedron, size_t count,
                                        size_t depth) {
    printf("depth %zu\n", depth);

    size_t perFace = pow(4, depth);
    printf("perface %zu\n", perFace);
    size_t verticesPerFace = perFace * 3;
    printf("verticesPerFace %zu\n", verticesPerFace);

    // if (perFace == 1) {
    //     combineVerticesAndIndicesForOctahedron(vulkan, octahedron, count);
    //     return;
    // }

    vulkan->shapes.vertices =
        malloc(count * verticesPerFace * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(count * verticesPerFace * sizeof(*vulkan->shapes.indices));

    for (size_t j = 0; j < count; j++) {
        Vertex *face = octahedron[j];

        Triangle *faceTriangles =
            malloc(verticesPerFace * sizeof(*faceTriangles));
        memset(faceTriangles, 0, verticesPerFace * sizeof(*faceTriangles));
        size_t index = 0;
        findTriangles(face, 0, depth, faceTriangles, &index);

        // printf("\n");

        Vertex v[verticesPerFace];
        for (size_t i = 0; i < verticesPerFace; i++) {

            glm_vec3_copy(faceTriangles[i]->pos, v[i].pos);

            vec3 a = {0.0f, 0.0f, 0.0f};
            normalize(a, &v[i], 0.8);

            glm_vec3_copy((vec3)WHITE, v[i].colour);
        }

        // for (size_t i = 0; i < 3; i++) {
        //     printf("%f %f %f\n", face[i].pos[0], face[i].pos[1],
        //            face[i].pos[2]);
        // }

        // memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, face,
        //    verticesPerFace * sizeof(*face));
        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, v,
               verticesPerFace * sizeof(*v));

        vulkan->shapes.verticesCount += verticesPerFace;

        // calculateIndicesForOctahedron(&vulkan->shapes);
        calculateIndicesForSphere(&vulkan->shapes, verticesPerFace);

        vulkan->shapes.indicesCount += verticesPerFace;
    }

    // for (size_t i = 0; i < vulkan->shapes.verticesCount; i++) {
    //     if (i % 3 == 0) {
    //         printf("\n");
    //     }
    //     printf("%f %f %f\n", vulkan->shapes.vertices[i].pos[0],
    //            vulkan->shapes.vertices[i].pos[1],
    //            vulkan->shapes.vertices[i].pos[2]);
    // }
}

void combineVerticesAndIndicesForOctahedron(Vulkan *vulkan,
                                            Octahedron octahedron,
                                            size_t count) {
    vulkan->shapes.vertices =
        malloc(count * 3 * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(count * 3 * sizeof(*vulkan->shapes.indices));

    for (uint32_t i = 0; i < count; i++) {
        Vertex *shape = octahedron[i];

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, shape,
               3 * sizeof(*shape));

        vulkan->shapes.verticesCount += 3;

        calculateIndicesForOctahedron(&vulkan->shapes);

        vulkan->shapes.indicesCount += 3;
    }
}

void combineVerticesAndIndicesForSquare(Vulkan *vulkan, Cube cube,
                                        size_t count) {
    vulkan->shapes.vertices =
        malloc(count * 4 * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(count * 6 * sizeof(*vulkan->shapes.indices));

    for (uint32_t i = 0; i < count; i++) {
        Vertex *shape = cube[i];

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, shape,
               4 * sizeof(*shape));

        vulkan->shapes.verticesCount += 4;

        calculateIndicesForSquare(&vulkan->shapes, shape, 4);

        vulkan->shapes.indicesCount += 6;
    }
}

void createDepthResources(Vulkan *vulkan) {
    VkFormat depthFormat = findDepthFormat(vulkan);

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
    // combineVerticesAndIndicesForSquare(vulkan, cube1, 6);
    // combineVerticesAndIndicesForOctahedron(vulkan, octahedron1, 8);
    combineVerticesAndIndicesForSphere(vulkan, octahedron1, 8, 5);

    VkDeviceSize bufferSize =
        sizeof(*vulkan->shapes.vertices) * vulkan->shapes.verticesCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device.device, stagingBufferMemory, 0, bufferSize, 0,
                &data);
    memcpy(data, vulkan->shapes.vertices, (size_t)bufferSize);
    vkUnmapMemory(vulkan->device.device, stagingBufferMemory);

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

    void *data;
    vkMapMemory(vulkan->device.device, stagingBufferMemory, 0, bufferSize, 0,
                &data);
    memcpy(data, vulkan->shapes.indices, (size_t)bufferSize);
    vkUnmapMemory(vulkan->device.device, stagingBufferMemory);

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
