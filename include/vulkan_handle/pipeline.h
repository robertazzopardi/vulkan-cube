#ifndef INCLUDE_VULKAN_HANDLE_PIPELINE
#define INCLUDE_VULKAN_HANDLE_PIPELINE

typedef struct VkPipelineLayout_T *VkPipelineLayout;
typedef struct VkPipeline_T *VkPipeline;
typedef struct VkRenderPass_T *VkRenderPass;

typedef struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;
} GraphicsPipeline;

typedef struct Vulkan Vulkan;
typedef enum VkFormat VkFormat;

void createGraphicsPipeline(Vulkan *);

void createRenderPass(Vulkan *);

VkFormat findDepthFormat(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_PIPELINE */
