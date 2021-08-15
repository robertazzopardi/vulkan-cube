#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include "vulkan_handle/vulkan_handle.h"
#include "error_handle.h"
#include "memory.h"
#include "swapchain.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/validation.h"
#include "window/window.h"
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/io.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

// const Vertex vertices[] = {
//     // 1
//     {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

//     // 2
//     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
// };

Vertex shape1[] = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};
// uint16_t indices1[] = {
//     0, 1, 2, 2, 3, 0,
// };

Vertex shape2[] = {
    {{-0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};
// uint16_t indices2[] = {
//     4, 5, 6, 6, 7, 4,
// };

// const uint16_t indices[] = {
//     // 1
//     0, 1, 2, 2, 3, 0,
//     // 2
//     4, 5, 6, 6, 7, 4};

#pragma region VERTEX / INDEX / UNIFORM BUFFER

void createDescriptorSetLayout(Vulkan *vulkan) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding,
                                               samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = SIZEOF(bindings);
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vulkan->device.device, &layoutInfo, NULL,
                                    &vulkan->descriptorSetLayout) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor set layout!\n");
    }
}

uint32_t findMemoryType(Vulkan *vulkan, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkan->device.physicalDevice,
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    THROW_ERROR("failed to find suitable memory type!\n");
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, Vulkan *vulkan,
                  VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
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

    VkMemoryAllocateInfo allocInfo = {};
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

VkCommandBuffer beginSingleTimeCommands(Vulkan *vulkan) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkan->device.device, &allocInfo,
                             &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(Vulkan *vulkan, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->graphicsQueue);

    vkFreeCommandBuffers(vulkan->device.device, vulkan->commandPool, 1,
                         &commandBuffer);
}

void copyBuffer(Vulkan *vulkan, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createVertexBuffer(Vulkan *vulkan) {
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
                 &vulkan->vertexBuffer, &vulkan->vertexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->vertexBuffer, bufferSize);

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
                 &vulkan->indexBuffer, &vulkan->indexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->indexBuffer, bufferSize);

    vkDestroyBuffer(vulkan->device.device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device.device, stagingBufferMemory, NULL);
}

void createUniformBuffers(Vulkan *vulkan) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    vulkan->uniformBuffers =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->uniformBuffers));
    vulkan->uniformBuffersMemory = malloc(
        vulkan->swapChainImagesCount * sizeof(*vulkan->uniformBuffersMemory));

    for (size_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &vulkan->uniformBuffers[i],
                     &vulkan->uniformBuffersMemory[i]);
    }

    glm_mat4_identity(vulkan->ubo.model);
}

void createDescriptorSets(Vulkan *vulkan) {
    VkDescriptorSetLayout *layouts =
        malloc(vulkan->swapChainImagesCount * sizeof(*layouts));
    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        layouts[i] = vulkan->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkan->descriptorPool;
    allocInfo.descriptorSetCount = vulkan->swapChainImagesCount;
    allocInfo.pSetLayouts = layouts;

    vulkan->descriptorSets =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->descriptorSets));
    if (vkAllocateDescriptorSets(vulkan->device.device, &allocInfo,
                                 vulkan->descriptorSets) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate descriptor sets!\n");
    }

    for (size_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vulkan->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkan->textureImageView;
        imageInfo.sampler = vulkan->textureSampler;

        VkWriteDescriptorSet descriptorWrites[2];

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vulkan->descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pNext = NULL;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vulkan->descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pNext = NULL;

        vkUpdateDescriptorSets(vulkan->device.device, SIZEOF(descriptorWrites),
                               descriptorWrites, 0, NULL);
    }

    freeMem(1, layouts);
}

