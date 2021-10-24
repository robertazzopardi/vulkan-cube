#include "vulkan_handle/pipeline.h"
#include "error_handle.h"
#include "geometry/geometry.h"
#include "shaders/shader.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <vulkan/vulkan.h>

void createShaderModule(unsigned char *code, uint32_t length, VkDevice device,
                        VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = length,
        .pCode = (uint32_t *)code,
    };

    if (vkCreateShaderModule(device, &createInfo, NULL, shaderModule) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create shader module!\n");
    }
}

static inline VkPipelineShaderStageCreateInfo
createPipelineShaderInfo(VkShaderStageFlagBits stage, VkShaderModule module) {
    VkPipelineShaderStageCreateInfo shaderInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = module,
        .pName = "main",
    };
    return shaderInfo;
}

void createGraphicsPipeline(Vulkan *vulkan) {
    VkShaderModule vertShaderModule;
    createShaderModule(SRC_SHADERS_TEXTURE_VERT_SPV,
                       SRC_SHADERS_TEXTURE_VERT_SPV_LEN, vulkan->device.device,
                       &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(SRC_SHADERS_TEXTURE_FRAG_SPV,
                       SRC_SHADERS_TEXTURE_FRAG_SPV_LEN, vulkan->device.device,
                       &fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        createPipelineShaderInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule),
        createPipelineShaderInfo(VK_SHADER_STAGE_FRAGMENT_BIT,
                                 fragShaderModule),
    };

    VkVertexInputBindingDescription bindingDescription =
        getBindingDescription();
    VkVertexInputAttributeDescription *attributeDescriptions =
        getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .vertexAttributeDescriptionCount = 4,
        .pVertexBindingDescriptions = &bindingDescription,
        .pVertexAttributeDescriptions = attributeDescriptions,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)vulkan->swapchain.swapChainExtent->width,
        .height = (float)vulkan->swapchain.swapChainExtent->height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = (VkOffset2D){0, 0},
        .extent = *vulkan->swapchain.swapChainExtent,
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        // .polygonMode = VK_POLYGON_MODE_LINE,
        // .polygonMode = VK_POLYGON_MODE_POINT,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        // .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = vulkan->msaaSamples,
        .sampleShadingEnable = VK_TRUE, // enable sample shading in the pipeline
        .minSampleShading =
            .2f, // min fraction for sample shading; closer to one is smoother
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .pushConstantRangeCount = 0,
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &vulkan->descriptorSet.descriptorSetLayout,
    };

    if (vkCreatePipelineLayout(vulkan->device.device, &pipelineLayoutInfo, NULL,
                               &vulkan->graphicsPipeline.pipelineLayout) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create pipeline layout!\n");
    }

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDynamicState = &dynamicStateCreateInfo,
        .pColorBlendState = &colorBlending,
        .layout = vulkan->graphicsPipeline.pipelineLayout,
        .renderPass = vulkan->graphicsPipeline.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .pDepthStencilState = &depthStencil,
    };

    if (vkCreateGraphicsPipelines(
            vulkan->device.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
            &vulkan->graphicsPipeline.graphicsPipeline) != VK_SUCCESS) {
        THROW_ERROR("failed to create graphics pipeline!\n");
    }

    vkDestroyShaderModule(vulkan->device.device, fragShaderModule, NULL);
    vkDestroyShaderModule(vulkan->device.device, vertShaderModule, NULL);

    freeMem(1, attributeDescriptions);
}

VkFormat findSupportedFormat(const VkFormat *candidates, size_t length,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features,
                             VkPhysicalDevice physicalDevice) {
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

inline VkFormat findDepthFormat(Vulkan *vulkan) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(candidates, SIZEOF(candidates),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               vulkan->device.physicalDevice);
}

void createRenderPass(Vulkan *vulkan) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = *vulkan->swapchain.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.samples = vulkan->msaaSamples;

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
    colorAttachmentResolve.format = *vulkan->swapchain.swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
                           &vulkan->graphicsPipeline.renderPass) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create render pass!\n");
    }
}
