#include "error.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void PrintErrorLocationInSource(LoadedFile loadedFile, unsigned int location, unsigned int lineNumber, unsigned int column, char *errorMsg)
{
    printf("%s:%u:%u: error: %s\n", loadedFile.path.data, lineNumber, column, errorMsg);

    if (location >= loadedFile.source.length) return;

    char *source = loadedFile.source.data;

    int leftPos = location;
    int rightPos = location;

    while(source[leftPos] != '\n' && leftPos >= 0) leftPos--;
    while(source[rightPos] != '\n' && rightPos < loadedFile.source.length) rightPos++;

    leftPos++;
    rightPos--;

    // printf("location: %d, left: %d, right: %d\n", location, leftPos, rightPos);
    // printf("location: '%c'(%d), left: '%c'(%d), right: '%c'(%d)\n", source[location], source[location], source[leftPos], source[leftPos], source[rightPos], source[rightPos]);
    // printf("line size: %u\n", rightPos - leftPos + 1);

    int lineSize = rightPos - leftPos + 1;
    char *line = (char*)malloc(lineSize + 1);
    strncpy(line, source + leftPos, lineSize);
    line[lineSize] = 0;

    printf(ANSI_COLOR_GREEN);
    printf("\t %s\n", line);
    printf(ANSI_COLOR_RESET);
    printf("\t ");
    printf(ANSI_COLOR_RED);
    for(int n = 0; n < location - leftPos; n++) printf(" ");
    printf("%c\n", '^');
    printf("\t ");
    for(int n = 0; n < location - leftPos; n++) printf(" ");
    printf("|\n");
    printf("\t ");
    for(int n = 0; n < location - leftPos; n++) printf("-");
    printf("\n");
    printf(ANSI_COLOR_RESET);
    free(line);
}
