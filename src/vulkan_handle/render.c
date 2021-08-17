#include "vulkan_handle/render.h"
#include "error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include "window/window.h"
#include <SDL_events.h>
#include <SDL_video.h>

void createCommandPool(Window *window, Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->device.physicalDevice, window->surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(vulkan->device.device, &poolInfo, NULL,
                            &vulkan->renderBuffers.commandPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create command pool!\n");
    }
}

void createCommandBuffers(Vulkan *vulkan, Window *window) {
    vulkan->renderBuffers.commandBuffers =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->renderBuffers.commandBuffers));

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkan->renderBuffers.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vulkan->swapchain.swapChainImagesCount;

    if (vkAllocateCommandBuffers(vulkan->device.device, &allocInfo,
                                 vulkan->renderBuffers.commandBuffers) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to allocate command renderBuffers!\n");
    }

    int width, height;
    SDL_GetWindowSize(window->win, &width, &height);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // viewport.width = window.width;
    // viewport.height = window.height;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = (VkOffset2D){0, 0};
    // scissor.extent = (VkExtent2D){window.width, window.height};
    scissor.extent = (VkExtent2D){width, height};

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(vulkan->renderBuffers.commandBuffers[i],
                                 &beginInfo) != VK_SUCCESS) {
            THROW_ERROR("failed to begin recording command buffer!\n");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkan->graphicsPipeline.renderPass;
        renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
        renderPassInfo.renderArea.extent = vulkan->swapchain.swapChainExtent;
        renderPassInfo.framebuffer =
            vulkan->renderBuffers.swapChainFramebuffers[i];

        VkClearValue clearValues[] = {{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
                                      {{{1.0f, 0}}}};

        renderPassInfo.clearValueCount = SIZEOF(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(vulkan->renderBuffers.commandBuffers[i],
                             &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(vulkan->renderBuffers.commandBuffers[i], 0, 1,
                         &viewport);

        vkCmdSetScissor(vulkan->renderBuffers.commandBuffers[i], 0, 1,
                        &scissor);

        vkCmdBindPipeline(vulkan->renderBuffers.commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vulkan->graphicsPipeline.graphicsPipeline);

        VkBuffer vertexBuffers[] = {vulkan->shapeBuffers.vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(vulkan->renderBuffers.commandBuffers[i], 0, 1,
                               vertexBuffers, offsets);

        vkCmdBindIndexBuffer(vulkan->renderBuffers.commandBuffers[i],
                             vulkan->shapeBuffers.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(vulkan->renderBuffers.commandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vulkan->graphicsPipeline.pipelineLayout, 0, 1,
                                &vulkan->ubo.descriptorSets[i], 0, NULL);

        vkCmdDrawIndexed(vulkan->renderBuffers.commandBuffers[i],
                         vulkan->shapes.indicesCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(vulkan->renderBuffers.commandBuffers[i]);

        if (vkEndCommandBuffer(vulkan->renderBuffers.commandBuffers[i]) !=
            VK_SUCCESS) {
            THROW_ERROR("failed to record command buffer!\n");
        }
    }
}

void createSyncObjects(Vulkan *vulkan) {
    VkSemaphore *imageAvailableSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*imageAvailableSemaphoresTemp));
    VkSemaphore *renderFinishedSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*renderFinishedSemaphoresTemp));
    VkFence *inFlightFencesTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*inFlightFencesTemp));
    vulkan->semaphores.imagesInFlight =
        calloc(vulkan->swapchain.swapChainImagesCount,
               sizeof(*vulkan->semaphores.imagesInFlight));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vulkan->device.device, &semaphoreInfo, NULL,
                              &imageAvailableSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan->device.device, &semaphoreInfo, NULL,
                              &renderFinishedSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan->device.device, &fenceInfo, NULL,
                          &inFlightFencesTemp[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
        }
    }

    vulkan->semaphores.imageAvailableSemaphores = imageAvailableSemaphoresTemp;
    vulkan->semaphores.renderFinishedSemaphores = renderFinishedSemaphoresTemp;
    vulkan->semaphores.inFlightFences = inFlightFencesTemp;
}

void drawFrame(Window *window, SDL_Event event, Vulkan *vulkan, float dt) {
    vkWaitForFences(vulkan->device.device, 1,
                    &vulkan->semaphores.inFlightFences[window->currentFrame],
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vulkan->device.device, vulkan->swapchain.swapChain, UINT64_MAX,
        vulkan->semaphores.imageAvailableSemaphores[window->currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(window, event, vulkan);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_ERROR("failed to acquire swap chain image!\n");
    }

    updateUniformBuffer(vulkan, imageIndex, dt);

    if (vulkan->semaphores.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vulkan->device.device, 1,
                        &vulkan->semaphores.imagesInFlight[imageIndex], VK_TRUE,
                        UINT64_MAX);
    }
    vulkan->semaphores.imagesInFlight[imageIndex] =
        vulkan->semaphores.inFlightFences[window->currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        vulkan->semaphores.imageAvailableSemaphores[window->currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &vulkan->renderBuffers.commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {
        vulkan->semaphores.renderFinishedSemaphores[window->currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(vulkan->device.device, 1,
                  &vulkan->semaphores.inFlightFences[window->currentFrame]);

    if (vkQueueSubmit(
            vulkan->device.graphicsQueue, 1, &submitInfo,
            vulkan->semaphores.inFlightFences[window->currentFrame]) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to submit draw command buffer!\n");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkan->swapchain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkan->device.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        vulkan->framebufferResized) {
        vulkan->framebufferResized = false;
        recreateSwapChain(window, event, vulkan);
    } else if (result != VK_SUCCESS) {
        THROW_ERROR("failed to present swap chain image!\n");
    }

    window->currentFrame = (window->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
