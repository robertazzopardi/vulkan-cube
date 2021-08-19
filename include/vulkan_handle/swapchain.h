#ifndef INCLUDE_VULKAN_HANDLE_SWAPCHAIN
#define INCLUDE_VULKAN_HANDLE_SWAPCHAIN

#include <vulkan/vulkan.h>

typedef struct Vulkan Vulkan;

typedef struct Window Window;

typedef struct SwapChainSupportDetails SwapChainSupportDetails;

typedef struct SwapChain SwapChain;

typedef union SDL_Event SDL_Event;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
};

struct SwapChain {
    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);

void createSwapChain(Window *, Vulkan *);

void recreateSwapChain(Window *, Vulkan *);

void cleanupSwapChain(Vulkan *);

#endif /* INCLUDE_VULKAN_HANDLE_SWAPCHAIN */
