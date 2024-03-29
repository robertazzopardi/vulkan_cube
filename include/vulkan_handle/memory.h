#ifndef INCLUDE_VULKAN_HANDLE_MEMORY
#define INCLUDE_VULKAN_HANDLE_MEMORY

#include <stdint.h>

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))

typedef unsigned long size_t;
typedef struct Vulkan Vulkan;
typedef uint32_t VkFlags;
typedef VkFlags VkMemoryPropertyFlags;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef uint64_t VkDeviceSize;
typedef struct VkDevice_T *VkDevice;

void freeMem(const size_t, ...);

uint32_t findMemoryType(Vulkan *, uint32_t, VkMemoryPropertyFlags);

void mapMemory(VkDevice, VkDeviceMemory, VkDeviceSize, void *);

void allocateVerticesAndIndices(Vulkan *, size_t, size_t);

#endif /* INCLUDE_VULKAN_HANDLE_MEMORY */
