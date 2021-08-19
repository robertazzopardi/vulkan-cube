#ifndef INCLUDE_VULKAN_HANDLE_SWAPCHAIN
#define INCLUDE_VULKAN_HANDLE_SWAPCHAIN

typedef struct VkSurfaceCapabilitiesKHR VkSurfaceCapabilitiesKHR;
typedef struct VkSurfaceFormatKHR VkSurfaceFormatKHR;
typedef enum VkPresentModeKHR VkPresentModeKHR;
typedef unsigned int uint32_t;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR *capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

typedef struct VkSwapchainKHR_T *VkSwapchainKHR;
typedef struct VkImage_T *VkImage;
typedef enum VkFormat VkFormat;
typedef struct VkExtent2D VkExtent2D;
typedef struct VkImageView_T *VkImageView;

typedef struct SwapChain {
    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkFormat *swapChainImageFormat;
    VkExtent2D *swapChainExtent;
    VkImageView *swapChainImageViews;
} SwapChain;

typedef struct Vulkan Vulkan;
typedef struct Window Window;

void createSwapChain(Window *, Vulkan *);

void recreateSwapChain(Window *, Vulkan *);

void cleanupSwapChain(Vulkan *);

typedef struct VkPhysicalDevice_T *VkPhysicalDevice;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);

#endif /* INCLUDE_VULKAN_HANDLE_SWAPCHAIN */