void createDescriptorPool(Vulkan *vulkan) {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = vulkan->swapChainImagesCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = vulkan->swapChainImagesCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = SIZEOF(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = vulkan->swapChainImagesCount;

    if (vkCreateDescriptorPool(vulkan->device.device, &poolInfo, NULL,
                               &vulkan->descriptorPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor pool!\n");
    }
}

#pragma endregion

#pragma region VERTEX DATA

#pragma region

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

        bool inArray = isInVertexArray(vert, arr, vertIndex);

        if (!inArray) {
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

#pragma endregion

void combineVerticesAndIndices(Vulkan *vulkan, uint32_t count, ...) {
    va_list valist;

    va_start(valist, count);

    vulkan->shapes.vertices =
        malloc(count * 4 * sizeof(*vulkan->shapes.vertices));
    vulkan->shapes.indices =
        malloc(count * 6 * sizeof(*vulkan->shapes.indices));

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

static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

static VkVertexInputAttributeDescription *getAttributeDescriptions() {
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
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

#pragma endregion

#pragma region CREATING INSTANCE

#pragma endregion

#pragma region DEBUG MESSENGER

#pragma endregion

#pragma region CREATE SURFACE

#pragma endregion

#pragma region PHYSICAL DEVICE

VkSampleCountFlagBits getMaxUsableSampleCount(Vulkan *);

#pragma endregion

#pragma region LOGICAL DEVICE

void createLogicalDevice(Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, vulkan->surface);

    uint32_t uniqueQueueFamilies[] = {queueFamilyIndices.graphicsFamily,
                                      queueFamilyIndices.presentFamily};
    VkDeviceQueueCreateInfo queueCreateInfos[SIZEOF(uniqueQueueFamilies)];

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < SIZEOF(uniqueQueueFamilies); i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    // enable sample shading feature for the device
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = SIZEOF(queueCreateInfos);
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.enabledExtensionCount = SIZEOF(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vulkan->device.physicalDevice, &createInfo, NULL,
                       &vulkan->device.device) != VK_SUCCESS) {
        THROW_ERROR("failed to create logical device!\n");
    }

    vkGetDeviceQueue(vulkan->device.device, queueFamilyIndices.graphicsFamily,
                     0, &vulkan->graphicsQueue);
    vkGetDeviceQueue(vulkan->device.device, queueFamilyIndices.presentFamily,
                     0, &vulkan->presentQueue);
}

#pragma endregion

#pragma region SWAP CHAIN

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats,
                                           uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

// void clamp(uint32_t *val, uint32_t min, uint32_t max) {
//     if (*val < min) {
//         *val = min;
//     }
//     if (*val > max) {
//         *val = max;
//     }
// }

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,
                                       uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void createSwapChain(SDL_Window *window, Vulkan *vulkan) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(vulkan->device.physicalDevice, vulkan->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    vulkan->swapChainImagesCount =
        swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        vulkan->swapChainImagesCount >
            swapChainSupport.capabilities.maxImageCount) {
        vulkan->swapChainImagesCount =
            swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkan->surface;
    createInfo.minImageCount = vulkan->swapChainImagesCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, vulkan->surface);

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndicesArr[] = {queueFamilyIndices.graphicsFamily,
                                            queueFamilyIndices.presentFamily};
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArr;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vulkan->device.device, &createInfo, NULL,
                             &vulkan->swapChain) != VK_SUCCESS) {
        THROW_ERROR("failed to create swap chain!\n");
    }

    vkGetSwapchainImagesKHR(vulkan->device.device, vulkan->swapChain,
                            &vulkan->swapChainImagesCount, NULL);

    vulkan->swapChainImages =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->swapChainImages));

    if (vulkan->swapChainImages == NULL) {
        THROW_ERROR("Could not init swap chain images\n");
    }
    vkGetSwapchainImagesKHR(vulkan->device.device, vulkan->swapChain,
                            &vulkan->swapChainImagesCount,
                            vulkan->swapChainImages);

    vulkan->swapChainImageFormat = surfaceFormat.format;
    vulkan->swapChainExtent = extent;
}

#pragma endregion

#pragma region RENDER PASS

VkFormat findDepthFormat(Vulkan *);

void createRenderPass(Vulkan *vulkan) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = vulkan->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.samples = vulkan->msaaSamples;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat(vulkan);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.samples = vulkan->msaaSamples;

    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = vulkan->swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment,
                                             colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = SIZEOF(attachments);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vulkan->device.device, &renderPassInfo, NULL,
                           &vulkan->renderPass) != VK_SUCCESS) {
        THROW_ERROR("failed to create render pass!\n");
    }
}

#pragma endregion

#pragma region GRAPHICS PIPELINE

void createShaderModule(char *code, uint32_t length, VkDevice device,
                        VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = length;
    createInfo.pCode = (uint32_t *)code;

    if (vkCreateShaderModule(device, &createInfo, NULL, shaderModule) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create shader module!\n");
    }
}

typedef struct {
    char *buff;
    uint32_t len;
} FileData;

