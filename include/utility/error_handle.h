#ifndef INCLUDE_UTILITY_ERROR_HANDLE
#define INCLUDE_UTILITY_ERROR_HANDLE

#include <stdio.h>
#include <stdlib.h>

#define THROW_ERROR(errorMsg)                                                  \
    printf("%s\n", errorMsg);                                                  \
    exit(EXIT_FAILURE);

// #define WHERE fprintf(stderr, "[LOG]%s:%d\n", __FILE__, __LINE__);

#endif /* INCLUDE_UTILITY_ERROR_HANDLE */
