#include "vulkan_handle/memory.h"
#include "error_handle.h"
#include "vulkan_handle/vulkan_handle.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

inline void freeMem(const size_t count, ...) {
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

inline void allocateVerticesAndIndices(Vulkan *vulkan, size_t numVertices,
                                       size_t numIndices) {
    vulkan->shapes[vulkan->shapeCount].vertices = malloc(
        numVertices * sizeof(*vulkan->shapes[vulkan->shapeCount].vertices));
    vulkan->shapes[vulkan->shapeCount].indices = malloc(
        numIndices * sizeof(*vulkan->shapes[vulkan->shapeCount].indices));
}

inline uint32_t findMemoryType(Vulkan *vulkan, uint32_t typeFilter,
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

inline void mapMemory(VkDevice device, VkDeviceMemory deviceMemory,
                      VkDeviceSize deviceSize, void *toMap) {
    void *data;
    vkMapMemory(device, deviceMemory, 0, deviceSize, 0, &data);
    memcpy(data, toMap, (size_t)deviceSize);
    vkUnmapMemory(device, deviceMemory);
}
