#include "vulkan_handle/shape.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec3.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

// Vertex vertices[] = {
//     {{+0.5f, +0.5f, +0.5f}, {+0.f, +1.f, +0.f}}, // Top
//     {{-0.5f, +0.5f, +0.5f}, {+0.f, +1.f, +0.f}},
//     {{+0.5f, +0.5f, -0.5f}, {+0.f, +1.f, +0.f}},
//     {{-0.5f, +0.5f, -0.5f}, {+0.f, +1.f, +0.f}},
//     {{+0.5f, -0.5f, +0.5f}, {+0.f, -1.f, +0.f}}, // Bottom
//     {{-0.5f, -0.5f, +0.5f}, {+0.f, -1.f, +0.f}},
//     {{+0.5f, -0.5f, -0.5f}, {+0.f, -1.f, +0.f}},
//     {{-0.5f, -0.5f, -0.5f}, {+0.f, -1.f, +0.f}},
//     {{+0.5f, +0.5f, +0.5f}, {+1.f, +0.f, +0.f}}, // Right
//     {{+0.5f, +0.5f, -0.5f}, {+1.f, +0.f, +0.f}},
//     {{+0.5f, -0.5f, -0.5f}, {+1.f, +0.f, +0.f}},
//     {{+0.5f, -0.5f, +0.5f}, {+1.f, +0.f, +0.f}},
//     {{-0.5f, +0.5f, +0.5f}, {-1.f, +0.f, +0.f}}, // left
//     {{-0.5f, +0.5f, -0.5f}, {-1.f, +0.f, +0.f}},
//     {{-0.5f, -0.5f, -0.5f}, {-1.f, +0.f, +0.f}},
//     {{-0.5f, -0.5f, +0.5f}, {-1.f, +0.f, +0.f}},
//     {{-0.5f, +0.5f, +0.5f}, {+0.f, +0.f, +1.f}}, // front
//     {{+0.5f, +0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
//     {{+0.5f, -0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
//     {{-0.5f, -0.5f, +0.5f}, {+0.f, +0.f, +1.f}},
//     {{-0.5f, +0.5f, -0.5f}, {+0.f, +0.f, -1.f}}, // back
//     {{+0.5f, +0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
//     {{+0.5f, -0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
//     {{-0.5f, -0.5f, -0.5f}, {+0.f, +0.f, -1.f}},
// };
// uint32_t indices[] = {
//     0,  1,  2,  1,  2,  3,  /*top*/
//     4,  5,  6,  5,  6,  7,  /*bottom*/
//     8,  9,  10, 8,  10, 11, /*right*/
//     12, 13, 14, 12, 14, 15, /*left*/
//     16, 17, 18, 16, 18, 19, /*front*/
//     20, 21, 22, 20, 22, 23, /*back*/
// };

Vertex top[] = {
    {{-0.5f, -0.5f, 0.5f}, WHITE},
    {{0.5f, -0.5f, 0.5f}, GREEN},
    {{0.5f, 0.5f, 0.5f}, BLUE},
    {{-0.5f, 0.5f, 0.5f}, RED},
};
Vertex bottom[] = {
    {{-0.5f, 0.5f, -0.5f}, GREEN},
    {{0.5f, 0.5f, -0.5f}, WHITE},
    {{0.5f, -0.5f, -0.5f}, RED},
    {{-0.5f, -0.5f, -0.5f}, BLUE},
};
Vertex right[] = {
    {{0.5f, -0.5f, 0.5f}, GREEN}, // Right
    {{0.5f, -0.5f, -0.5f}, RED},
    {{0.5f, 0.5f, -0.5f}, WHITE},
    {{0.5f, 0.5f, 0.5f}, BLUE},
};
Vertex left[] = {
    {{-0.5f, 0.5f, 0.5f}, RED}, // left
    {{-0.5f, 0.5f, -0.5f}, GREEN},
    {{-0.5f, -0.5f, -0.5f}, BLUE},
    {{-0.5f, -0.5f, 0.5f}, WHITE},
};
Vertex front[] = {
    {{0.5f, 0.5f, 0.5f}, BLUE}, // front
    {{0.5f, 0.5f, -0.5f}, WHITE},
    {{-0.5f, 0.5f, -0.5f}, GREEN},
    {{-0.5f, 0.5f, 0.5f}, RED},
};
Vertex back[] = {
    {{0.5f, -0.5f, -0.5f}, RED}, // back
    {{0.5f, -0.5f, 0.5f}, GREEN},
    {{-0.5f, -0.5f, 0.5f}, WHITE},
    {{-0.5f, -0.5f, -0.5f}, BLUE},
};

