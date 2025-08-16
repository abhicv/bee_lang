#ifndef ERROR_H
#define ERROR_H

#include "file.h"

typedef struct Position {
    unsigned int lineNumber;
    unsigned int column;
    unsigned int index; // position in source code character array
} Position;

typedef struct Location {
    Position start;
    Position end;
} Location;

void PrintErrorLocationInSource(LoadedFile loadedFile, unsigned int location, unsigned int lineNumber, unsigned int column, char *errorMsg);
#endif