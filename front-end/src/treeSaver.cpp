#include "treeSaver.h"
#include <stdlib.h>
#include "binaryTreeDef.h"
#include "buffer.h"
#include "nameTable.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

char *getFileName() {
    char *fileName = (char *)calloc(MAX_FILE_NAME_SIZE, sizeof(char));
    customWarning(fileName, NULL);

    const time_t currentTime = time(NULL);
    tm localTime             = *localtime(&currentTime);

    snprintf(fileName, MAX_FILE_NAME_SIZE, "tree/%.2d.%.2d.%.4d-%.2d:%.2d:%.2d", 
                localTime.tm_mday, localTime.tm_mon, localTime.tm_year + 1900,
                localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    return fileName;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

saveDataError initializeSaveDataContext(saveDataContext *saveContext, char *NTFileName, char *ASTFileName) {
    customWarning(saveContext,   saveDataError::SAVE_CONTEXT_BAD_POINTER);
    customWarning(NTFileName,    saveDataError::BAD_FILENAME);
    customWarning(ASTFileName,   saveDataError::BAD_FILENAME);

    saveContext->NTBuffer = (Buffer<char> *)calloc(1, sizeof(Buffer<char>));
    customWarning(saveContext->NTBuffer, saveDataError::ALLOCATION_ERROR);

    if (bufferInitialize(saveContext->NTBuffer) != bufferError::NO_BUFFER_ERROR) {
        return saveDataError::INITIALIZATION_ERROR;
    }

    saveContext->NTFileName = NTFileName;
    strcat(saveContext->NTFileName, ".nameTable");

    saveContext->ASTBuffer = (Buffer<char> *)calloc(1, sizeof(Buffer<char>));
    customWarning(saveContext->ASTBuffer, saveDataError::ALLOCATION_ERROR);

    if (bufferInitialize(saveContext->ASTBuffer) != bufferError::NO_BUFFER_ERROR) {
        return saveDataError::INITIALIZATION_ERROR;
    }

    saveContext->ASTFileName = ASTFileName;
    strcat(saveContext->ASTFileName, ".AST");

    return saveDataError::NO_ERRORS;
}

saveDataError destroySaveDataContext(saveDataContext *saveContext) {
    customWarning(saveContext,              saveDataError::SAVE_CONTEXT_BAD_POINTER);
    customWarning(saveContext->NTBuffer,    saveDataError::BUFFER_BAD_POINTER);
    customWarning(saveContext->ASTBuffer,   saveDataError::BUFFER_BAD_POINTER);
    customWarning(saveContext->NTFileName,  saveDataError::BAD_FILENAME);
    customWarning(saveContext->ASTFileName, saveDataError::BAD_FILENAME);

    FREE_(saveContext->NTFileName);
    FREE_(saveContext->ASTFileName);

    bufferDestruct(saveContext->NTBuffer);
    bufferDestruct(saveContext->ASTBuffer);

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

    size_t keywordsCount = getKeywordsCount(context);

    saveGlobalNameTable(context, saveContext, keywordsCount);

    char numToStr[16 + 1] = {};
    snprintf(numToStr, sizeof(numToStr), "\n%lu\n", context->localTables->currentIndex); 

    writeStringToBuffer(saveContext->NTBuffer, numToStr);

    for (size_t localTableIndex = 0; localTableIndex < context->localTables->currentIndex; localTableIndex++) {
        saveLocalNameTable(context, saveContext, localTableIndex, keywordsCount);
    }

    FILE *file = fopen(saveContext->NTFileName, "w");
    customWarning(file, saveDataError::FILE_OPEN_ERROR);

    if (fwrite(saveContext->NTBuffer->data, sizeof(char), saveContext->NTBuffer->currentIndex, file) != saveContext->NTBuffer->currentIndex) {
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

    writeStringToBuffer(saveContext->NTBuffer, numToStr);

    for (size_t index = keywordsCount; index < context->nameTable->currentIndex; index++) {
        writeStringToBuffer(saveContext->NTBuffer, context->nameTable->data[index].name);
        writeStringToBuffer(saveContext->NTBuffer, "\n");
    }

    return saveDataError::NO_ERRORS;
}

saveDataError saveLocalNameTable(compilationContext *context, saveDataContext *saveContext, size_t localTableIndex, size_t keywordsCount) {
    customWarning(context,    saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    char numToStr[32 + 1] = {};
    snprintf(numToStr, sizeof(numToStr), "\n%lu ", context->localTables->data[localTableIndex].size);

    writeStringToBuffer(saveContext->NTBuffer, numToStr);

    int nameTableID = context->localTables->data[localTableIndex].nameTableID;
    nameTableID = nameTableID - (nameTableID > 0 ? (int) keywordsCount : 0);

    snprintf(numToStr, sizeof(numToStr), "%d\n", nameTableID);
    writeStringToBuffer(saveContext->NTBuffer, numToStr);

    for (size_t elementIndex = 0; elementIndex < context->localTables->data[localTableIndex].size; elementIndex++) {
        localNameTableElement *element = &context->localTables->data[localTableIndex].elements.data[elementIndex];

        snprintf(numToStr, sizeof(numToStr), "%lu %d\n", element->globalNameID - keywordsCount, static_cast<int>(element->type));
        writeStringToBuffer(saveContext->NTBuffer, numToStr);
    }

    return saveDataError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

saveDataError saveASTTree(compilationContext *context, saveDataContext *saveContext) {
    customWarning(context,     saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    size_t keywordsCount = getKeywordsCount(context);

    saveASTSubtree(context, saveContext, context->AST->root, keywordsCount);

    writeStringToBuffer(saveContext->ASTBuffer, "\n");

    FILE *file = fopen(saveContext->ASTFileName, "w");
    customWarning(file, saveDataError::FILE_OPEN_ERROR);

    fwrite(saveContext->ASTBuffer->data, sizeof(char), saveContext->ASTBuffer->currentIndex, file);
    fclose(file);

    return saveDataError::NO_ERRORS;
}

saveDataError saveASTSubtree(compilationContext *context, saveDataContext *saveContext, node<astNode> *node, size_t keywordsCount) {
    customWarning(context,     saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);

    if (!node) {
        writeStringToBuffer(saveContext->ASTBuffer, "_ ");
        return saveDataError::NO_ERRORS;
    }

    writeStringToBuffer(saveContext->ASTBuffer, "( ");

    saveASTNode(context, saveContext, node, keywordsCount);

    saveASTSubtree(context, saveContext, node->left, keywordsCount);
    saveASTSubtree(context, saveContext, node->right, keywordsCount);

    writeStringToBuffer(saveContext->ASTBuffer, ") ");

    return saveDataError::NO_ERRORS;
}

saveDataError saveASTNode(compilationContext *context, saveDataContext *saveContext, node<astNode> *node, size_t keywordsCount) {
    customWarning(context,     saveDataError::CONTEXT_BAD_POINTER);
    customWarning(saveContext, saveDataError::SAVE_CONTEXT_BAD_POINTER);
    customWarning(node,        saveDataError::NODE_BAD_POINTER);

    char numStr[16 + 1] = {};

    if (node->data.type == nodeType::STRING) {
        if (context->nameTable->data[node->data.data.nameTableIndex].keyword == Keyword::UNDEFINED) {
            snprintf(numStr, sizeof(numStr), "%d ", static_cast<int>(context->nameTable->data[node->data.data.nameTableIndex].keyword));
            writeStringToBuffer(saveContext->ASTBuffer, numStr);

            snprintf(numStr, sizeof(numStr), "%lu ", node->data.data.nameTableIndex - keywordsCount);
            writeStringToBuffer(saveContext->ASTBuffer, numStr);
        } else {
            snprintf(numStr, sizeof(numStr), "%d ", static_cast<int>(node->data.type) + 1);
            writeStringToBuffer(saveContext->ASTBuffer, numStr);
            snprintf(numStr, sizeof(numStr), "%d ", static_cast<int>(context->nameTable->data[node->data.data.nameTableIndex].keyword));
            writeStringToBuffer(saveContext->ASTBuffer, numStr);
        }

        return saveDataError::NO_ERRORS;
    } else {
        snprintf(numStr, sizeof(numStr), "%d ", static_cast<int>(node->data.type));
        writeStringToBuffer(saveContext->ASTBuffer, numStr);
    }

    if (node->data.type == nodeType::CONSTANT) {
        snprintf(numStr, sizeof(numStr), "%d ", node->data.data.number);
        writeStringToBuffer(saveContext->ASTBuffer, numStr);
    } else if (node->data.type == nodeType::FUNCTION_DEFINITION || node->data.type == nodeType::VARIABLE_DECLARATION) {
        snprintf(numStr, sizeof(numStr), "%lu ", node->data.data.nameTableIndex - keywordsCount);
        writeStringToBuffer(saveContext->ASTBuffer, numStr);
    }

    return saveDataError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //