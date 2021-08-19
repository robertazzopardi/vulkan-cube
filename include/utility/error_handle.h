#ifndef INCLUDE_UTILITY_ERROR_HANDLE
#define INCLUDE_UTILITY_ERROR_HANDLE

#include <stdio.h>
#include <stdlib.h>

#define THROW_ERROR(errorMsg)                                                  \
    printf("%s\n", errorMsg);                                                  \
    exit(EXIT_FAILURE);

#endif /* INCLUDE_UTILITY_ERROR_HANDLE */
