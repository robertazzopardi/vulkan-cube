#ifndef INCLUDE_VULKAN_HANDLE_PIPELINE
#define INCLUDE_VULKAN_HANDLE_PIPELINE

#include <vulkan/vulkan.h>

typedef struct VkPipelineLayout_T *VkPipelineLayout;
typedef struct VkPipeline_T *VkPipeline;
typedef struct VkRenderPass_T *VkRenderPass;
typedef struct VkDescriptorSetLayout_T *VkDescriptorSetLayout;

typedef struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkPrimitiveTopology topology;
    VkCullModeFlagBits cullMode;
} GraphicsPipeline;

typedef struct Vulkan Vulkan;
typedef enum VkFormat VkFormat;

void createGraphicsPipeline(Vulkan *, VkDescriptorSetLayout *,
                            GraphicsPipeline *);

void createRenderPass(Vulkan *);

VkFormat findDepthFormat(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_PIPELINE */