FileData readFile(const char *path) {
    FileData data;

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("File not found: %s\n", path);
        exit(EXIT_FAILURE);
    }

    // get filesize
    fseek(file, 0, SEEK_END);
    data.len = ftell(file);

    fseek(file, 0, SEEK_SET);
    // allocate buffer **note** that if you like
    // to use the buffer as a c-string then you must also
    // allocate space for the terminating null character
    data.buff = malloc(data.len);
    // read the file into buffer
    fread(data.buff, data.len, 1, file);
    // close the file
    fclose(file);

    return data;
}

void createGraphicsPipeline(Vulkan *vulkan) {
    FileData vertShaderCode = readFile("shaders/vert.spv");
    FileData fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule;
    createShaderModule(vertShaderCode.buff, vertShaderCode.len,
                       vulkan->device.device, &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(fragShaderCode.buff, fragShaderCode.len,
                       vulkan->device.device, &fragShaderModule);

    freeMem(2, vertShaderCode.buff, fragShaderCode.buff);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription =
        getBindingDescription();
    VkVertexInputAttributeDescription *attributeDescriptions =
        getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vulkan->swapChainExtent.width;
    viewport.height = (float)vulkan->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = vulkan->swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vulkan->msaaSamples;
    // multisampling.sampleShadingEnable =
    //     VK_TRUE; // enable sample shading in the pipeline
    // multisampling.minSampleShading =
    //     .2f; // min fraction for sample shading; closer to one is smoother

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vulkan->descriptorSetLayout;

    if (vkCreatePipelineLayout(vulkan->device.device, &pipelineLayoutInfo,
                               NULL, &vulkan->pipelineLayout) != VK_SUCCESS) {
        THROW_ERROR("failed to create pipeline layout!\n");
    }

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pNext = NULL;
    dynamicStateCreateInfo.flags = 0;
    dynamicStateCreateInfo.dynamicStateCount = 2;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = vulkan->pipelineLayout;
    pipelineInfo.renderPass = vulkan->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(vulkan->device.device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, NULL,
                                  &vulkan->graphicsPipeline) != VK_SUCCESS) {
        THROW_ERROR("failed to create graphics pipeline!\n");
    }

    vkDestroyShaderModule(vulkan->device.device, fragShaderModule, NULL);
    vkDestroyShaderModule(vulkan->device.device, vertShaderModule, NULL);

    freeMem(1, attributeDescriptions);
}

#pragma endregion

#pragma region FRAME BUFFERS

void createFramebuffers(Vulkan *vulkan) {
    vulkan->swapChainFramebuffers = malloc(
        vulkan->swapChainImagesCount * sizeof(*vulkan->swapChainFramebuffers));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkImageView attachments[] = {vulkan->colorImageView,
                                     vulkan->depthImageView,
                                     vulkan->swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan->renderPass;
        framebufferInfo.attachmentCount = SIZEOF(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkan->swapChainExtent.width;
        framebufferInfo.height = vulkan->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkan->device.device, &framebufferInfo, NULL,
                                &vulkan->swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            THROW_ERROR("failed to create framebuffer!\n");
        }
    }
}

#pragma endregion

#pragma region COMMAND POOL

void createCommandPool(Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, vulkan->surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(vulkan->device.device, &poolInfo, NULL,
                            &vulkan->commandPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create command pool!\n");
    }
}

#pragma endregion

#pragma region COMMAND BUFFER

#pragma endregion

#pragma region SYNC OBJECTS

void createSyncObjects(Vulkan *vulkan) {
    VkSemaphore *imageAvailableSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*imageAvailableSemaphoresTemp));
    VkSemaphore *renderFinishedSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*renderFinishedSemaphoresTemp));
    VkFence *inFlightFencesTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*inFlightFencesTemp));
    vulkan->imagesInFlight =
        calloc(vulkan->swapChainImagesCount, sizeof(*vulkan->imagesInFlight));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vulkan->device.device, &semaphoreInfo, NULL,
                              &imageAvailableSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan->device.device, &semaphoreInfo, NULL,
                              &renderFinishedSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan->device.device, &fenceInfo, NULL,
                          &inFlightFencesTemp[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
        }
    }

    vulkan->imageAvailableSemaphores = imageAvailableSemaphoresTemp;
    vulkan->renderFinishedSemaphores = renderFinishedSemaphoresTemp;
    vulkan->inFlightFences = inFlightFencesTemp;
}

#pragma endregion

void createImageViews(Vulkan *);

void createDepthResources(Vulkan *);

