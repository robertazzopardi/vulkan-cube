#include <vulkan_handle/vulkan_handle.h>

int main(void) {

    Vulkan vulkan = initialise();

    mainLoop(&vulkan);

    terminate(&vulkan);

    return EXIT_SUCCESS;
}
