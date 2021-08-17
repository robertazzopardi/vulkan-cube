#include "utility/file_handle.h"
#include "stdio.h"
#include "stdlib.h"

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
