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
    // poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = vulkan->swapchain.swapChainImagesCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = SIZEOF(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = vulkan->swapchain.swapChainImagesCount;

    if (vkCreateDescriptorPool(vulkan->device.device, &poolInfo, NULL,
                               &vulkan->descriptorSet.descriptorPool) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor pool!\n");
    }
}

void createDescriptorSets(Vulkan *vulkan) {
    VkDescriptorSetLayout *layouts =
        malloc(vulkan->swapchain.swapChainImagesCount * sizeof(*layouts));
    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        layouts[i] = vulkan->descriptorSet.descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkan->descriptorSet.descriptorPool;
    allocInfo.descriptorSetCount = vulkan->swapchain.swapChainImagesCount;
    allocInfo.pSetLayouts = layouts;

    vulkan->descriptorSet.descriptorSets =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->descriptorSet.descriptorSets));
    if (vkAllocateDescriptorSets(vulkan->device.device, &allocInfo,
                                 vulkan->descriptorSet.descriptorSets) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to allocate descriptor sets!\n");
    }

    for (size_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vulkan->descriptorSet.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrites = {};

        descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites.dstSet = vulkan->descriptorSet.descriptorSets[i];
        descriptorWrites.dstBinding = 0;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites.descriptorCount = 1;
        descriptorWrites.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vulkan->device.device, 1, &descriptorWrites, 0,
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
    // samplerLayoutBinding.descriptorType =
    // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {
        uboLayoutBinding,
        samplerLayoutBinding,
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = SIZEOF(bindings);
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(
            vulkan->device.device, &layoutInfo, NULL,
            &vulkan->descriptorSet.descriptorSetLayout) != VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor set layout!\n");
    }
}

void createUniformBuffers(Vulkan *vulkan) {
    vulkan->descriptorSet.uniformBuffers =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->descriptorSet.uniformBuffers));
    vulkan->descriptorSet.uniformBuffersMemory =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->descriptorSet.uniformBuffersMemory));

    for (size_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        createBuffer(sizeof(UniformBufferObject),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &vulkan->descriptorSet.uniformBuffers[i],
                     &vulkan->descriptorSet.uniformBuffersMemory[i]);
    }

    glm_mat4_identity(vulkan->ubo.model);
}

void updateUniformBuffer(Vulkan *vulkan, uint32_t currentImage,
                         float dt __unused) {

    glm_rotate(vulkan->ubo.model, dt * glm_rad(25.0f), (vec3)Z_AXIS);

    glm_lookat((vec3)VEC_3(2.0f), (vec3)CENTER, (vec3)Z_AXIS, vulkan->ubo.view);

    glm_perspective(glm_rad(45.0f),
                    vulkan->swapchain.swapChainExtent->width /
                        (float)vulkan->swapchain.swapChainExtent->height,
                    0.1f, 10.0f, vulkan->ubo.proj);

    vulkan->ubo.proj[1][1] *= -1;

    mapMemory(vulkan->device.device,
              vulkan->descriptorSet.uniformBuffersMemory[currentImage],
              sizeof(vulkan->ubo), &vulkan->ubo);
}
