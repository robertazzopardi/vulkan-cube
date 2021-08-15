#include "vulkan_handle/memory.h"
#include <stdarg.h>
#include <stdlib.h>

void freeMem(const size_t count, ...) {
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
