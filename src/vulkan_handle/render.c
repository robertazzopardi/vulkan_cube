#include "vulkan_handle/render.h"
#include "geometry/geometry.h"
#include "utility/error_handle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include "window/window.h"
#include <SDL_events.h>
#include <SDL_video.h>
#include <vulkan/vulkan.h>

inline void createCommandPool(Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(
        vulkan->device.physicalDevice, vulkan->window.surface);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily,
    };

    if (vkCreateCommandPool(vulkan->device.device, &poolInfo, NULL,
                            &vulkan->renderBuffers.commandPool) != VK_SUCCESS) {
        THROW_ERROR("failed to create command pool!\n");
    }
}

void createCommandBuffers(Vulkan *vulkan) {
    vulkan->renderBuffers.commandBuffers =
        malloc(vulkan->swapchain.swapChainImagesCount *
               sizeof(*vulkan->renderBuffers.commandBuffers));

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkan->renderBuffers.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = vulkan->swapchain.swapChainImagesCount,
    };

    if (vkAllocateCommandBuffers(vulkan->device.device, &allocInfo,
                                 vulkan->renderBuffers.commandBuffers) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to allocate command renderBuffers!\n");
    }

    int width, height;
    SDL_GetWindowSize(vulkan->window.win, &width, &height);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = width,
        .height = height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = (VkOffset2D) {0, 0},
        .extent = (VkExtent2D) {width, height},
    };

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vulkan->renderPass;
    renderPassInfo.renderArea.offset = (VkOffset2D) {0, 0};
    renderPassInfo.renderArea.extent = *vulkan->swapchain.swapChainExtent;

    VkClearValue clearValues[] = {
        {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
        {{{1.0f, 0}}},
    };

    VkDeviceSize offsets[] = {0};

    for (uint32_t i = 0; i < vulkan->swapchain.swapChainImagesCount; i++) {

        if (vkBeginCommandBuffer(vulkan->renderBuffers.commandBuffers[i],
                                 &beginInfo) != VK_SUCCESS) {
            THROW_ERROR("failed to begin recording command buffer!\n");
        }

        renderPassInfo.framebuffer =
            vulkan->renderBuffers.swapChainFramebuffers[i];

        renderPassInfo.clearValueCount = SIZEOF(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(vulkan->renderBuffers.commandBuffers[i],
                             &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(vulkan->renderBuffers.commandBuffers[i], 0, 1,
                         &viewport);

        vkCmdSetScissor(vulkan->renderBuffers.commandBuffers[i], 0, 1,
                        &scissor);

        for (uint32_t j = 0; j < vulkan->shapeCount; j++) {
            vkCmdBindPipeline(
                vulkan->renderBuffers.commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                vulkan->shapes[j].graphicsPipeline.graphicsPipeline);

            vkCmdBindDescriptorSets(
                vulkan->renderBuffers.commandBuffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                vulkan->shapes[j].graphicsPipeline.pipelineLayout, 0, 1,
                &vulkan->shapes[j].descriptorSet.descriptorSets[i], 0, NULL);

            VkBuffer vertexBuffers[] = {
                vulkan->shapeBuffers.vertexBuffer[j],
            };

            vkCmdBindVertexBuffers(vulkan->renderBuffers.commandBuffers[i], 0,
                                   1, vertexBuffers, offsets);

            vkCmdDraw(vulkan->renderBuffers.commandBuffers[i],
                      vulkan->shapes[j].verticesCount, 1, 0, 0);

            if (vulkan->shapes[j].indexed) {
                vkCmdBindIndexBuffer(vulkan->renderBuffers.commandBuffers[i],
                                     vulkan->shapeBuffers.indexBuffer[j], 0,
                                     VK_INDEX_TYPE_UINT16);

                vkCmdDrawIndexed(vulkan->renderBuffers.commandBuffers[i],
                                 vulkan->shapes[j].indicesCount, 1, 0, 0, 0);
            }
        }

        vkCmdEndRenderPass(vulkan->renderBuffers.commandBuffers[i]);

        if (vkEndCommandBuffer(vulkan->renderBuffers.commandBuffers[i]) !=
            VK_SUCCESS) {
            THROW_ERROR("failed to record command buffer!\n");
        }
    }
}

void createSyncObjects(Vulkan *vulkan) {
    vulkan->semaphores.imageAvailableSemaphores =
        malloc(MAX_FRAMES_IN_FLIGHT *
               sizeof(*vulkan->semaphores.imageAvailableSemaphores));
    vulkan->semaphores.renderFinishedSemaphores =
        malloc(MAX_FRAMES_IN_FLIGHT *
               sizeof(*vulkan->semaphores.renderFinishedSemaphores));
    vulkan->semaphores.inFlightFences = malloc(
        MAX_FRAMES_IN_FLIGHT * sizeof(*vulkan->semaphores.inFlightFences));
    vulkan->semaphores.imagesInFlight =
        calloc(vulkan->swapchain.swapChainImagesCount,
               sizeof(*vulkan->semaphores.imagesInFlight));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(
                vulkan->device.device, &semaphoreInfo, NULL,
                &vulkan->semaphores.imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(
                vulkan->device.device, &semaphoreInfo, NULL,
                &vulkan->semaphores.renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(vulkan->device.device, &fenceInfo, NULL,
                          &vulkan->semaphores.inFlightFences[i]) !=
                VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
        }
    }
}

void drawFrame(Vulkan *vulkan) {
    vkWaitForFences(vulkan->device.device, 1, vulkan->semaphores.inFlightFences,
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vulkan->device.device, vulkan->swapchain.swapChain, UINT64_MAX,
        vulkan->semaphores.imageAvailableSemaphores[vulkan->currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(vulkan);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        THROW_ERROR("failed to acquire swap chain image!\n");
    }

    updateUniformBuffer(vulkan);
    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        mapMemory(
            vulkan->device.device,
            vulkan->shapes[i].descriptorSet.uniformBuffersMemory[imageIndex],
            sizeof(vulkan->ubo), &vulkan->ubo);
    }

    if (vulkan->semaphores.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vulkan->device.device, 1,
                        &vulkan->semaphores.imagesInFlight[imageIndex], VK_TRUE,
                        UINT64_MAX);
    }
    vulkan->semaphores.imagesInFlight[imageIndex] =
        vulkan->semaphores.inFlightFences[vulkan->currentFrame];

    VkSemaphore waitSemaphores[] = {
        vulkan->semaphores.imageAvailableSemaphores[vulkan->currentFrame],
    };
    VkSemaphore signalSemaphores[] = {
        vulkan->semaphores.renderFinishedSemaphores[vulkan->currentFrame],
    };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vulkan->renderBuffers.commandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    vkResetFences(vulkan->device.device, 1,
                  &vulkan->semaphores.inFlightFences[vulkan->currentFrame]);

    if (vkQueueSubmit(
            vulkan->device.presentQueue, 1, &submitInfo,
            vulkan->semaphores.inFlightFences[vulkan->currentFrame]) !=
        VK_SUCCESS) {
        THROW_ERROR("failed to submit draw command buffer!\n");
    }

    VkSwapchainKHR swapChains[] = {
        vulkan->swapchain.swapChain,
    };

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
    };

    result = vkQueuePresentKHR(vulkan->device.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        vulkan->window.windowResized) {
        vulkan->window.windowResized = false;
        recreateSwapChain(vulkan);
    } else if (result != VK_SUCCESS) {
        THROW_ERROR("failed to present swap chain image!\n");
    }

    vulkan->currentFrame = (vulkan->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
