#include <SDL.h>
#include <SDL_vulkan.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct {
    VkInstance instance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkImageView *swapChainImageViews;

    VkRenderPass renderPass;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;

    VkCommandPool commandPool;

    VkCommandBuffer *commandBuffers;

    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    VkFence *imagesInFlight;

    bool framebufferResized;
} Vulkan;

typedef struct {
    SDL_Window *win;
    SDL_Event event;
    bool *framebufferResized;
    int width;
    int height;
} Window;

const Uint32 FRAME_DELAY = 1000 / 60;
const char *APP_NAME = "App";
const int WIDTH_INIT = 1280;
const int HEIGHT_INIT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;

const uint32_t MAX_FAMILY = 1000;

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))

const char *validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"};

const char *deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#pragma region CREATING INSTANCE
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < SIZEOF(validationLayers); i++) {
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity __attribute__((unused)),
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType __attribute__((unused)),
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData __attribute__((unused))) {
    printf("validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

void createInstance(SDL_Window *window, Vulkan *vulkan) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        printf("validation layers requested, but not available!\n");
        exit(-1);
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Get the required extension count
    uint32_t extensionCount;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL)) {
        printf("Could not get instance extensions count\n");
        exit(-1);
    }

    const char *extensionNames[extensionCount + 1];
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames)) {
        printf("Could not get instance extensions names\n");
        exit(-1);
    }
    extensionNames[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = SIZEOF(extensionNames);
    createInfo.ppEnabledExtensionNames = extensionNames;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &vulkan->instance) != VK_SUCCESS) {
        printf("failed to create instance!\n");
        exit(-1);
    }
}
#pragma endregion

#pragma region DEBUG MESSENGER
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    printf("%d\n", func == NULL);
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setupDebugMessenger(Vulkan *vulkan) {
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(vulkan->instance, &createInfo, NULL, &vulkan->debugMessenger) != VK_SUCCESS) {
        printf("failed to set up debug messenger!\n");
        exit(-1);
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}
#pragma endregion

#pragma region CREATE SURFACE
void createSurface(SDL_Window *window, Vulkan *vulkan) {
    if (!SDL_Vulkan_CreateSurface(window, vulkan->instance, &vulkan->surface)) {
        // failed to create a surface!
        printf("failed to create window surface!\n");
        exit(-1);
    }
}

static int resizingEventCallback(void *data, SDL_Event *event) {
    Window *mainWindow = (Window *)data;

    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window *win = SDL_GetWindowFromID(event->window.windowID);

        if (win == mainWindow->win) {
            // printf("resizing.....\n");
            *mainWindow->framebufferResized = true;

            SDL_GetWindowSize(win, &mainWindow->width, &mainWindow->height);
        }
    }
    return 0;
}

#pragma endregion

#pragma region PHYSICAL DEVICE
typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

bool isComplete(QueueFamilyIndices queueFamilyIndices) {
    return queueFamilyIndices.graphicsFamily != MAX_FAMILY && queueFamilyIndices.presentFamily != MAX_FAMILY;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {.graphicsFamily = MAX_FAMILY, .presentFamily = MAX_FAMILY};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    int i = 0;
    for (uint32_t j = 0; j < queueFamilyCount; j++) {
        if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (isComplete(indices)) {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details = {
        .formats = NULL,
        .presentModes = NULL,
        .formatCount = 0,
        .presentModeCount = 0,
    };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, NULL);

    if (details.formatCount != 0) {
        details.formats = malloc(details.formatCount * sizeof(*details.formats));
        if (details.formats == NULL) {
            printf("Could not init SwapChainSupportDetails formats\n");
            exit(-1);
        }
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, NULL);

    if (details.presentModeCount != 0) {
        details.presentModes = malloc(details.presentModeCount * sizeof(*details.presentModes));
        if (details.presentModes == NULL) {
            printf("Could not init SwapChainSupportDetails presentModes\n");
            exit(-1);
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModeCount, details.presentModes);
    }

    return details;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    const char *requiredExtensions[SIZEOF(deviceExtensions)];
    for (uint32_t i = 0; i < SIZEOF(deviceExtensions); i++) {
        requiredExtensions[i] = deviceExtensions[i];
    }

    for (uint32_t i = 0; i < extensionCount; i++) {
        for (uint32_t j = 0; j < SIZEOF(deviceExtensions); j++) {
            if (strcmp(availableExtensions[i].extensionName, requiredExtensions[j]) == 0) {
                requiredExtensions[j] = "";

                break;
            }
        }
    }

    bool empty = false;
    for (uint32_t i = 0; i < SIZEOF(deviceExtensions); i++) {
        if (strcmp(requiredExtensions[i], "") == 0) {
            empty = true;
            break;
        }
    }

    return empty;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = swapChainSupport.formats != NULL && swapChainSupport.presentModes != NULL;

        if (swapChainSupport.formats != NULL) {
            free(swapChainSupport.formats);
            swapChainSupport.formats = NULL;
        }
        if (swapChainSupport.presentModes != NULL) {
            free(swapChainSupport.presentModes);
            swapChainSupport.presentModes = NULL;
        }
    }

    return isComplete(indices) && extensionsSupported && swapChainAdequate;
}

