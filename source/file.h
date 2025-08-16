#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct String {
    char *data;
    unsigned int length;    
} String;

typedef struct LoadedFile {
    String source;
    String path;
    bool isLoaded;
} LoadedFile;

#endif