#pragma endregion

#pragma region DRAW FRAME

void updateUniformBuffer(Vulkan *vulkan, uint32_t currentImage, float dt) {

    glm_rotate(vulkan->ubo.model, dt * glm_rad(45.0f),
               (vec3){0.0f, 0.0f, 1.0f});

    glm_lookat((vec3){2.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f},
               (vec3){0.0f, 0.0f, 1.0f}, vulkan->ubo.view);

    glm_perspective(glm_rad(45.0f),
                    vulkan->swapChainExtent.width /
                        vulkan->swapChainExtent.height,
                    0.1f, 20.0f, vulkan->ubo.proj);

    vulkan->ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(vulkan->device.device,
                vulkan->uniformBuffersMemory[currentImage], 0,
                sizeof(vulkan->ubo), 0, &data);
    memcpy(data, &vulkan->ubo, sizeof(vulkan->ubo));
    vkUnmapMemory(vulkan->device.device,
                  vulkan->uniformBuffersMemory[currentImage]);
}

#pragma endregion

#pragma region TEXTURE MAPPING

void createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkSampleCountFlagBits numSamples, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, Vulkan *vulkan,
                 VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vulkan->device.device, &imageInfo, NULL, image) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create image!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkan->device.device, *image,
                                 &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkan, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkan->device.device, &allocInfo, NULL,
                         imageMemory) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate image memory!\n");
    }

    vkBindImageMemory(vulkan->device.device, *image, *imageMemory, 0);
}

void transitionImageLayout(Vulkan *vulkan, VkImage image,
                           VkFormat format __unused, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        THROW_ERROR("unsupported layout transition!\n");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         NULL, 0, NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void copyBufferToImage(Vulkan *vulkan, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(vulkan, commandBuffer);
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags,
                            uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    // viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        THROW_ERROR("failed to create texture image view!\n");
    }

    return imageView;
}

void createTextureImageView(Vulkan *vulkan) {
    vulkan->textureImageView = createImageView(
        vulkan->device.device, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT, vulkan->mipLevels);
}