void pickPhysicalDevice(Vulkan *vulkan) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!\n");
        exit(-1);
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, devices);

    for (uint32_t i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], vulkan->surface)) {
            vulkan->physicalDevice = devices[i];
            break;
        }
    }

    if (vulkan->physicalDevice == VK_NULL_HANDLE) {
        printf("failed to find a suitable GPU!\n");
        exit(-1);
    }
}
#pragma endregion

#pragma region LOGICAL DEVICE
void createLogicalDevice(Vulkan *vulkan) {
    QueueFamilyIndices indices = findQueueFamilies(vulkan->physicalDevice, vulkan->surface);

    uint32_t uniqueQueueFamilies[] = {indices.graphicsFamily, indices.presentFamily};
    VkDeviceQueueCreateInfo queueCreateInfos[SIZEOF(uniqueQueueFamilies)];

    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < SIZEOF(uniqueQueueFamilies); i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = SIZEOF(queueCreateInfos);
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.enabledExtensionCount = SIZEOF(deviceExtensions);
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = SIZEOF(validationLayers);
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vulkan->physicalDevice, &createInfo, NULL, &vulkan->device) != VK_SUCCESS) {
        printf("failed to create logical device!\n");
        exit(-1);
    }

    vkGetDeviceQueue(vulkan->device, indices.graphicsFamily, 0, &vulkan->graphicsQueue);
    vkGetDeviceQueue(vulkan->device, indices.presentFamily, 0, &vulkan->presentQueue);
}
#pragma endregion

#pragma region SWAP CHAIN
VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities, SDL_Window *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

        if (actualExtent.width > capabilities.maxImageExtent.width) {
            actualExtent.width = capabilities.maxImageExtent.width;
        }
        if (actualExtent.width < capabilities.minImageExtent.width) {
            actualExtent.width = capabilities.minImageExtent.width;
        }

        if (actualExtent.height > capabilities.maxImageExtent.height) {
            actualExtent.height = capabilities.maxImageExtent.height;
        }
        if (actualExtent.height < capabilities.minImageExtent.height) {
            actualExtent.height = capabilities.minImageExtent.height;
        }

        return actualExtent;
    }
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void createSwapChain(SDL_Window *window, Vulkan *vulkan) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkan->physicalDevice, vulkan->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    vulkan->swapChainImagesCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && vulkan->swapChainImagesCount > swapChainSupport.capabilities.maxImageCount) {
        vulkan->swapChainImagesCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkan->surface;

    createInfo.minImageCount = vulkan->swapChainImagesCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(vulkan->physicalDevice, vulkan->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vulkan->device, &createInfo, NULL, &vulkan->swapChain) != VK_SUCCESS) {
        printf("failed to create swap chain!\n");
        exit(-1);
    }

    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain, &vulkan->swapChainImagesCount, NULL);

    VkImage *swapChainImagesTemp = malloc(vulkan->swapChainImagesCount * sizeof(VkImage));
    if (swapChainImagesTemp == NULL) {
        printf("Could not init swap chain images\n");
        exit(-1);
    }
    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain, &vulkan->swapChainImagesCount, swapChainImagesTemp);

    vulkan->swapChainImageFormat = surfaceFormat.format;
    vulkan->swapChainExtent = extent;
    vulkan->swapChainImages = swapChainImagesTemp;
}
#pragma endregion

