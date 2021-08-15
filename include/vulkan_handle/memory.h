#ifndef INCLUDE_VULKAN_HANDLE_MEMORY
#define INCLUDE_VULKAN_HANDLE_MEMORY

#include <stdlib.h>

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))
// #define LENGTH(arr) (sizeof(arr[0]) * SIZEOF(arr))

void freeMem(const size_t, ...);

#endif /* INCLUDE_VULKAN_HANDLE_MEMORY */
