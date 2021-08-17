#include "vulkan_handle/memory.h"
#include "error_handle.h"
#include "vulkan_handle/vulkan_handle.h"
#include <stdarg.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

void freeMem(const size_t count, ...) {
    va_list valist;

    va_start(valist, count);

    for (size_t i = 0; i < count; i++) {
        void *ptr = va_arg(valist, void *);

        if (ptr) {
            free(ptr);
            ptr = NULL;
        }
    }

    va_end(valist);
}

uint32_t findMemoryType(Vulkan *vulkan, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkan->device.physicalDevice,
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    THROW_ERROR("failed to find suitable memory type!\n");
}