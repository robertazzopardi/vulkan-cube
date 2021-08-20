#include "vulkan_handle/uniforms.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/util.h>
#include <string.h>
#include <vulkan/vulkan.h>

void createDescriptorPool(Vulkan *vulkan) {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = vulkan->swapchain.swapChainImagesCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = vulkan->swapchain.swapChainImagesCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = SIZEOF(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = vulkan->swapchain.swapChainImagesCount;

    if (vkCreateDescriptorPool(vulkan->device.device, &poolInfo, NULL,
                               &vulkan->ubo.descriptorPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor pool!\n");
    }
}

void createDescriptorSets(Vulkan *vulkan) {
    VkDescriptorSetLayout *layouts =
        malloc(vulkan->swapchain.swapChainImagesCount * sizeof(*layouts));
    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        layouts[i] = vulkan->ubo.descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkan->ubo.descriptorPool;
    allocInfo.descriptorSetCount = vulkan->swapchain.swapChainImagesCount;
    allocInfo.pSetLayouts = layouts;

    vulkan->ubo.descriptorSets = malloc(vulkan->swapchain.swapChainImagesCount *
                                        sizeof(*vulkan->ubo.descriptorSets));
    if (vkAllocateDescriptorSets(vulkan->device.device, &allocInfo,
                                 vulkan->ubo.descriptorSets) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate descriptor sets!\n");
    }

    for (size_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vulkan->ubo.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // VkDescriptorImageInfo imageInfo = {};
        // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // imageInfo.imageView = vulkan->texture.textureImageView;
        // imageInfo.sampler = vulkan->texture.textureSampler;

        // VkWriteDescriptorSet descriptorWrites[2];

        // descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptorWrites[0].dstSet = vulkan->ubo.descriptorSets[i];
        // descriptorWrites[0].dstBinding = 0;
        // descriptorWrites[0].dstArrayElement = 0;
        // descriptorWrites[0].descriptorType =
        // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // descriptorWrites[0].descriptorCount = 1;
        // descriptorWrites[0].pBufferInfo = &bufferInfo;
        // descriptorWrites[0].pNext = NULL;

        // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptorWrites[1].dstSet = vulkan->ubo.descriptorSets[i];
        // descriptorWrites[1].dstBinding = 1;
        // descriptorWrites[1].dstArrayElement = 0;
        // descriptorWrites[1].descriptorType =
        //     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // descriptorWrites[1].descriptorCount = 1;
        // descriptorWrites[1].pImageInfo = &imageInfo;
        // descriptorWrites[1].pNext = NULL;

        // vkUpdateDescriptorSets(vulkan->device.device,
        // SIZEOF(descriptorWrites),
        //                        descriptorWrites, 0, NULL);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = vulkan->ubo.descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vulkan->device.device, 1, &descriptorWrite, 0,
                               NULL);
    }

    freeMem(1, layouts);
}

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
                                    &vulkan->ubo.descriptorSetLayout) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor set layout!\n");
    }
}

void createUniformBuffers(Vulkan *vulkan) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    vulkan->ubo.uniformBuffers = malloc(vulkan->swapchain.swapChainImagesCount *
                                        sizeof(*vulkan->ubo.uniformBuffers));
    vulkan->ubo.uniformBuffersMemory =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->ubo.uniformBuffersMemory));

    for (size_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &vulkan->ubo.uniformBuffers[i],
                     &vulkan->ubo.uniformBuffersMemory[i]);
    }

    glm_mat4_identity(vulkan->ubo.model);
}

void updateUniformBuffer(Vulkan *vulkan, uint32_t currentImage, float dt) {
    glm_rotate(vulkan->ubo.model, dt * glm_rad(20.0f),
               (vec3){1.0f, 1.0f, -1.0f});

    glm_lookat((vec3){2.0f, 2.0f, 2.0f}, (vec3){0.0f, 0.0f, 0.0f},
               (vec3){0.0f, 0.0f, 1.0f}, vulkan->ubo.view);

    glm_perspective(glm_rad(45.0f),
                    vulkan->swapchain.swapChainExtent->width /
                        (float)vulkan->swapchain.swapChainExtent->height,
                    0.1f, 10.0f, vulkan->ubo.proj);

    vulkan->ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(vulkan->device.device,
                vulkan->ubo.uniformBuffersMemory[currentImage], 0,
                sizeof(vulkan->ubo), 0, &data);
    memcpy(data, &vulkan->ubo, sizeof(vulkan->ubo));
    vkUnmapMemory(vulkan->device.device,
                  vulkan->ubo.uniformBuffersMemory[currentImage]);
}
