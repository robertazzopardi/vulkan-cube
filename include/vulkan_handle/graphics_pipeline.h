#ifndef INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE
#define INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE

#include "graphics_pipeline.h"
#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct GraphicsPipeline GraphicsPipeline;

struct GraphicsPipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;
};

void createGraphicsPipeline(Vulkan *);

void createRenderPass(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_GRAPHICS_PIPELINE */
