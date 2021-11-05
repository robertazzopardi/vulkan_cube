#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "vulkan_handle/vulkan_handle.h"
#include "error_handle.h"
#include "vulkan_handle/device.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/swapchain.h"
#include "vulkan_handle/texture.h"
#include "vulkan_handle/validation.h"
#include "window/window.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

void createInstance(Vulkan *vulkan) {
    vulkan->device.physicalDevice = VK_NULL_HANDLE;
    vulkan->msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        THROW_ERROR("validation layers requested, but not available!\n");
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = APP_NAME,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    // Get the required extension count
    uint32_t extensionCount;
    if (!SDL_Vulkan_GetInstanceExtensions(vulkan->window.win, &extensionCount,
                                          NULL)) {
        THROW_ERROR("Could not get instance extensions count\n");
    }

    const char *extensionNames[extensionCount + 1];
    if (!SDL_Vulkan_GetInstanceExtensions(vulkan->window.win, &extensionCount,
                                          extensionNames)) {
        THROW_ERROR("Could not get instance extensions names\n");
    }
    extensionNames[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = SIZEOF(extensionNames),
        .ppEnabledExtensionNames = extensionNames,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &vulkan->instance) != VK_SUCCESS) {
        THROW_ERROR("failed to create instance!\n");
    }
}

void initVulkan(Vulkan *vulkan) {
    createInstance(vulkan);

    setupDebugMessenger(vulkan);

    createSurface(vulkan);

    pickPhysicalDevice(vulkan);

    createLogicalDevice(vulkan);

    createSwapChain(vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createColorResources(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(vulkan);

    createTextureImage(vulkan);

    createTextureImageView(vulkan);

    createTextureSampler(vulkan);

    // generateShape(vulkan, CUBE);
    // generateShape(vulkan, SPHERE);
    generateShape(vulkan, CIRCLE);

    vulkan->shapeBuffers.vertexBuffer =
        malloc(vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.vertexBuffer));
    vulkan->shapeBuffers.vertexBufferMemory = malloc(
        vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.vertexBufferMemory));
    vulkan->shapeBuffers.indexBuffer =
        malloc(vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.indexBuffer));
    vulkan->shapeBuffers.indexBufferMemory = malloc(
        vulkan->shapeCount * sizeof(*vulkan->shapeBuffers.indexBufferMemory));

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        createVertexIndexBuffer(vulkan, vulkan->shapes[i].vertices,
                                sizeof(*vulkan->shapes[i].vertices) *
                                    vulkan->shapes[i].verticesCount,
                                &vulkan->shapeBuffers.vertexBuffer[i],
                                &vulkan->shapeBuffers.vertexBufferMemory[i],
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        createVertexIndexBuffer(vulkan, vulkan->shapes[i].indices,
                                sizeof(*vulkan->shapes[i].indices) *
                                    vulkan->shapes[i].indicesCount,
                                &vulkan->shapeBuffers.indexBuffer[i],
                                &vulkan->shapeBuffers.indexBufferMemory[i],
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Vulkan *vulkan) {
    vkDeviceWaitIdle(vulkan->device.device);

    cleanupSwapChain(vulkan);

    vkDestroySampler(vulkan->device.device, vulkan->texture.textureSampler,
                     NULL);
    vkDestroyImageView(vulkan->device.device, vulkan->texture.textureImageView,
                       NULL);

    vkDestroyImage(vulkan->device.device, vulkan->texture.textureImage, NULL);
    vkFreeMemory(vulkan->device.device, vulkan->texture.textureImageMemory,
                 NULL);

    vkDestroyDescriptorSetLayout(
        vulkan->device.device, vulkan->descriptorSet.descriptorSetLayout, NULL);

    if (vulkan->graphicsPipeline.graphicsPipeline == VK_NULL_HANDLE) {
        vkDestroyPipeline(vulkan->device.device,
                          vulkan->graphicsPipeline.graphicsPipeline, NULL);
    }
    if (vulkan->graphicsPipeline.pipelineLayout == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vulkan->device.device,
                                vulkan->graphicsPipeline.pipelineLayout, NULL);
    }

    for (uint32_t i = 0; i < vulkan->shapeCount; i++) {
        vkDestroyBuffer(vulkan->device.device,
                        vulkan->shapeBuffers.indexBuffer[i], NULL);
        vkFreeMemory(vulkan->device.device,
                     vulkan->shapeBuffers.indexBufferMemory[i], NULL);

        vkDestroyBuffer(vulkan->device.device,
                        vulkan->shapeBuffers.vertexBuffer[i], NULL);
        vkFreeMemory(vulkan->device.device,
                     vulkan->shapeBuffers.vertexBufferMemory[i], NULL);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.renderFinishedSemaphores[i],
                           NULL);
        vkDestroySemaphore(vulkan->device.device,
                           vulkan->semaphores.imageAvailableSemaphores[i],
                           NULL);
        vkDestroyFence(vulkan->device.device,
                       vulkan->semaphores.inFlightFences[i], NULL);
    }

    freeMem(11, vulkan->descriptorSet.descriptorSets,
            vulkan->semaphores.renderFinishedSemaphores,
            vulkan->semaphores.imageAvailableSemaphores,
            vulkan->semaphores.inFlightFences,
            vulkan->semaphores.imagesInFlight,
            vulkan->renderBuffers.commandBuffers, vulkan->shapes,

            vulkan->shapeBuffers.indexBuffer,
            vulkan->shapeBuffers.indexBufferMemory,
            vulkan->shapeBuffers.vertexBuffer,
            vulkan->shapeBuffers.vertexBufferMemory

    );

    vkDestroyCommandPool(vulkan->device.device,
                         vulkan->renderBuffers.commandPool, NULL);

    vkDestroyDevice(vulkan->device.device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan->instance,
                                      vulkan->validation.debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(vulkan->instance, vulkan->window.surface, NULL);
    vkDestroyInstance(vulkan->instance, NULL);
}

inline Vulkan initialise() {
    initSDL();

    Vulkan vulkan = {0};
    vulkan.window = createWindow();

    initVulkan(&vulkan);

    return vulkan;
}

inline void terminate(Vulkan *vulkan) {
    cleanUpVulkan(vulkan);

    freeMem(1, vulkan->window.event);

    SDL_DestroyWindow(vulkan->window.win);

    IMG_Quit();

    SDL_Quit();
}
