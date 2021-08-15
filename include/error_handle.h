#ifndef INCLUDE_ERROR_HANDLE
#define INCLUDE_ERROR_HANDLE

#define THROW_ERROR(errorMsg)                                                  \
    printf("%s\n", errorMsg);                                                  \
    exit(EXIT_FAILURE);

#endif /* INCLUDE_ERROR_HANDLE */
