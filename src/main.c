#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_pixels.h>
#include <SDL_vulkan.h>
#include <cglm/affine.h>
#include <cglm/cglm.h>
#include <cglm/io.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

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

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    UniformBufferObject ubo;

    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkCommandPool commandPool;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

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

const float FRAME_DELAY = 1000.0f / 60.0f;
const char *APP_NAME = "Vulkan App";
#define WIDTH_INIT 1280
#define HEIGHT_INIT 720

const int MAX_FRAMES_IN_FLIGHT = 2;

const uint32_t MAX_FAMILY = 1000;

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))
#define LENGTH(arr) (sizeof(arr[0]) * SIZEOF(arr))

const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static inline void freeMem(void *pointer) {
    free(pointer);
    pointer = NULL;
}

#pragma region VERTEX DATA
typedef struct {
    vec3 pos;
    vec3 colour;
    vec2 texCoord;
} Vertex;

// const Vertex vertices[] = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f,
// 0.0f}},
//                            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//                            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//                            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f},
//                            {1.0f, 1.0f}}};
const Vertex vertices[] = {
    // 1
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    // 2
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};

const uint16_t indices[] = {
    // 1
    0, 1, 2, 2, 3, 0,
    // 2
    4, 5, 6, 6, 7, 4};

static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

static VkVertexInputAttributeDescription *getAttributeDescriptions() {
    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(3 * sizeof(*attributeDescriptions));

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, colour);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}
#pragma endregion

#pragma region CREATING INSTANCE
bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < SIZEOF(validationLayers); i++) {
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) ==
                0) {
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

static VkBool32
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity __unused,
              VkDebugUtilsMessageTypeFlagsEXT messageType __unused,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData __unused) {
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        printf("\n");
    }
    printf("validation layer: %s\n", pCallbackData->pMessage);
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        printf("\n");
    }
    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

void createInstance(SDL_Window *window, Vulkan *vulkan) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        printf("validation layers requested, but not available!\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    const char *extensionNames[extensionCount + 1];
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount,
                                          extensionNames)) {
        printf("Could not get instance extensions names\n");
        exit(EXIT_FAILURE);
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
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &vulkan->instance) != VK_SUCCESS) {
        printf("failed to create instance!\n");
        exit(EXIT_FAILURE);
    }
}
#pragma endregion

#pragma region DEBUG MESSENGER
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    // printf("%d\n", func == NULL);
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void setupDebugMessenger(Vulkan *vulkan) {
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(vulkan->instance, &createInfo, NULL,
                                     &vulkan->debugMessenger) != VK_SUCCESS) {
        printf("failed to set up debug messenger!\n");
        exit(EXIT_FAILURE);
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
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
        exit(EXIT_FAILURE);
    }
}

