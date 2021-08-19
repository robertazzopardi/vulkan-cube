#ifndef INCLUDE_VULKAN_HANDLE_MEMORY
#define INCLUDE_VULKAN_HANDLE_MEMORY

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))

typedef unsigned long size_t;

void freeMem(const size_t, ...);

typedef struct Vulkan Vulkan;
typedef unsigned int uint32_t;
typedef uint32_t VkFlags;
typedef VkFlags VkMemoryPropertyFlags;

uint32_t findMemoryType(Vulkan *, uint32_t, VkMemoryPropertyFlags);

#endif /* INCLUDE_VULKAN_HANDLE_MEMORY */