#pragma region IMAGE VIEWS
void createImageViews(Vulkan *vulkan) {
    VkImageView *tmpImageViews = malloc(vulkan->swapChainImagesCount * sizeof(VkImageView));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vulkan->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vulkan->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vulkan->device, &createInfo, NULL, &tmpImageViews[i]) != VK_SUCCESS) {
            printf("failed to create image views!\n");
            exit(-1);
        }
    }

    vulkan->swapChainImageViews = tmpImageViews;
}
#pragma endregion

#pragma region RENDER PASS
void createRenderPass(Vulkan *vulkan) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = vulkan->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vulkan->device, &renderPassInfo, NULL, &vulkan->renderPass) != VK_SUCCESS) {
        printf("failed to create render pass!\n");
        exit(-1);
    }
}
#pragma endregion

#pragma region GRAPHICS PIPELINE
void createShaderModule(char *code, uint32_t length, VkDevice device, VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = length;
    createInfo.pCode = (uint32_t *)code;

    if (vkCreateShaderModule(device, &createInfo, NULL, shaderModule) != VK_SUCCESS) {
        printf("failed to create shader module!\n");
        exit(-1);
    }
}

typedef struct {
    char *buff;
    uint32_t len;
} FileData;

FileData readFile(const char *path) {
    FileData data;

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("File not found: %s\n", path);
        exit(-1);
    }

    // get filesize
    fseek(file, 0, SEEK_END);
    data.len = ftell(file);

    fseek(file, 0, SEEK_SET);
    // allocate buffer **note** that if you like
    // to use the buffer as a c-string then you must also
    // allocate space for the terminating null character
    data.buff = malloc(data.len);
    // read the file into buffer
    fread(data.buff, data.len, 1, file);
    // close the file
    fclose(file);

    return data;
}

void createGraphicsPipeline(Vulkan *vulkan) {
    FileData vertShaderCode = readFile("shaders/vert.spv");
    FileData fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule;
    createShaderModule(vertShaderCode.buff, vertShaderCode.len, vulkan->device, &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(fragShaderCode.buff, fragShaderCode.len, vulkan->device, &fragShaderModule);

    free(vertShaderCode.buff);
    free(fragShaderCode.buff);
    vertShaderCode.buff = NULL;
    fragShaderCode.buff = NULL;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vulkan->swapChainExtent.width;
    viewport.height = (float)vulkan->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = vulkan->swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(vulkan->device, &pipelineLayoutInfo, NULL, &vulkan->pipelineLayout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!\n");
        exit(-1);
    }

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.pNext = NULL;
    dynamicStateCreateInfo.flags = 0;
    dynamicStateCreateInfo.dynamicStateCount = 2;
    dynamicStateCreateInfo.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = vulkan->pipelineLayout;
    pipelineInfo.renderPass = vulkan->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vulkan->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vulkan->graphicsPipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!\n");
        exit(-1);
    }

    vkDestroyShaderModule(vulkan->device, fragShaderModule, NULL);
    vkDestroyShaderModule(vulkan->device, vertShaderModule, NULL);
}
#pragma endregion

#pragma region FRAME BUFFERS
void createFramebuffers(Vulkan *vulkan) {
    VkFramebuffer *swapChainFramebuffersTemp = malloc(vulkan->swapChainImagesCount * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkImageView attachments[] = {vulkan->swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkan->swapChainExtent.width;
        framebufferInfo.height = vulkan->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkan->device, &framebufferInfo, NULL, &swapChainFramebuffersTemp[i]) != VK_SUCCESS) {
            printf("failed to create framebuffer!\n");
            exit(-1);
        }
    }

    vulkan->swapChainFramebuffers = swapChainFramebuffersTemp;
}
#pragma endregion