static int resizingEventCallback(void *data, SDL_Event *event) {
    Window *mainWindow = (Window *)data;

    if (event->type == SDL_WINDOWEVENT &&
        event->window.event == SDL_WINDOWEVENT_RESIZED) {
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
    return queueFamilyIndices.graphicsFamily != MAX_FAMILY &&
           queueFamilyIndices.presentFamily != MAX_FAMILY;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {.graphicsFamily = MAX_FAMILY,
                                  .presentFamily = MAX_FAMILY};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies);

    int i = 0;
    for (uint32_t j = 0; j < queueFamilyCount; j++) {
        if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

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

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
    SwapChainSupportDetails details = {};
    details.formats = NULL;
    details.presentModes = NULL;
    details.formatCount = 0;
    details.presentModeCount = 0;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                              &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatCount,
                                         NULL);

    if (details.formatCount != 0) {
        details.formats =
            malloc(details.formatCount * sizeof(*details.formats));
        if (details.formats == NULL) {
            printf("Could not init SwapChainSupportDetails formats\n");
            exit(EXIT_FAILURE);
        }
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &details.formatCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                              &details.presentModeCount, NULL);

    if (details.presentModeCount != 0) {
        details.presentModes =
            malloc(details.presentModeCount * sizeof(*details.presentModes));
        if (details.presentModes == NULL) {
            printf("Could not init SwapChainSupportDetails presentModes\n");
            exit(EXIT_FAILURE);
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &details.presentModeCount, details.presentModes);
    }

    return details;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
                                         availableExtensions);

    for (uint32_t i = 0; i < extensionCount; i++) {
        printf("%s\n", availableExtensions[i].extensionName);
    }

    const char *requiredExtensions[SIZEOF(deviceExtensions)];
    for (uint32_t i = 0; i < SIZEOF(deviceExtensions); i++) {
        requiredExtensions[i] = deviceExtensions[i];
    }

    for (uint32_t i = 0; i < extensionCount; i++) {
        for (uint32_t j = 0; j < SIZEOF(deviceExtensions); j++) {
            if (strcmp(availableExtensions[i].extensionName,
                       requiredExtensions[j]) == 0) {
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
        SwapChainSupportDetails swapChainSupport =
            querySwapChainSupport(device, surface);
        swapChainAdequate = swapChainSupport.formats != NULL &&
                            swapChainSupport.presentModes != NULL;

        if (swapChainSupport.formats != NULL) {
            freeMem(swapChainSupport.formats);
        }
        if (swapChainSupport.presentModes != NULL) {
            freeMem(swapChainSupport.presentModes);
        }
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return isComplete(indices) && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;

    return isComplete(indices) && extensionsSupported && swapChainAdequate;
}

void pickPhysicalDevice(Vulkan *vulkan) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkan->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }
}
#pragma endregion

#pragma region LOGICAL DEVICE
void createLogicalDevice(Vulkan *vulkan) {
    QueueFamilyIndices indices =
        findQueueFamilies(vulkan->physicalDevice, vulkan->surface);

    uint32_t uniqueQueueFamilies[] = {indices.graphicsFamily,
                                      indices.presentFamily};
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
    deviceFeatures.samplerAnisotropy = VK_TRUE;

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

    if (vkCreateDevice(vulkan->physicalDevice, &createInfo, NULL,
                       &vulkan->device) != VK_SUCCESS) {
        printf("failed to create logical device!\n");
        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(vulkan->device, indices.graphicsFamily, 0,
                     &vulkan->graphicsQueue);
    vkGetDeviceQueue(vulkan->device, indices.presentFamily, 0,
                     &vulkan->presentQueue);
}
#pragma endregion

#pragma region SWAP CHAIN
VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats,
                                           uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

void clamp(uint32_t *val, uint32_t min, uint32_t max) {
    if (*val < min) {
        *val = min;
    }
    if (*val > max) {
        *val = max;
    }
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities,
                            SDL_Window *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};

        clamp(&actualExtent.width, capabilities.minImageExtent.width,
              capabilities.maxImageExtent.width);
        clamp(&actualExtent.height, capabilities.minImageExtent.height,
              capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR *availablePresentModes,
                                       uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void createSwapChain(SDL_Window *window, Vulkan *vulkan) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(vulkan->physicalDevice, vulkan->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    vulkan->swapChainImagesCount =
        swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        vulkan->swapChainImagesCount >
            swapChainSupport.capabilities.maxImageCount) {
        vulkan->swapChainImagesCount =
            swapChainSupport.capabilities.maxImageCount;
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

    QueueFamilyIndices indices =
        findQueueFamilies(vulkan->physicalDevice, vulkan->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily,
                                     indices.presentFamily};

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

    if (vkCreateSwapchainKHR(vulkan->device, &createInfo, NULL,
                             &vulkan->swapChain) != VK_SUCCESS) {
        printf("failed to create swap chain!\n");
        exit(EXIT_FAILURE);
    }

    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain,
                            &vulkan->swapChainImagesCount, NULL);

    vulkan->swapChainImages =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->swapChainImages));

    if (vulkan->swapChainImages == NULL) {
        printf("Could not init swap chain images\n");
        exit(EXIT_FAILURE);
    }
    vkGetSwapchainImagesKHR(vulkan->device, vulkan->swapChain,
                            &vulkan->swapChainImagesCount,
                            vulkan->swapChainImages);

    vulkan->swapChainImageFormat = surfaceFormat.format;
    vulkan->swapChainExtent = extent;
}
#pragma endregion

