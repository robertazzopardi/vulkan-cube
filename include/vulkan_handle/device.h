#ifndef INCLUDE_VULKAN_HANDLE_DEVICE
#define INCLUDE_VULKAN_HANDLE_DEVICE

typedef struct VkPhysicalDevice_T *VkPhysicalDevice;
typedef struct VkDevice_T *VkDevice;
typedef struct VkQueue_T *VkQueue;

typedef struct Device {
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
} Device;

typedef unsigned int uint32_t;

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);

typedef struct Window Window;
typedef struct Vulkan Vulkan;

void createLogicalDevice(Window *, Vulkan *);
void pickPhysicalDevice(Window *, Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_DEVICE */