void createImageViews(Vulkan *vulkan) {
    vulkan->swapChainImageViews = malloc(vulkan->swapChainImagesCount *
                                         sizeof(*vulkan->swapChainImageViews));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vulkan->swapChainImageViews[i] = createImageView(
            vulkan->device.device, vulkan->swapChainImages[i],
            vulkan->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void createTextureSampler(Vulkan *vulkan) {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(vulkan->device.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = (float)vulkan->mipLevels;
    samplerInfo.mipLodBias = 0.0f; // Optional

    if (vkCreateSampler(vulkan->device.device, &samplerInfo, NULL,
                        &vulkan->textureSampler) != VK_SUCCESS) {
        THROW_ERROR("failed to create texture sampler!\n");
    }
}

#pragma endregion

#pragma region DEPTH_BUFFERING

VkFormat findSupportedFormat(const VkFormat *candidates, size_t length,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features,
                             VkPhysicalDevice physicalDevice) {
    // for (VkFormat format : candidates) {
    for (size_t i = 0; i < length; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i],
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    THROW_ERROR("Failed to find supported format!\n");
}

VkFormat findDepthFormat(Vulkan *vulkan) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(candidates, SIZEOF(candidates),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               vulkan->device.physicalDevice);
}

void createDepthResources(Vulkan *vulkan) {
    VkFormat depthFormat = findDepthFormat(vulkan);

    createImage(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height,
                1, vulkan->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->depthImage, &vulkan->depthImageMemory);

    vulkan->depthImageView =
        createImageView(vulkan->device.device, vulkan->depthImage, depthFormat,
                        VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

#pragma endregion

#pragma region MIPMAPS

#pragma endregion

#pragma region MULTI_SAMPLING

void createColorResources(Vulkan *vulkan) {
    VkFormat colorFormat = vulkan->swapChainImageFormat;

    createImage(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height,
                1, vulkan->msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->colorImage, &vulkan->colorImageMemory);

    vulkan->colorImageView =
        createImageView(vulkan->device.device, vulkan->colorImage, colorFormat,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

#pragma endregion

#pragma region RECREATE SWAP CHAIN

void cleanupSwapChain(Vulkan *vulkan) {
    vkDestroyImageView(vulkan->device.device, vulkan->colorImageView, NULL);
    vkDestroyImage(vulkan->device.device, vulkan->colorImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->colorImageMemory, NULL);

    vkDestroyImageView(vulkan->device.device, vulkan->depthImageView, NULL);
    vkDestroyImage(vulkan->device.device, vulkan->depthImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->depthImageMemory, NULL);

    vkFreeCommandBuffers(vulkan->device.device, vulkan->commandPool,
                         vulkan->swapChainImagesCount, vulkan->commandBuffers);

    vkDestroyPipeline(vulkan->device.device, vulkan->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vulkan->device.device, vulkan->pipelineLayout,
                            NULL);
    vkDestroyRenderPass(vulkan->device.device, vulkan->renderPass, NULL);

    vkDestroySwapchainKHR(vulkan->device.device, vulkan->swapChain, NULL);

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyFramebuffer(vulkan->device.device,
                             vulkan->swapChainFramebuffers[i], NULL);

        vkDestroyImageView(vulkan->device.device,
                           vulkan->swapChainImageViews[i], NULL);

        vkDestroyBuffer(vulkan->device.device, vulkan->uniformBuffers[i],
                        NULL);

        vkFreeMemory(vulkan->device.device, vulkan->uniformBuffersMemory[i],
                     NULL);
    }

    freeMem(4, vulkan->swapChainFramebuffers, vulkan->swapChainImageViews,
            vulkan->uniformBuffers, vulkan->uniformBuffersMemory);

    vkDestroyDescriptorPool(vulkan->device.device, vulkan->descriptorPool,
                            NULL);
}

void generateMipmaps(Vulkan *vulkan, VkImage image, VkFormat imageFormat,
                     int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(vulkan->device.physicalDevice,
                                        imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        THROW_ERROR("Texture image format does not support linear blitting!\n");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0,
                             NULL, 1, &barrier);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        blit.dstOffsets[1] = (VkOffset3D){mipWidth > 1 ? mipWidth / 2 : 1,
                                          mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL,
                             0, NULL, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
                         NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void initVulkan(Window *window, Vulkan *vulkan) {
    createInstance(window->win, vulkan);

    setupDebugMessenger(vulkan);

    createSurface(window->win, vulkan);

    pickPhysicalDevice(vulkan);

    createLogicalDevice(vulkan);

    createSwapChain(window->win, vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createColorResources(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(vulkan);

    createTextureImage(vulkan);

    createTextureImageView(vulkan);

    createTextureSampler(vulkan);

    combineVerticesAndIndices(vulkan, 2, (Shape){shape1, 4, NULL, 6, 0},
                              (Shape){shape2, 4, NULL, 6, 0});
    createVertexBuffer(vulkan);
    createIndexBuffer(vulkan);

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan, window);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Vulkan *vulkan) {
    cleanupSwapChain(vulkan);

    vkDestroySampler(vulkan->device.device, vulkan->textureSampler, NULL);
    vkDestroyImageView(vulkan->device.device, vulkan->textureImageView, NULL);

    vkDestroyImage(vulkan->device.device, vulkan->textureImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->textureImageMemory, NULL);

    vkDestroyDescriptorSetLayout(vulkan->device.device,
                                 vulkan->descriptorSetLayout, NULL);

    if (vulkan->graphicsPipeline == VK_NULL_HANDLE) {
        vkDestroyPipeline(vulkan->device.device, vulkan->graphicsPipeline,
                          NULL);
    }
    if (vulkan->pipelineLayout == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vulkan->device.device, vulkan->pipelineLayout,
                                NULL);
    }

    vkDestroyBuffer(vulkan->device.device, vulkan->indexBuffer, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->indexBufferMemory, NULL);

    vkDestroyBuffer(vulkan->device.device, vulkan->vertexBuffer, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->vertexBufferMemory, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(vulkan->device.device, vulkan->inFlightFences[i], NULL);
    }

    freeMem(8, vulkan->descriptorSets, vulkan->shapes.vertices,
            vulkan->shapes.indices, vulkan->renderFinishedSemaphores,
            vulkan->imageAvailableSemaphores, vulkan->inFlightFences,
            vulkan->imagesInFlight, vulkan->commandBuffers);

    vkDestroyCommandPool(vulkan->device.device, vulkan->commandPool, NULL);

    vkDestroyDevice(vulkan->device.device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan->instance.instance,
                                      vulkan->validation->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(vulkan->instance.instance, vulkan->surface, NULL);
    vkDestroyInstance(vulkan->instance.instance, NULL);
}