#pragma region RENDER PASS

VkFormat findDepthFormat(Vulkan *);

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

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat(vulkan);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = SIZEOF(attachments);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vulkan->device, &renderPassInfo, NULL,
                           &vulkan->renderPass) != VK_SUCCESS) {
        printf("failed to create render pass!\n");
        exit(EXIT_FAILURE);
    }
}
#pragma endregion

#pragma region GRAPHICS PIPELINE
void createShaderModule(char *code, uint32_t length, VkDevice device,
                        VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = length;
    createInfo.pCode = (uint32_t *)code;

    if (vkCreateShaderModule(device, &createInfo, NULL, shaderModule) !=
        VK_SUCCESS) {
        printf("failed to create shader module!\n");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
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
    createShaderModule(vertShaderCode.buff, vertShaderCode.len, vulkan->device,
                       &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(fragShaderCode.buff, fragShaderCode.len, vulkan->device,
                       &fragShaderModule);

    freeMem(vertShaderCode.buff);
    freeMem(fragShaderCode.buff);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription =
        getBindingDescription();
    VkVertexInputAttributeDescription *attributeDescriptions =
        getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
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
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vulkan->descriptorSetLayout;

    if (vkCreatePipelineLayout(vulkan->device, &pipelineLayoutInfo, NULL,
                               &vulkan->pipelineLayout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!\n");
        exit(EXIT_FAILURE);
    }

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
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
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(vulkan->device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, NULL,
                                  &vulkan->graphicsPipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!\n");
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(vulkan->device, fragShaderModule, NULL);
    vkDestroyShaderModule(vulkan->device, vertShaderModule, NULL);

    freeMem(attributeDescriptions);
}
#pragma endregion

#pragma region FRAME BUFFERS
void createFramebuffers(Vulkan *vulkan) {
    vulkan->swapChainFramebuffers = malloc(
        vulkan->swapChainImagesCount * sizeof(*vulkan->swapChainFramebuffers));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkImageView attachments[] = {vulkan->swapChainImageViews[i],
                                     vulkan->depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkan->renderPass;
        framebufferInfo.attachmentCount = SIZEOF(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkan->swapChainExtent.width;
        framebufferInfo.height = vulkan->swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkan->device, &framebufferInfo, NULL,
                                &vulkan->swapChainFramebuffers[i]) !=
            VK_SUCCESS) {
            printf("failed to create framebuffer!\n");
            exit(EXIT_FAILURE);
        }
    }
}
#pragma endregion

#pragma region COMMAND POOL
void createCommandPool(Vulkan *vulkan) {
    QueueFamilyIndices queueFamilyIndices =
        findQueueFamilies(vulkan->physicalDevice, vulkan->surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(vulkan->device, &poolInfo, NULL,
                            &vulkan->commandPool) != VK_SUCCESS) {
        printf("failed to create command pool!\n");
        exit(EXIT_FAILURE);
    }
}
#pragma endregion

#pragma region COMMAND BUFFER
void createCommandBuffers(Vulkan *vulkan, Window window) {
    vulkan->commandBuffers =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->commandBuffers));

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vulkan->swapChainImagesCount;

    if (vkAllocateCommandBuffers(vulkan->device, &allocInfo,
                                 vulkan->commandBuffers) != VK_SUCCESS) {
        printf("failed to allocate command buffers!\n");
        exit(EXIT_FAILURE);
    }

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = window.width;
    viewport.height = window.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = (VkExtent2D){window.width, window.height};

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(vulkan->commandBuffers[i], &beginInfo) !=
            VK_SUCCESS) {
            printf("failed to begin recording command buffer!\n");
            exit(EXIT_FAILURE);
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkan->renderPass;
        renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
        renderPassInfo.renderArea.extent = vulkan->swapChainExtent;
        renderPassInfo.framebuffer = vulkan->swapChainFramebuffers[i];

        VkClearValue clearValues[] = {{{{0.0f, 0.0f, 0.0f, 1.0f}}},
                                      {{{1.0f, 0}}}};

        renderPassInfo.clearValueCount = SIZEOF(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(vulkan->commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(vulkan->commandBuffers[i], 0, 1, &viewport);

        vkCmdSetScissor(vulkan->commandBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(vulkan->commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vulkan->graphicsPipeline);

        VkBuffer vertexBuffers[] = {vulkan->vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(vulkan->commandBuffers[i], 0, 1, vertexBuffers,
                               offsets);

        vkCmdBindIndexBuffer(vulkan->commandBuffers[i], vulkan->indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(
            vulkan->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
            vulkan->pipelineLayout, 0, 1, &vulkan->descriptorSets[i], 0, NULL);

        vkCmdDrawIndexed(vulkan->commandBuffers[i], SIZEOF(indices), 1, 0, 0,
                         0);

        vkCmdEndRenderPass(vulkan->commandBuffers[i]);

        if (vkEndCommandBuffer(vulkan->commandBuffers[i]) != VK_SUCCESS) {
            printf("failed to record command buffer!\n");
        }
    }
}
#pragma endregion

#pragma region SYNC OBJECTS
void createSyncObjects(Vulkan *vulkan) {
    VkSemaphore *imageAvailableSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*imageAvailableSemaphoresTemp));
    VkSemaphore *renderFinishedSemaphoresTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*renderFinishedSemaphoresTemp));
    VkFence *inFlightFencesTemp =
        malloc(MAX_FRAMES_IN_FLIGHT * sizeof(*inFlightFencesTemp));
    vulkan->imagesInFlight =
        calloc(vulkan->swapChainImagesCount, sizeof(*vulkan->imagesInFlight));

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(vulkan->device, &semaphoreInfo, NULL,
                              &imageAvailableSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkan->device, &semaphoreInfo, NULL,
                              &renderFinishedSemaphoresTemp[i]) != VK_SUCCESS ||
            vkCreateFence(vulkan->device, &fenceInfo, NULL,
                          &inFlightFencesTemp[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!\n");
        }
    }

    vulkan->imageAvailableSemaphores = imageAvailableSemaphoresTemp;
    vulkan->renderFinishedSemaphores = renderFinishedSemaphoresTemp;
    vulkan->inFlightFences = inFlightFencesTemp;
}
#pragma endregion

#pragma region VERTEX / INDEX / UNIFORM BUFFER

void createDescriptorSetLayout(Vulkan *vulkan) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding,
                                               samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = SIZEOF(bindings);
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, NULL,
                                    &vulkan->descriptorSetLayout) !=
        VK_SUCCESS) {
        printf("failed to create descriptor set layout!\n");
        exit(EXIT_FAILURE);
    }
}

