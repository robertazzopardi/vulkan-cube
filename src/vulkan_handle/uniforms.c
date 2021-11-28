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

static inline VkDescriptorPoolSize
createDescriptorPoolSize(VkDescriptorType type, uint32_t count) {
    VkDescriptorPoolSize poolSize;
    poolSize.type = type;
    poolSize.descriptorCount = count;
    return poolSize;
}

inline void createDescriptorPool(Vulkan *vulkan,
                                 VkDescriptorPool *descriptorPool) {
    VkDescriptorPoolSize poolSizes[] = {
        createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                 vulkan->swapchain.swapChainImagesCount),
        createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 vulkan->swapchain.swapChainImagesCount),
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = SIZEOF(poolSizes),
        .pPoolSizes = poolSizes,
        .maxSets = vulkan->swapchain.swapChainImagesCount,
    };

    if (vkCreateDescriptorPool(vulkan->device.device, &poolInfo, NULL,
                               descriptorPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor pool!\n");
    }
}

static inline VkWriteDescriptorSet createWriteDescriptorInfo(
    VkDescriptorBufferInfo *bufferInfo, VkDescriptorImageInfo *imageInfo,
    VkDescriptorSet descriptorSet, uint32_t binding, VkDescriptorType type) {
    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorType = type,
        .descriptorCount = 1,
        .pBufferInfo = bufferInfo,
        .pImageInfo = imageInfo,
    };
    return descriptorWrite;
}

void createDescriptorSets(Vulkan *vulkan, DescriptorSet *descriptorSet,
                          Texture *texture) {
    VkDescriptorSetLayout layouts[vulkan->swapchain.swapChainImagesCount];
    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        layouts[i] = descriptorSet->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorSet->descriptorPool,
        .descriptorSetCount = vulkan->swapchain.swapChainImagesCount,
        .pSetLayouts = layouts,
    };

    descriptorSet->descriptorSets =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*descriptorSet->descriptorSets));
    if (vkAllocateDescriptorSets(vulkan->device.device, &allocInfo,
                                 descriptorSet->descriptorSets) != VK_SUCCESS) {
        THROW_ERROR("failed to allocate descriptor sets!\n");
    }

    VkDescriptorImageInfo imageInfo = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView = texture->textureImageView,
        .sampler = texture->textureSampler,
    };

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = descriptorSet->uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject),
        };

        VkWriteDescriptorSet descriptorWrites[] = {
            createWriteDescriptorInfo(&bufferInfo, NULL,
                                      descriptorSet->descriptorSets[i], 0,
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
            createWriteDescriptorInfo(
                NULL, &imageInfo, descriptorSet->descriptorSets[i], 1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
        };

        vkUpdateDescriptorSets(vulkan->device.device, SIZEOF(descriptorWrites),
                               descriptorWrites, 0, NULL);
    }
}

static inline VkDescriptorSetLayoutBinding
createDescriptorSetLayoutBinding(VkDescriptorType dType,
                                 VkShaderStageFlags sFlags, uint32_t binding) {
    VkDescriptorSetLayoutBinding layoutBinding = {
        .binding = binding,
        .descriptorCount = 1,
        .descriptorType = dType,
        .pImmutableSamplers = NULL, // Optional
        .stageFlags = sFlags,
    };
    return layoutBinding;
}

inline void
createDescriptorSetLayout(Vulkan *vulkan,
                          VkDescriptorSetLayout *descriptorSetLayout) {
    VkDescriptorSetLayoutBinding bindings[] = {
        createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT, 0),
        createDescriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT, 1),
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = SIZEOF(bindings),
        .pBindings = bindings,
    };

    if (vkCreateDescriptorSetLayout(vulkan->device.device, &layoutInfo, NULL,
                                    descriptorSetLayout) != VK_SUCCESS) {
        THROW_ERROR("failed to create descriptor set layout!\n");
    }
}

inline void createUniformBuffers(Vulkan *vulkan, DescriptorSet *descriptorSet) {
    descriptorSet->uniformBuffers =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*descriptorSet->uniformBuffers));
    descriptorSet->uniformBuffersMemory =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*descriptorSet->uniformBuffersMemory));

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        createBuffer(sizeof(UniformBufferObject),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &descriptorSet->uniformBuffers[i],
                     &descriptorSet->uniformBuffersMemory[i]);
    }
}

inline void updateUniformBuffer(Vulkan *vulkan, DescriptorSet *descriptorSet,
                                uint32_t currentImage) {

    // glm_rotate(vulkan->ubo.model, -vulkan->window.dt * glm_rad(95.0f),
    // GLM_ZUP);

    glm_rotate(vulkan->ubo.model, vulkan->window.mX * glm_rad(1.0) * 0.005,
               GLM_ZUP);
    glm_rotate(vulkan->ubo.model, -vulkan->window.mY * glm_rad(1.0) * 0.005,
               GLM_XUP);

    glm_vec3_rotate(vulkan->camera.eye, 0.005f, GLM_ZUP);
    glm_lookat(vulkan->camera.eye, GLM_VEC3_ZERO, GLM_ZUP, vulkan->ubo.view);
    glm_rotate(vulkan->ubo.view, GLM_PI, GLM_XUP);

    float aspectRatio = vulkan->swapchain.swapChainExtent->width /
                        (float)vulkan->swapchain.swapChainExtent->height;
    glm_perspective(glm_rad(45.0f), aspectRatio, 0.1f, 10.0f, vulkan->ubo.proj);

    vulkan->ubo.proj[1][1] *= -1;
}
