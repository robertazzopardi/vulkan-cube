#include "vulkan_handle/uniforms.h"
#include "error_handle.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/util.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <window/window.h>

static inline VkDescriptorPoolSize createDescriptorPoolSize(uint32_t count) {
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = count;
    return poolSize;
}

void createDescriptorPool(Vulkan *vulkan) {
    VkDescriptorPoolSize poolSizes[] = {
        createDescriptorPoolSize(vulkan->swapchain.swapChainImagesCount),
        createDescriptorPoolSize(vulkan->swapchain.swapChainImagesCount),
    };

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

static inline VkWriteDescriptorSet
createWriteDescriptorSet(VkDescriptorBufferInfo *bufferInfo,
                         VkDescriptorSet descriptorSet, uint32_t binding) {
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = bufferInfo;
    return descriptorWrite;
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

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkDescriptorBufferInfo mvpBufferInfo = {};
        mvpBufferInfo.buffer = vulkan->descriptorSet.uniformBuffers[i];
        mvpBufferInfo.offset = 0;
        mvpBufferInfo.range = sizeof(UniformMVP);

        VkDescriptorBufferInfo lightBufferInfo = {};
        lightBufferInfo.buffer = vulkan->descriptorSet.uniformBuffers[i];
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(UniformLight);

        VkWriteDescriptorSet descriptorWrites[] = {
            createWriteDescriptorSet(
                &mvpBufferInfo, vulkan->descriptorSet.descriptorSets[i], 0),
            createWriteDescriptorSet(
                &lightBufferInfo, vulkan->descriptorSet.descriptorSets[i], 1),
        };

        vkUpdateDescriptorSets(vulkan->device.device, SIZEOF(descriptorWrites),
                               descriptorWrites, 0, NULL);
    }

    freeMem(1, layouts);
}

static inline VkDescriptorSetLayoutBinding
createDescriptorSetLayoutBinding(VkDescriptorType dType,
                                 VkShaderStageFlags sFlags, uint32_t binding) {
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorCount = 1;
    layoutBinding.descriptorType = dType;
    layoutBinding.pImmutableSamplers = NULL; // Optional
    layoutBinding.stageFlags = sFlags;
    return layoutBinding;
}

void createDescriptorSetLayout(Vulkan *vulkan) {
    VkDescriptorSetLayoutBinding bindings[] = {
        createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT, 0),
        createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_FRAGMENT_BIT, 1),
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

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        createBuffer(sizeof(UniformBufferObject),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &vulkan->descriptorSet.uniformBuffers[i],
                     &vulkan->descriptorSet.uniformBuffersMemory[i]);
    }

    glm_mat4_identity(vulkan->uniforms.mvp.model);

    glm_vec3_copy((vec3)WHITE, vulkan->uniforms.light.colour);
    glm_vec3_copy((vec3){1.0f, -5.0f, 1.0f}, vulkan->uniforms.light.pos);
}

void updateUniformBuffer(Vulkan *vulkan, Window *window,
                         uint32_t currentImage) {

    // glm_vec3_rotate(vulkan->uniforms.light.pos, window->dt * glm_rad(25.0f),
    //                 (vec3)Z_AXIS);

    glm_rotate(vulkan->uniforms.mvp.model, window->dt * glm_rad(25.0f),
               (vec3)Z_AXIS);

    glm_rotate(vulkan->uniforms.mvp.model, window->mX * 0.00005, (vec3)Y_AXIS);
    glm_rotate(vulkan->uniforms.mvp.model, window->mY * 0.00005, (vec3)X_AXIS);

    glm_lookat((vec3)VEC_3(2.0f), (vec3)CENTER, (vec3)Z_AXIS,
               vulkan->uniforms.mvp.view);

    float aspectRatio = vulkan->swapchain.swapChainExtent->width /
                        (float)vulkan->swapchain.swapChainExtent->height;
    glm_perspective(glm_rad(45.0f), aspectRatio, 0.1f, 10.0f,
                    vulkan->uniforms.mvp.proj);

    vulkan->uniforms.mvp.proj[1][1] *= -1;

    mapMemory(vulkan->device.device,
              vulkan->descriptorSet.uniformBuffersMemory[currentImage],
              sizeof(vulkan->uniforms), &vulkan->uniforms);
}