#pragma region COMMAND POOL
void createCommandPool(Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkan->physicalDevice, vulkan->surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(vulkan->device, &poolInfo, NULL, &vulkan->commandPool) != VK_SUCCESS) {
        printf("failed to create command pool!\n");
        exit(-1);
    }
}
#pragma endregion

#pragma region COMMAND BUFFER
void createCommandBuffers(Vulkan *vulkan, Window window) {
    VkCommandBuffer *commandBuffersTemp = malloc(vulkan->swapChainImagesCount * sizeof(VkCommandBuffer));

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vulkan->swapChainImagesCount;

    if (vkAllocateCommandBuffers(vulkan->device, &allocInfo, commandBuffersTemp) != VK_SUCCESS) {
        printf("failed to allocate command buffers!\n");
        exit(-1);
    }

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffersTemp[i], &beginInfo) != VK_SUCCESS) {
            printf("failed to begin recording command buffer!\n");
            exit(-1);
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkan->renderPass;
        renderPassInfo.framebuffer = vulkan->swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
        renderPassInfo.renderArea.extent = vulkan->swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffersTemp[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffersTemp[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->graphicsPipeline);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = window.width;
        viewport.height = window.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffersTemp[i], 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset = (VkOffset2D){0, 0};
        scissor.extent = (VkExtent2D){window.width, window.height};
        vkCmdSetScissor(commandBuffersTemp[i], 0, 1, &scissor);

        vkCmdDraw(commandBuffersTemp[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffersTemp[i]);

        if (vkEndCommandBuffer(commandBuffersTemp[i]) != VK_SUCCESS) {
            printf("failed to record command buffer!\n");
        }
    }

    vulkan->commandBuffers = commandBuffersTemp;
}
#pragma endregion

#pragma region SYNC OBJECTS
void createSyncObjects(Vulkan *vulkan) {
    VkSemaphore *imageAvailableSemaphoresTemp = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    VkSemaphore *renderFinishedSemaphoresTemp = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
    VkFence *inFlightFencesTemp = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
    vulkan->imagesInFlight = calloc(vulkan->swapChainImagesCount, sizeof(VkFence));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vulkan->device, &semaphoreInfo, NULL, &imageAvailableSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan->device, &semaphoreInfo, NULL, &renderFinishedSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan->device, &fenceInfo, NULL, &inFlightFencesTemp[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
        }
    }

    vulkan->imageAvailableSemaphores = imageAvailableSemaphoresTemp;
    vulkan->renderFinishedSemaphores = renderFinishedSemaphoresTemp;
    vulkan->inFlightFences = inFlightFencesTemp;
}
#pragma endregion

#pragma region RECREATE SWAP CHAIN
void cleanupSwapChain(Vulkan *vulkan) {

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyFramebuffer(vulkan->device, vulkan->swapChainFramebuffers[i], NULL);
    }

    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool, vulkan->swapChainImagesCount, vulkan->commandBuffers);

    // vkDestroyPipeline(vulkan->device, vulkan->graphicsPipeline, NULL);
    // vkDestroyPipelineLayout(vulkan->device, vulkan->pipelineLayout, NULL);
    vkDestroyRenderPass(vulkan->device, vulkan->renderPass, NULL);

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyImageView(vulkan->device, vulkan->swapChainImageViews[i], NULL);
    }

    if (vulkan->swapChainFramebuffers) {
        free(vulkan->swapChainFramebuffers);
        vulkan->swapChainFramebuffers = NULL;
    }

    if (vulkan->swapChainImageViews) {
        free(vulkan->swapChainImageViews);
        vulkan->swapChainImageViews = NULL;
    }

    vkDestroySwapchainKHR(vulkan->device, vulkan->swapChain, NULL);
}

