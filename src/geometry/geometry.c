#include "geometry/geometry.h"
#include "error_handle.h"
#include "geometry/circle/circle.h"
#include "geometry/cube/cube.h"
#include "geometry/ring/ring.h"
#include "geometry/shpere/sphere.h"
#include "geometry/shpere/trisphere.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

static const struct Shape EmptyStruct;

inline void getMiddlePoint(vec3 point1, vec3 point2, vec3 res) {
    res[0] = (point1[0] + point2[0]) / 2.0f;
    res[1] = (point1[1] + point2[1]) / 2.0f;
    res[2] = (point1[2] + point2[2]) / 2.0f;
}

inline VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

inline void normalize(vec3 a, Vertex *b, float length) {
    // get the distance between a and b along the x and y axes
    vec3 d;
    glm_vec3_sub((*b).pos, a, d);

    // right now, sqrt(dx^2 + dy^2) = distance(a,b).
    // we want to modify them so that sqrt(dx^2 + dy^2) = the given length.
    float norm = length / glm_vec3_distance(a, (*b).pos);

    glm_vec3_scale(d, norm, d);

    glm_vec3_add(a, d, (*b).pos);
}

inline VkVertexInputAttributeDescription *getAttributeDescriptions() {

    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(4 * sizeof(*attributeDescriptions));

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

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

static inline void getNormal(vec3 p1, vec3 p2, vec3 p3, vec3 normal) {
    vec3 a, b;
    glm_vec3_sub(p3, p2, a);
    glm_vec3_sub(p1, p2, b);
    glm_vec3_cross(a, b, normal);
}

inline void calculateNormals(Vertex *shape, uint32_t vertsToUpdate) {
    vec3 normal;
    getNormal(shape[0].pos, shape[1].pos, shape[2].pos, normal);

    for (size_t i = 0; i < vertsToUpdate; i++) {
        glm_vec3_copy(normal, shape[i].normal);
    }
}

inline void createDepthResources(Vulkan *vulkan) {
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
        VkImageView attachments[] = {vulkan->colorImageView,
                                     vulkan->depth.depthImageView,
                                     vulkan->swapchain.swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan->renderPass;
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

// for (size_t i = 0; i < SIZEOF(cube1); i++) {
//     vec3 normal;
//     getNormal(octahedron1[i][0].pos, octahedron1[i][1].pos,
//     octahedron1[i][2].pos, normal);
//     for (size_t j = 0; j < 4; j++) {
//         glm_vec3_copy(normal, octahedron1[i][j].normal);
//     }
// }

void copyBuffer(Vulkan *vulkan, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createVertexIndexBuffer(Vulkan *vulkan, void *data, uint64_t bufferSize,
                             VkBuffer *buffer, VkDeviceMemory *bufferMemory,
                             VkBufferUsageFlags usageFlags) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    mapMemory(vulkan->device.device, stagingBufferMemory, bufferSize, data);

    createBuffer(bufferSize, usageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 vulkan, buffer, bufferMemory);

    copyBuffer(vulkan, stagingBuffer, *buffer, bufferSize);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);
}

inline void bindVertexAndIndexBuffers(Vulkan *vulkan) {
    vulkan->shapeBuffers.vertexBuffer =
        malloc(vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.vertexBuffer));
    vulkan->shapeBuffers.vertexBufferMemory = malloc(
        vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.vertexBufferMemory));
    vulkan->shapeBuffers.indexBuffer =
        malloc(vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.indexBuffer));
    vulkan->shapeBuffers.indexBufferMemory = malloc(
        vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.indexBufferMemory));

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        createVertexIndexBuffer(vulkan, vulkan->shapes[i].vertices,
                                sizeof(*vulkan->shapes[i].vertices) *
                                    vulkan->shapes[i].verticesCount,
                                &vulkan->shapeBuffers.vertexBuffer[i],
                                &vulkan->shapeBuffers.vertexBufferMemory[i],
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        createVertexIndexBuffer(vulkan, vulkan->shapes[i].indices,
                                sizeof(*vulkan->shapes[i].indices) *
                                    vulkan->shapes[i].indicesCount,
                                &vulkan->shapeBuffers.indexBuffer[i],
                                &vulkan->shapeBuffers.indexBufferMemory[i],
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }
}

inline void generateShape(Vulkan *vulkan, ShapeType shapeType,
                          const char *textureFileName) {

    vulkan->shapes = realloc(vulkan->shapes, (vulkan->shapeCount + 1) *
                                                 sizeof(*vulkan->shapes));
    vulkan->shapes[vulkan->shapeCount] = EmptyStruct;

    Shape *shape = &vulkan->shapes[vulkan->shapeCount];

    vulkan->shapes[vulkan->shapeCount].graphicsPipeline.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vulkan->shapes[vulkan->shapeCount].graphicsPipeline.cullMode =
        VK_CULL_MODE_BACK_BIT;
    vulkan->shapes[vulkan->shapeCount].indexed = true;

    switch (shapeType) {
    case CUBE:
        makeCube(vulkan);
        break;
    case SPHERE:
        makeSphere(shape, 60, 60, 0.4);
        break;
    case ICOSPHERE:
    case OCTASPHERE:
        makeTriSphere(vulkan, shapeType, 3);
        break;
    case CIRCLE:
        makeCircle(shape, 40, 2);
        break;
    case PLAIN:
        break;
    case RING:
        makeRing(shape, 60, 2);
        vulkan->shapes[vulkan->shapeCount].graphicsPipeline.topology =
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        vulkan->shapes[vulkan->shapeCount].graphicsPipeline.cullMode =
            VK_CULL_MODE_NONE;
        vulkan->shapes[vulkan->shapeCount].indexed = false;
    default:
        break;
    }

    createTextureImage(vulkan, textureFileName);
    createTextureImageView(vulkan);
    createTextureSampler(vulkan);

    vulkan->shapeCount++;
}
