#include "treeSaver.h"
#include <stdlib.h>
#include "binaryTreeDef.h"

char *getNameTableFileName() {
    char *fileName = (char *)calloc(MAX_FILE_NAME_SIZE, sizeof(char));
    customWarning(fileName, NULL);

    const time_t currentTime = time(NULL);
    tm localTime             = *localtime(&currentTime);

    snprintf(fileName, MAX_FILE_NAME_SIZE, "nameTable/%.2d.%.2d.%.4d-%.2d:%.2d:%.2d.nameTable", 
                localTime.tm_mday, localTime.tm_mon, localTime.tm_year + 1900,
                localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    return fileName;
}

saveDataError initializeSaveDataContext(saveDataContext *saveContext, char *fileName) {
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);
    customWarning(fileName,    saveDataError::BAD_FILENAME);

    saveContext->buffer = (Buffer<char> *)calloc(1, sizeof(Buffer<char>));
    customWarning(saveContext->buffer, saveDataError::ALLOCATION_ERROR);

    if (bufferInitialize(saveContext->buffer) != bufferError::NO_BUFFER_ERROR) {
        return saveDataError::INITIALIZATION_ERROR;
    }

    saveContext->fileName = fileName;

    return saveDataError::NO_ERRORS;
}

saveDataError destroySaveDataContext(saveDataContext *saveContext) {
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    if (saveContext->buffer) {
        bufferDestruct(saveContext->buffer);
    }

    if (saveContext->fileName) {
        FREE_(saveContext->fileName);
    }

    return saveDataError::NO_ERRORS;
}

static size_t getKeywordsCount(compilationContext *context) {
    customWarning(context, 0);

    size_t keywordsCount = 0;

    while (keywordsCount < context->nameTable->currentIndex && context->nameTable->data[keywordsCount].type != nameType::IDENTIFIER) {
        keywordsCount++;
    }

    return keywordsCount;
}

saveDataError saveNameTable(compilationContext *context, saveDataContext *saveContext) {
    customWarning(context,    saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    initializeSaveDataContext(saveContext, getNameTableFileName());

    size_t keywordsCount = getKeywordsCount(context);

    saveGlobalNameTable(context, saveContext, keywordsCount);

    char numToStr[16 + 1] = {};
    snprintf(numToStr, sizeof(numToStr), "\n%lu\n", context->localTables->currentIndex); 

    writeStringToBuffer(saveContext->buffer, numToStr);

    for (size_t localTableIndex = 0; localTableIndex < context->localTables->currentIndex; localTableIndex++) {
        saveLocalNameTable(context, saveContext, localTableIndex, keywordsCount);
    }

    FILE *file = fopen(saveContext->fileName, "w");
    customWarning(file, saveDataError::FILE_OPEN_ERROR);

    if (fwrite(saveContext->buffer->data, sizeof(char), saveContext->buffer->currentIndex, file) != saveContext->buffer->currentIndex) {
        fclose(file);
        return saveDataError::FILE_WRITE_ERROR;
    }

    fclose(file);

    return saveDataError::NO_ERRORS;
}

saveDataError saveGlobalNameTable(compilationContext *context, saveDataContext *saveContext, size_t keywordsCount) {
    customWarning(context,   saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    char numToStr[16 + 1] = {};

    snprintf(numToStr, sizeof(numToStr), "%lu\n", context->nameTable->currentIndex - keywordsCount); 

    writeStringToBuffer(saveContext->buffer, numToStr);

    for (size_t index = keywordsCount; index < context->nameTable->currentIndex; index++) {
        writeStringToBuffer(saveContext->buffer, context->nameTable->data[index].name);
        writeStringToBuffer(saveContext->buffer, "\n");
    }

    return saveDataError::NO_ERRORS;
}

saveDataError saveLocalNameTable(compilationContext *context, saveDataContext *saveContext, size_t localTableIndex, size_t keywordsCount) {
    customWarning(context,    saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    char numToStr[32 + 1] = {};
    snprintf(numToStr, sizeof(numToStr), "\n%lu ", context->localTables->data[localTableIndex].size);

    writeStringToBuffer(saveContext->buffer, numToStr);

    int nameTableID = context->localTables->data[localTableIndex].nameTableID;
    nameTableID = nameTableID - (nameTableID > 0 ? (int) keywordsCount : 0);

    snprintf(numToStr, sizeof(numToStr), "%d\n", nameTableID);
    writeStringToBuffer(saveContext->buffer, numToStr);

    for (size_t elementIndex = 0; elementIndex < context->localTables->data[localTableIndex].size; elementIndex++) {
        localNameTableElement *element = &context->localTables->data[localTableIndex].elements.data[elementIndex];

        snprintf(numToStr, sizeof(numToStr), "%lu %d\n", element->globalNameID - keywordsCount, static_cast<int>(element->type));
        writeStringToBuffer(saveContext->buffer, numToStr);
    }

    return saveDataError::NO_ERRORS;
}