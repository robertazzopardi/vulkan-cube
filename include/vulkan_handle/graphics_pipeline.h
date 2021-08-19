#ifndef INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE
#define INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE

typedef struct VkPipelineLayout_T *VkPipelineLayout;
typedef struct VkPipeline_T *VkPipeline;
typedef struct VkRenderPass_T *VkRenderPass;

typedef struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;
} GraphicsPipeline;

typedef struct Vulkan Vulkan;

void createGraphicsPipeline(Vulkan *);

void createRenderPass(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE */
