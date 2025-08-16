#include "file.h"

LoadedFile LoadFileNullTerminated(const char *fileName)
{
    LoadedFile loadedFile = {0};
    loadedFile.isLoaded = false;

    String path = {0};
    path.length = strlen(fileName);
    path.data = (char*)malloc(path.length + 1);
    strncpy(path.data, fileName, path.length);
    path.data[path.length] = 0;

    loadedFile.path = path;

    FILE *file = fopen(fileName, "r");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        unsigned int size = ftell(file);
        fseek(file, 0, SEEK_SET);

        String source = {0};
        source.data = (char*)malloc(size + 1);
        fread(source.data, sizeof(char), size, file);
        source.data[size] = 0;
        source.length = size;
        
        fclose(file);

        loadedFile.source = source;
        loadedFile.isLoaded = true;
    }
    else
    {
        printf("error: failed to open input file '%s'\n", fileName);
    }
    
    return loadedFile;
}
