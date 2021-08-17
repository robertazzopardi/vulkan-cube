#ifndef INCLUDE_VULKAN_HANDLE_MEMORY
#define INCLUDE_VULKAN_HANDLE_MEMORY

#include <stdint.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))
// #define LENGTH(arr) (sizeof(arr[0]) * SIZEOF(arr))

typedef struct Vulkan Vulkan;

void freeMem(const size_t, ...);

uint32_t findMemoryType(Vulkan *, uint32_t, VkMemoryPropertyFlags);

#endif /* INCLUDE_VULKAN_HANDLE_MEMORY */