uint32_t findMemoryType(Vulkan *vulkan, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkan->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    printf("failed to find suitable memory type!\n");
    exit(EXIT_FAILURE);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, Vulkan *vulkan,
                  VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkan->device, &bufferInfo, NULL, buffer) !=
        VK_SUCCESS) {
        printf("failed to create buffer!\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkan->device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkan, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkan->device, &allocInfo, NULL, bufferMemory) !=
        VK_SUCCESS) {
        printf("failed to allocate buffer memory!\n");
        exit(EXIT_FAILURE);
    }

    vkBindBufferMemory(vulkan->device, *buffer, *bufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(Vulkan *vulkan) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkan->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkan->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(Vulkan *vulkan, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->graphicsQueue);

    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool, 1,
                         &commandBuffer);
}

void copyBuffer(Vulkan *vulkan, VkBuffer srcBuffer, VkBuffer dstBuffer,
                VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createIndexVertexBuffer(Vulkan *vulkan, VkBuffer *buffer,
                             VkDeviceMemory *bufferMemory, VkDeviceSize size) {
    // VkDeviceSize bufferSize = sizeof(vertices[0]) * SIZEOF(vertices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device, stagingBufferMemory, 0, size, 0, &data);
    memcpy(data, vertices, (size_t)size);
    vkUnmapMemory(vulkan->device, stagingBufferMemory);

    createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan, buffer, bufferMemory);

    copyBuffer(vulkan, stagingBuffer, *buffer, size);

    vkDestroyBuffer(vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device, stagingBufferMemory, NULL);
}

