#ifndef INCLUDE_VULKAN_HANDLE_VALIDATION
#define INCLUDE_VULKAN_HANDLE_VALIDATION

#include <stdbool.h>
#include <vulkan/vulkan.h>

static const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

#define NDEBUG
#ifdef NDEBUG
static const bool enableValidationLayers = false;
#else
static const bool enableValidationLayers = true;
#endif

typedef struct Validation Validation;

typedef struct Vulkan Vulkan;

struct Validation {
    VkDebugUtilsMessengerEXT debugMessenger;
};

bool checkValidationLayerSupport();

typedef uint32_t VkBool32;

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                       VkDebugUtilsMessageTypeFlagsEXT,
                       const VkDebugUtilsMessengerCallbackDataEXT *, void *);

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *);

void setupDebugMessenger(Vulkan *);

void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT,
                                   const VkAllocationCallbacks *);

#endif /* INCLUDE_VULKAN_HANDLE_VALIDATION */
