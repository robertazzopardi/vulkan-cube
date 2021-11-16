#include "vulkan_handle/pipeline.h"
#include "error_handle.h"
#include "geometry/geometry.h"
#include "shaders/shader.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <vulkan/vulkan.h>

static inline void createShaderModule(unsigned char *code, uint32_t length,
                                      VkDevice device,
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

void createGraphicsPipeline(Vulkan *vulkan,
                            VkDescriptorSetLayout *descriptorSetLayout,
                            GraphicsPipeline *graphicsPipeline) {

    VkShaderModule vertShaderModule;
    createShaderModule(SRC_SHADERS_LIGHT_TEXTURE_VERT_SPV,
                       SRC_SHADERS_LIGHT_TEXTURE_VERT_SPV_LEN,
                       vulkan->device.device, &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(SRC_SHADERS_LIGHT_TEXTURE_FRAG_SPV,
                       SRC_SHADERS_LIGHT_TEXTURE_FRAG_SPV_LEN,
                       vulkan->device.device, &fragShaderModule);

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
        .topology = graphicsPipeline->topology,
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
        .cullMode = graphicsPipeline->cullMode,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = vulkan->msaaSamples,
        .sampleShadingEnable = VK_TRUE, // enable sample shading in the pipeline
        .minSampleShading = .2f,
        .pSampleMask = NULL,               // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable = VK_FALSE,      // Optional
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
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
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
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 0,
        .setLayoutCount = 1,
        .pSetLayouts = descriptorSetLayout,
    };

    if (vkCreatePipelineLayout(vulkan->device.device, &pipelineLayoutInfo, NULL,
                               &graphicsPipeline->pipelineLayout) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to create pipeline layout!\n");
    }

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

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
        .layout = graphicsPipeline->pipelineLayout,
        .renderPass = vulkan->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .pDepthStencilState = &depthStencil,
    };

    if (vkCreateGraphicsPipelines(
            vulkan->device.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
            &graphicsPipeline->graphicsPipeline) != VK_SUCCESS) {
        THROW_ERROR("failed to create graphics pipeline!\n");
    }

    vkDestroyShaderModule(vulkan->device.device, fragShaderModule, NULL);
    vkDestroyShaderModule(vulkan->device.device, vertShaderModule, NULL);

    freeMem(1, attributeDescriptions);
}

static inline VkFormat findSupportedFormat(const VkFormat *candidates,
                                           size_t length, VkImageTiling tiling,
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
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    return findSupportedFormat(candidates, SIZEOF(candidates),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                               vulkan->device.physicalDevice);
}

void createRenderPass(Vulkan *vulkan) {
    VkAttachmentDescription colorAttachment = {
        .format = *vulkan->swapchain.swapChainImageFormat,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .samples = vulkan->msaaSamples,
    };

    VkAttachmentDescription depthAttachment = {
        .format = findDepthFormat(vulkan),
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .samples = vulkan->msaaSamples,
    };

    VkAttachmentDescription colorAttachmentResolve = {
        .format = *vulkan->swapchain.swapChainImageFormat,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .samples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference colorAttachmentResolveRef = {
        .attachment = 2,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
        .pResolveAttachments = &colorAttachmentResolveRef,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcAccessMask = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    VkAttachmentDescription attachments[] = {
        colorAttachment,
        depthAttachment,
        colorAttachmentResolve,
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = SIZEOF(attachments),
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(vulkan->device.device, &renderPassInfo, NULL,
                           &vulkan->renderPass) != VK_SUCCESS) {
        THROW_ERROR("failed to create render pass!\n");
    }
}
