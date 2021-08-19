#ifndef INCLUDE_VULKAN_HANDLE_VALIDATION
#define INCLUDE_VULKAN_HANDLE_VALIDATION

#include <stdbool.h>

static const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

#define NDEBUG
#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

typedef struct VkDebugUtilsMessengerEXT_T *VkDebugUtilsMessengerEXT;

typedef struct Validation {
    VkDebugUtilsMessengerEXT debugMessenger;
} Validation;

typedef struct Vulkan Vulkan;

void setupDebugMessenger(Vulkan *);

bool checkValidationLayerSupport();

typedef struct VkDebugUtilsMessengerCreateInfoEXT
    VkDebugUtilsMessengerCreateInfoEXT;

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *);

typedef struct VkInstance_T *VkInstance;
typedef struct VkAllocationCallbacks VkAllocationCallbacks;

void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT,
                                   const VkAllocationCallbacks *);

#endif /* INCLUDE_VULKAN_HANDLE_VALIDATION */