// Vertex vertices[] = {
//     {{-0.5f, -0.5f, 0.5f}, RED}, // top
//     {{0.5f, -0.5f, 0.5f}, RED},     {{0.5f, 0.5f, 0.5f}, RED},
//     {{-0.5f, 0.5f, 0.5f}, RED},

//     {{-0.5f, 0.5f, -0.5f}, RED}, // bottom
//     {{0.5f, 0.5f, -0.5f}, GREEN},   {{0.5f, -0.5f, -0.5f}, BLUE},
//     {{-0.5f, -0.5f, -0.5f}, WHITE},

//     {{0.5f, -0.5f, 0.5f}, GREEN}, // right
//     {{0.5f, -0.5f, -0.5f}, WHITE},  {{0.5f, 0.5f, -0.5f}, RED},
//     {{0.5f, 0.5f, 0.5f}, BLUE},

//     {{-0.5f, 0.5f, 0.5f}, RED}, // left
//     {{-0.5f, 0.5f, -0.5f}, GREEN},  {{-0.5f, -0.5f, -0.5f}, BLUE},
//     {{-0.5f, -0.5f, 0.5f}, WHITE},

//     {{0.5f, 0.5f, 0.5f}, BLUE}, // front
//     {{0.5f, 0.5f, -0.5f}, RED},     {{-0.5f, 0.5f, -0.5f}, GREEN},
//     {{-0.5f, 0.5f, 0.5f}, WHITE},

//     {{0.5f, -0.5f, -0.5f}, RED}, // back
//     {{0.5f, -0.5f, 0.5f}, GREEN},   {{-0.5f, -0.5f, 0.5f}, BLUE},
//     {{-0.5f, -0.5f, -0.5f}, WHITE},
// };

// uint16_t indices[] = {
//     0,  1,  2,  1,  2,  3,  /*top*/
//     4,  5,  6,  5,  6,  7,  /*bottom*/
//     8,  9,  10, 8,  10, 11, /*right*/
//     12, 13, 14, 12, 14, 15, /*left*/
//     16, 17, 18, 16, 18, 19, /*front*/
//     20, 21, 22, 20, 22, 23, /*back*/

//     // 0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
//     // 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23,
//     20,
// };

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

void calculateIndices(Shape *vulkanShape, Shape shape) {
    Vertex arr[shape.verticesCount];
    uint32_t vertIndex = 0;

    uint16_t *indis = malloc((shape.verticesCount + 2) * sizeof(*indis));
    uint32_t indisIndex = 0;

    for (uint32_t i = 0; i < shape.verticesCount; i++) {
        Vertex vert = shape.vertices[i];

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

    // for (uint32_t i = 0; i < indisIndex; i++) {
    //     printf("%u ", indis[i]);
    // }
    // printf("\n");

    freeMem(1, indis);
}

void combineVerticesAndIndices(Vulkan *vulkan, uint32_t count, ...) {
    va_list valist;

    va_start(valist, count);

    vulkan->shapes.vertices =
        malloc(count * 4 * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(count * 6 * sizeof(*vulkan->shapes.indices));

    // printf("%u %u\n", count * 4, count * 6);

    for (uint32_t i = 0; i < count; i++) {
        Shape shape = va_arg(valist, Shape);

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount,
               shape.vertices, shape.verticesCount * sizeof(*shape.vertices));

        vulkan->shapes.verticesCount += shape.verticesCount;

        calculateIndices(&vulkan->shapes, shape);

        vulkan->shapes.indicesCount += shape.indicesCount;
    }

    va_end(valist);
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
    combineVerticesAndIndices(
        vulkan, 6, (Shape){top, 4, NULL, 6, 0}, (Shape){bottom, 4, NULL, 6, 0},
        (Shape){right, 4, NULL, 6, 0}, (Shape){left, 4, NULL, 6, 0},
        (Shape){front, 4, NULL, 6, 0}, (Shape){back, 4, NULL, 6, 0});

    // vulkan->shapes.verticesCount = 24;
    // vulkan->shapes.indicesCount = 36;

    // vulkan->shapes.vertices = malloc(24 * sizeof(*vulkan->shapes.vertices));
    // vulkan->shapes.indices = malloc(36 * sizeof(*vulkan->shapes.indices));

    // vulkan->shapes.vertices = vertices;
    // vulkan->shapes.indices = indices;

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