void createVertexBuffer(Vulkan *vulkan) {
    VkDeviceSize bufferSize = LENGTH(vertices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(vulkan->device, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                 &vulkan->vertexBuffer, &vulkan->vertexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->vertexBuffer, bufferSize);

    vkDestroyBuffer(vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device, stagingBufferMemory, NULL);
}

void createIndexBuffer(Vulkan *vulkan) {
    VkDeviceSize bufferSize = LENGTH(indices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(vulkan->device, stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                 &vulkan->indexBuffer, &vulkan->indexBufferMemory);

    copyBuffer(vulkan, stagingBuffer, vulkan->indexBuffer, bufferSize);

    vkDestroyBuffer(vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device, stagingBufferMemory, NULL);
}

void createUniformBuffers(Vulkan *vulkan) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    vulkan->uniformBuffers =
        malloc(vulkan->swapChainImagesCount * sizeof(*vulkan->uniformBuffers));
    vulkan->uniformBuffersMemory = malloc(
        vulkan->swapChainImagesCount * sizeof(*vulkan->uniformBuffersMemory));

    for (size_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     vulkan, &vulkan->uniformBuffers[i],
                     &vulkan->uniformBuffersMemory[i]);
    }
}

void createDescriptorSets(Vulkan *vulkan) {
    VkDescriptorSetLayout *layouts =
        malloc(vulkan->swapChainImagesCount * sizeof(*layouts));
    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        layouts[i] = vulkan->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkan->descriptorPool;
    allocInfo.descriptorSetCount = vulkan->swapChainImagesCount;
    allocInfo.pSetLayouts = layouts;

    // descriptorSets.resize(swapChainImages.size());
    vulkan->descriptorSets =
        malloc(vulkan->swapChainImagesCount * sizeof(vulkan->descriptorSets));
    if (vkAllocateDescriptorSets(vulkan->device, &allocInfo,
                                 vulkan->descriptorSets) != VK_SUCCESS) {
        printf("failed to allocate descriptor sets!\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vulkan->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkan->textureImageView;
        imageInfo.sampler = vulkan->textureSampler;

        VkWriteDescriptorSet descriptorWrites[2];

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vulkan->descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pNext = NULL;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vulkan->descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pNext = NULL;

        vkUpdateDescriptorSets(vulkan->device, SIZEOF(descriptorWrites),
                               descriptorWrites, 0, NULL);
    }

    freeMem(layouts);
}

void createDescriptorPool(Vulkan *vulkan) {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = vulkan->swapChainImagesCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = vulkan->swapChainImagesCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = SIZEOF(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = vulkan->swapChainImagesCount;

    if (vkCreateDescriptorPool(vulkan->device, &poolInfo, NULL,
                               &vulkan->descriptorPool) != VK_SUCCESS) {
        printf("failed to create descriptor pool!\n");
        exit(EXIT_FAILURE);
    }
}
#pragma endregion

#pragma region RECREATE SWAP CHAIN
void cleanupSwapChain(Vulkan *vulkan) {
    vkDestroyImageView(vulkan->device, vulkan->depthImageView, NULL);
    vkDestroyImage(vulkan->device, vulkan->depthImage, NULL);
    vkFreeMemory(vulkan->device, vulkan->depthImageMemory, NULL);

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyFramebuffer(vulkan->device, vulkan->swapChainFramebuffers[i],
                             NULL);
    }
    freeMem(vulkan->swapChainFramebuffers);

    vkFreeCommandBuffers(vulkan->device, vulkan->commandPool,
                         vulkan->swapChainImagesCount, vulkan->commandBuffers);

    vkDestroyPipeline(vulkan->device, vulkan->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vulkan->device, vulkan->pipelineLayout, NULL);
    vkDestroyRenderPass(vulkan->device, vulkan->renderPass, NULL);

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyImageView(vulkan->device, vulkan->swapChainImageViews[i],
                           NULL);
    }
    freeMem(vulkan->swapChainImageViews);

    vkDestroySwapchainKHR(vulkan->device, vulkan->swapChain, NULL);

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vkDestroyBuffer(vulkan->device, vulkan->uniformBuffers[i], NULL);
        vkFreeMemory(vulkan->device, vulkan->uniformBuffersMemory[i], NULL);
    }
    freeMem(vulkan->uniformBuffers);
    freeMem(vulkan->uniformBuffersMemory);

    vkDestroyDescriptorPool(vulkan->device, vulkan->descriptorPool, NULL);
}

