#ifndef INCLUDE_UTILITY_FILE_HANDLE
#define INCLUDE_UTILITY_FILE_HANDLE

#include <stdint.h>

typedef struct {
    char *buff;
    uint32_t len;
} FileData;

FileData readFile(const char *);

#endif /* INCLUDE_UTILITY_FILE_HANDLE */