void recreateSwapChain(Window window, SDL_Event event, Vulkan *vulkan) {
    int width = 0, height = 0;

    SDL_Vulkan_GetDrawableSize(window.win, &width, &height);
    while (width == 0 || height == 0) {
        SDL_Vulkan_GetDrawableSize(window.win, &width, &height);
        SDL_PollEvent(&event);
    }

    vkDeviceWaitIdle(vulkan->device);

    cleanupSwapChain(vulkan);

    createSwapChain(window.win, vulkan);
    createImageViews(vulkan);
    createRenderPass(vulkan);
    // createGraphicsPipeline(vulkan);
    createFramebuffers(vulkan);
    createCommandBuffers(vulkan, window);

    free(vulkan->imagesInFlight);
    vulkan->imagesInFlight = NULL;

    vulkan->imagesInFlight = calloc(vulkan->swapChainImagesCount, sizeof(VkFence));
}
#pragma endregion

#pragma region DRAW FRAME
void drawFrame(Window window, SDL_Event event, Vulkan *vulkan, size_t *currentFrame) {
    vkWaitForFences(vulkan->device, 1, &vulkan->inFlightFences[*currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vulkan->device, vulkan->swapChain, UINT64_MAX, vulkan->imageAvailableSemaphores[*currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(window, event, vulkan);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image!\n");
        exit(-1);
    }

    if (vulkan->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vulkan->device, 1, &vulkan->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    vulkan->imagesInFlight[imageIndex] = vulkan->inFlightFences[*currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vulkan->imageAvailableSemaphores[*currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan->commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {vulkan->renderFinishedSemaphores[*currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(vulkan->device, 1, &vulkan->inFlightFences[*currentFrame]);

    if (vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo, vulkan->inFlightFences[*currentFrame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer!\n");
        exit(-1);
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkan->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkan->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkan->framebufferResized) {
        vulkan->framebufferResized = false;
        recreateSwapChain(window, event, vulkan);
    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image!\n");
        exit(-1);
    }

    *currentFrame = (*currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
#pragma endregion

#pragma region INIT / CLEAN UP VULKAN
void initVulkan(Window window, Vulkan *vulkan) {
    createInstance(window.win, vulkan);

    setupDebugMessenger(vulkan);

    createSurface(window.win, vulkan);

    pickPhysicalDevice(vulkan);

    createLogicalDevice(vulkan);

    createSwapChain(window.win, vulkan);

    createImageViews(vulkan);

    createRenderPass(vulkan);

    createGraphicsPipeline(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(vulkan);

    createCommandBuffers(vulkan, window);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Vulkan vulkan) {
    cleanupSwapChain(&vulkan);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan.device, vulkan.renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(vulkan.device, vulkan.imageAvailableSemaphores[i], NULL);
        vkDestroyFence(vulkan.device, vulkan.inFlightFences[i], NULL);
    }
    free(vulkan.renderFinishedSemaphores);
    vulkan.renderFinishedSemaphores = NULL;
    free(vulkan.imageAvailableSemaphores);
    vulkan.imageAvailableSemaphores = NULL;
    free(vulkan.inFlightFences);
    vulkan.inFlightFences = NULL;
    free(vulkan.imagesInFlight);
    vulkan.imagesInFlight = NULL;

    free(vulkan.commandBuffers);
    vulkan.commandBuffers = NULL;

    vkDestroyCommandPool(vulkan.device, vulkan.commandPool, NULL);

    vkDestroyDevice(vulkan.device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan.instance, vulkan.debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, NULL);
    vkDestroyInstance(vulkan.instance, NULL);
}
#pragma endregion

int main(void) {
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    Window window = {.width = WIDTH_INIT, .height = HEIGHT_INIT};
    window.win = SDL_CreateWindow(APP_NAME,
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  window.width, window.height,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    // INIT VULKAN
    Vulkan vulkan;
    initVulkan(window, &vulkan);

    window.framebufferResized = &vulkan.framebufferResized;

    size_t currentFrame = 0;

    // MAIN LOOP
    SDL_Event event;
    bool running = true;

    while (running) {
        // Handle Events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        // Drawing
        drawFrame(window, event, &vulkan, &currentFrame);

        // Frame rate
        SDL_Delay(FRAME_DELAY);
    }

    vkDeviceWaitIdle(vulkan.device);

    // CLEAN UP
    cleanUpVulkan(vulkan);

    SDL_DestroyWindow(window.win);

    SDL_Quit();

    return 0;
}