void createImageViews(Vulkan *);

void createDepthResources(Vulkan *);

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
    createGraphicsPipeline(vulkan);
    createDepthResources(vulkan);
    createFramebuffers(vulkan);
    createUniformBuffers(vulkan);
    createDescriptorPool(vulkan);
    createDescriptorSets(vulkan);
    createCommandBuffers(vulkan, window);

    freeMem(vulkan->imagesInFlight);

    vulkan->imagesInFlight =
        calloc(vulkan->swapChainImagesCount, sizeof(*vulkan->imagesInFlight));
}
#pragma endregion

#pragma region DRAW FRAME
void updateUniformBuffer(Vulkan *vulkan, uint32_t currentImage, float dt) {

    glm_rotate(vulkan->ubo.model, dt * glm_rad(45.0f),
               (vec3){0.0f, 0.0f, 1.0f});

    glm_lookat((vec3){2.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f},
               (vec3){0.0f, 0.0f, 1.0f}, vulkan->ubo.view);

    glm_perspective(glm_rad(45.0f),
                    vulkan->swapChainExtent.width /
                        vulkan->swapChainExtent.height,
                    0.1f, 20.0f, vulkan->ubo.proj);

    vulkan->ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(vulkan->device, vulkan->uniformBuffersMemory[currentImage], 0,
                sizeof(vulkan->ubo), 0, &data);
    memcpy(data, &vulkan->ubo, sizeof(vulkan->ubo));
    vkUnmapMemory(vulkan->device, vulkan->uniformBuffersMemory[currentImage]);
}

