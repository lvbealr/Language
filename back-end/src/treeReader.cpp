#include "treeReader.h"
#include "core.h"

translationError readAST(translationContext *context, const char *fileName) {
    customWarning(context,  translationError::CONTEXT_BAD_POINTER);
    customWarning(fileName, translationError::BAD_FILE_NAME);

    // TODO

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError readNameTable(translationContext *context, const char *fileName) {
    customWarning(context,  translationError::CONTEXT_BAD_POINTER);
    customWarning(fileName, translationError::BAD_FILE_NAME);

    Buffer<char> fileContent = {};
    customWarning(writeFileDataToBuffer(&fileContent, fileName) == bufferError::NO_BUFFER_ERROR, 
                  translationError::FILE_READING_ERROR);
    
    size_t currentFilePosition = 0;

    readGlobalNameTable(context, &fileContent, &currentFilePosition);

    size_t localTablesCount       = 0;
    int    localTablesCountLength = 0;

    sscanf(fileContent.data + currentFilePosition, "%lu%n[^\n]", &localTablesCount, &localTablesCountLength);
    currentFilePosition += localTablesCountLength;

    for (size_t localTableIndex = 0; localTableIndex < localTablesCount; localTableIndex++) {
        addLocalNameTable(0, context->localTables);
        readLocalNameTable(context, localTableIndex, &fileContent, &currentFilePosition);
    }

    return translationError::NO_ERRORS;
}

translationError readGlobalNameTable(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition) {
    customWarning(context,             translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent,         translationError::BAD_BUFFER_POINTER);

    size_t globalNameTableSize       = 0;
    int    globalNameTableSizeLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%lu%n[^\n]", &globalNameTableSize, &globalNameTableSizeLength);
    (*currentFilePosition) += globalNameTableSizeLength;

    initializeNameTable(context->nameTable, false);

    for (size_t globalNameTableIndex = 0; globalNameTableIndex < globalNameTableSize; globalNameTableIndex++) {
        char *identifier       = NULL;
        int   identifierLength = 0;

        sscanf(fileContent->data + (*currentFilePosition), "%ms%n[^\n]", &identifier, &identifierLength);
        (*currentFilePosition) += identifierLength;

        addIdentifier(context->nameTable, identifier);
    }

    return translationError::NO_ERRORS;
}

translationError readLocalNameTable(translationContext *context, size_t localTableIndex, Buffer<char> *fileContent, size_t *currentFilePosition) {
    customWarning(context,             translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent,         translationError::BAD_BUFFER_POINTER);;

    size_t localNameTableSize       = 0;
    int    localNameTableID         = 0;
    int    localNameTableSizeLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%lu %d%n[^\n]", &localNameTableSize, &localNameTableID, &localNameTableSizeLength);
    (*currentFilePosition) += localNameTableSizeLength;

    context->localTables->data[localTableIndex].nameTableID = localNameTableID;

    for (size_t elementIndex = 0; elementIndex < localNameTableSize; elementIndex++) {
        size_t globalNameTableElementID = 0;
        size_t elementType              = 0;
        int    lineLength               = 0;

        sscanf(fileContent->data + (*currentFilePosition), "%lu %lu%n[^\n]", &globalNameTableElementID, &elementType, &lineLength);
        (*currentFilePosition) += lineLength;

        addLocalIdentifier(localTableIndex, context->localTables, 
            localNameTableElement {.type = static_cast<localNameType>(elementType), .globalNameID = globalNameTableElementID}, 1);
    }

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //