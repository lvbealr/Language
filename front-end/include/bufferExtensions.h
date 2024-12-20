#ifndef BUFFER_EXTENSIONS_H_
#define BUFFER_EXTENSIONS_H_

#include <cstdio>
#include <cstdlib>

#include "bufferDefinitions.h"
#include "customWarning.h"

// FUNCTION PROTOTYPES //
template <> inline int         countLines<char>          (Buffer<char> *text);
            inline bufferError getLinePointerFromFile    (Buffer<char *>  *bufferStruct,  Buffer<char> *text);
template <> inline bufferError bufferDestruct<char *>    (Buffer<char *>  *bufferStruct);
// FUNCTION PROTOTYPES //

template <>
inline bufferError scanFileToBuffer<char>(Buffer<char> *bufferStruct, const char *filename) {
    customWarning(bufferStruct,      POINTER_IS_NULL);
    customWarning(filename,          POINTER_IS_NULL);
    customWarning(bufferStruct != 0, FILE_READ_ERROR);

    FILE *file = fopen(filename, "r");
    customWarning(file, FILE_OPEN_ERROR);

    fread(bufferStruct->data, sizeof(char), (size_t) bufferStruct->size, file);

    fclose(file);

    return NO_ERRORS;
}

template <>
inline int countLines<char>(Buffer<char> *text) {
    customWarning(text, POINTER_IS_NULL);

    int linesNumber = 0;
    for(int index = 0; index < text->size; index++) {
        if(text->data[index] == '\n') {
            linesNumber++;
        }
    }

    return linesNumber;
}

template <>
inline bufferError bufferDestruct<char>(Buffer<char> *bufferStruct) {
    customWarning(bufferStruct, POINTER_IS_NULL);

    bufferStruct->size = 0;
    free(bufferStruct->data);
    bufferStruct->data = NULL;

    return NO_ERRORS;
}

template <>
inline bufferError bufferDestruct<char *>(Buffer<char *> *bufferStruct) {
    customWarning(bufferStruct, POINTER_IS_NULL);

    bufferStruct->size = 0;
    free(bufferStruct->data);
    bufferStruct->data = NULL;

    return NO_ERRORS;
}

#endif // BUFFER_EXTENSIONS_H_