void drawFrame(Window window, SDL_Event event, Vulkan *vulkan,
               size_t *currentFrame, float dt) {
    vkWaitForFences(vulkan->device, 1, &vulkan->inFlightFences[*currentFrame],
                    VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result =
        vkAcquireNextImageKHR(vulkan->device, vulkan->swapChain, UINT64_MAX,
                              vulkan->imageAvailableSemaphores[*currentFrame],
                              VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(window, event, vulkan);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image!\n");
        exit(EXIT_FAILURE);
    }

    updateUniformBuffer(vulkan, imageIndex, dt);

    if (vulkan->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vulkan->device, 1, &vulkan->imagesInFlight[imageIndex],
                        VK_TRUE, UINT64_MAX);
    }
    vulkan->imagesInFlight[imageIndex] = vulkan->inFlightFences[*currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        vulkan->imageAvailableSemaphores[*currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkan->commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {
        vulkan->renderFinishedSemaphores[*currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(vulkan->device, 1, &vulkan->inFlightFences[*currentFrame]);

    if (vkQueueSubmit(vulkan->graphicsQueue, 1, &submitInfo,
                      vulkan->inFlightFences[*currentFrame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer!\n");
        exit(EXIT_FAILURE);
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        vulkan->framebufferResized) {
        vulkan->framebufferResized = false;
        recreateSwapChain(window, event, vulkan);
    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image!\n");
        exit(EXIT_FAILURE);
    }

    *currentFrame = (*currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
#pragma endregion

#pragma region TEXTURE MAPPING

void createImage(uint32_t width, uint32_t height, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, Vulkan *vulkan,
                 VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vulkan->device, &imageInfo, NULL, image) != VK_SUCCESS) {
        printf("failed to create image!\n");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkan->device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(vulkan, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkan->device, &allocInfo, NULL, imageMemory) !=
        VK_SUCCESS) {
        printf("failed to allocate image memory!\n");
        exit(EXIT_FAILURE);
    }

    vkBindImageMemory(vulkan->device, *image, *imageMemory, 0);
}

void transitionImageLayout(Vulkan *vulkan, VkImage image,
                           VkFormat format __unused, VkImageLayout oldLayout,
                           VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        printf("unsupported layout transition!\n");
        exit(EXIT_FAILURE);
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         NULL, 0, NULL, 1, &barrier);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void copyBufferToImage(Vulkan *vulkan, VkBuffer buffer, VkImage image,
                       uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkan);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(vulkan, commandBuffer);
}

void createTextureImage(Vulkan *vulkan) {
    SDL_Surface *image =
        // IMG_Load("/Users/rob/Downloads/statue-1275469_640.jpg");
        IMG_Load("/Users/rob/Pictures/Affs/20160312_211430_Original.jpg");

    // convert to desired format
    // image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
    image = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ABGR8888, 0);

    VkDeviceSize imageSize = image->w * image->h * image->format->BytesPerPixel;

    if (!image) {
        SDL_Log("Could not load texture: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vulkan, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vulkan->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, image->pixels, (size_t)imageSize);
    vkUnmapMemory(vulkan->device, stagingBufferMemory);

    createImage(image->w, image->h, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->textureImage, &vulkan->textureImageMemory);

    transitionImageLayout(vulkan, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(vulkan, stagingBuffer, vulkan->textureImage,
                      (uint32_t)image->w, (uint32_t)image->h);
    transitionImageLayout(vulkan, vulkan->textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(vulkan->device, stagingBuffer, NULL);
    vkFreeMemory(vulkan->device, stagingBufferMemory, NULL);

    SDL_FreeSurface(image);
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
                            VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    // viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        printf("failed to create texture image view!\n");
        exit(EXIT_FAILURE);
    }

    return imageView;
}

void createTextureImageView(Vulkan *vulkan) {
    vulkan->textureImageView =
        createImageView(vulkan->device, vulkan->textureImage,
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createImageViews(Vulkan *vulkan) {
    vulkan->swapChainImageViews = malloc(vulkan->swapChainImagesCount *
                                         sizeof(*vulkan->swapChainImageViews));

    for (uint32_t i = 0; i < vulkan->swapChainImagesCount; i++) {
        vulkan->swapChainImageViews[i] = createImageView(
            vulkan->device, vulkan->swapChainImages[i],
            vulkan->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createTextureSampler(Vulkan *vulkan) {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(vulkan->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(vulkan->device, &samplerInfo, NULL,
                        &vulkan->textureSampler) != VK_SUCCESS) {
        printf("failed to create texture sampler!\n");
        exit(EXIT_FAILURE);
    }
}

#pragma endregion

#pragma region DEPTH_BUFFERING

VkFormat findSupportedFormat(const VkFormat *candidates, size_t length,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features,
                             VkPhysicalDevice physicalDevice) {
    // for (VkFormat format : candidates) {
    for (size_t i = 0; i < length; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i],
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }

    printf("Failed to find supported format!\n");
    exit(EXIT_FAILURE);
}

VkFormat findDepthFormat(Vulkan *vulkan) {
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};
    return findSupportedFormat(
        candidates, SIZEOF(candidates), VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, vulkan->physicalDevice);
}

void createDepthResources(Vulkan *vulkan) {
    VkFormat depthFormat = findDepthFormat(vulkan);

    createImage(vulkan->swapChainExtent.width, vulkan->swapChainExtent.height,
                depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkan,
                &vulkan->depthImage, &vulkan->depthImageMemory);

    vulkan->depthImageView =
        createImageView(vulkan->device, vulkan->depthImage, depthFormat,
                        VK_IMAGE_ASPECT_DEPTH_BIT);
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

    createDescriptorSetLayout(vulkan);

    createGraphicsPipeline(vulkan);

    createDepthResources(vulkan);

    createFramebuffers(vulkan);

    createCommandPool(vulkan);

    createTextureImage(vulkan);

    createTextureImageView(vulkan);

    createTextureSampler(vulkan);

    createVertexBuffer(vulkan);

    createIndexBuffer(vulkan);

    createUniformBuffers(vulkan);

    createDescriptorPool(vulkan);

    createDescriptorSets(vulkan);

    createCommandBuffers(vulkan, window);

    createSyncObjects(vulkan);
}

void cleanUpVulkan(Vulkan *vulkan) {
    cleanupSwapChain(vulkan);

    vkDestroySampler(vulkan->device, vulkan->textureSampler, NULL);
    vkDestroyImageView(vulkan->device, vulkan->textureImageView, NULL);

    vkDestroyImage(vulkan->device, vulkan->textureImage, NULL);
    vkFreeMemory(vulkan->device, vulkan->textureImageMemory, NULL);

    freeMem(vulkan->descriptorSets);

    vkDestroyDescriptorSetLayout(vulkan->device, vulkan->descriptorSetLayout,
                                 NULL);

    if (vulkan->graphicsPipeline == VK_NULL_HANDLE) {
        vkDestroyPipeline(vulkan->device, vulkan->graphicsPipeline, NULL);
    }
    if (vulkan->pipelineLayout == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vulkan->device, vulkan->pipelineLayout, NULL);
    }

    vkDestroyBuffer(vulkan->device, vulkan->indexBuffer, NULL);
    vkFreeMemory(vulkan->device, vulkan->indexBufferMemory, NULL);

    vkDestroyBuffer(vulkan->device, vulkan->vertexBuffer, NULL);
    vkFreeMemory(vulkan->device, vulkan->vertexBufferMemory, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vulkan->device, vulkan->renderFinishedSemaphores[i],
                           NULL);
        vkDestroySemaphore(vulkan->device, vulkan->imageAvailableSemaphores[i],
                           NULL);
        vkDestroyFence(vulkan->device, vulkan->inFlightFences[i], NULL);
    }

    freeMem(vulkan->renderFinishedSemaphores);
    freeMem(vulkan->imageAvailableSemaphores);
    freeMem(vulkan->inFlightFences);
    freeMem(vulkan->imagesInFlight);
    freeMem(vulkan->commandBuffers);

    vkDestroyCommandPool(vulkan->device, vulkan->commandPool, NULL);

    vkDestroyDevice(vulkan->device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(vulkan->instance, vulkan->debugMessenger,
                                      NULL);
    }

    vkDestroySurfaceKHR(vulkan->instance, vulkan->surface, NULL);
    vkDestroyInstance(vulkan->instance, NULL);
}
#pragma endregion

// float approxRollingAverage(float avg, float new_avg) {
//     return (avg * (5 - 1) + new_avg) / 5;
// }

int main(void) {
    // Init SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    if (IMG_Init(IMG_INIT_JPG) != IMG_INIT_JPG) {
        SDL_Log("Unable to initialize SDL_Image: %s", SDL_GetError());
        return -1;
    }

    // struct Image m = loadImage("/Users/rob/Pictures/160366.jpg");
    // printf("%d %d\n", m.h, m.w);
    // free(m.texture);

    Window window = {.width = WIDTH_INIT, .height = HEIGHT_INIT};
    window.win = SDL_CreateWindow(
        APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window.width,
        window.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Add Window resize callback
    SDL_AddEventWatch(resizingEventCallback, &window);

    // INIT VULKAN
    Vulkan vulkan;
    initVulkan(window, &vulkan);
    glm_mat4_identity(vulkan.ubo.model);

    window.framebufferResized = &vulkan.framebufferResized;

    size_t currentFrame = 0;

    // MAIN LOOP
    SDL_Event event;
    bool running = true;

    uint64_t start = SDL_GetPerformanceCounter();
    float dt;

    while (running) {
        // Delta Time
        uint64_t last = start;
        start = SDL_GetPerformanceCounter();
        dt = (((start - last) * 1000) / SDL_GetPerformanceFrequency()) * 0.001;

        // Handle Events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        // Update

        // Drawing
        drawFrame(window, event, &vulkan, &currentFrame, dt);

        SDL_Delay(floor(FRAME_DELAY - dt));
    }

    vkDeviceWaitIdle(vulkan.device);

    // CLEAN UP
    cleanUpVulkan(&vulkan);

    SDL_DestroyWindow(window.win);
    window.win = NULL;

    IMG_Quit();

    SDL_Quit();

    return 0;
}
