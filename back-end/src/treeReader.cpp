#include "treeReader.h"
#include "colorPrint.h"
#include "core.h"
#include <ctype.h>

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define SKIP_SPACES() do {                                      \
    while (isspace(fileContent->data[*currentFilePosition])) {  \
        (*currentFilePosition)++;                               \
    }                                                           \
} while (0)

#define CHECK_SYMBOL(symbol, ...) do {                          \
    if (fileContent->data[*currentFilePosition] != symbol) {    \
        (*currentFilePosition)++;                               \
        __VA_ARGS__;                                            \
    }                                                           \
} while (0)


// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static translationError readNone(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition, node<astNode> *node) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent, translationError::BUFFER_BAD_POINTER);

    return translationError::NO_ERRORS;
}

static translationError readConstant(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition, node<astNode> *node) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent, translationError::BUFFER_BAD_POINTER);

    int number = 0;
    int length = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%d%n", &number, &length);
    (*currentFilePosition) += length;

    node->data.data.number = number;

    return translationError::NO_ERRORS;
}

static translationError readKeyword(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition, node<astNode> *node) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent, translationError::BUFFER_BAD_POINTER);

    int keywordID     = 0;
    int keywordLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%d%n", &keywordID, &keywordLength);
    (*currentFilePosition) += keywordLength;
    
    node->data.data.keyword = static_cast<Keyword>(keywordID);

    return translationError::NO_ERRORS;
}

static translationError readIdentifierID(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition, node<astNode> *node) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent, translationError::BUFFER_BAD_POINTER);

    int identifierLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%lu%n", &node->data.data.nameTableIndex, &identifierLength);
    (*currentFilePosition) += identifierLength;

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

typedef translationError (*readElement)(translationContext *, Buffer<char> *, size_t *, node<astNode> *);
static const readElement READ_ELEMENT[] = {readConstant, 
                                           readIdentifierID, 
                                           readKeyword, 
                                           readIdentifierID, 
                                           readNone, 
                                           readIdentifierID, 
                                           readNone};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError readAST(translationContext *context, const char *fileName) {
    customWarning(context,  translationError::CONTEXT_BAD_POINTER);
    customWarning(fileName, translationError::BAD_FILE_NAME);

    Buffer<char> fileContent = {};
    customWarning(writeFileDataToBuffer(&fileContent, fileName) == bufferError::NO_BUFFER_ERROR, 
                  translationError::FILE_READING_ERROR);

    size_t currentFilePosition = 0;

    context->AST->root = readASTInternal(context, &fileContent, &currentFilePosition);

    customWarning(context->AST->root, translationError::BAD_FILE_CONTENT);

    return translationError::NO_ERRORS;
}

node<astNode> *readASTInternal(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition) {
    if (!context || !fileContent) {
        return NULL;
    }

    SKIP_SPACES();

    CHECK_SYMBOL('(', return NULL);

    (*currentFilePosition)++;
    SKIP_SPACES();

    int nodeTypeID = -1;
    sscanf(fileContent->data + (*currentFilePosition), "%d", &nodeTypeID);

    if (nodeTypeID < 1 || nodeTypeID > 7) {
        return NULL;
    }

    (*currentFilePosition)++;
    SKIP_SPACES();

    node<astNode> *node = NULL;
    nodeInitialize(&node);

    node->data.type = static_cast<nodeType>(nodeTypeID);

    READ_ELEMENT[nodeTypeID - 1](context, fileContent, currentFilePosition, node);

    node->left  = readASTInternal(context, fileContent, currentFilePosition);
    node->right = readASTInternal(context, fileContent, currentFilePosition);

    if (node->left) {
        node->left->parent = node;
    }

    if (node->right) {
        node->right->parent = node;
    }

    SKIP_SPACES();
    (*currentFilePosition)++;

    return node;
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

    sscanf(fileContent.data + currentFilePosition, "%lu%n", &localTablesCount, &localTablesCountLength);
    currentFilePosition += localTablesCountLength;

    for (size_t localTableIndex = 0; localTableIndex < localTablesCount; localTableIndex++) {
        addLocalNameTable(localTableIndex, context->localTables);
        readLocalNameTable(context, localTableIndex, &fileContent, &currentFilePosition);
    }

    return translationError::NO_ERRORS;
}

translationError readGlobalNameTable(translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition) {
    customWarning(context,             translationError::CONTEXT_BAD_POINTER);
    customWarning(fileContent,         translationError::BUFFER_BAD_POINTER);

    size_t globalNameTableSize       = 0;
    int    globalNameTableSizeLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%lu%n", &globalNameTableSize, &globalNameTableSizeLength);
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
    customWarning(fileContent,         translationError::BUFFER_BAD_POINTER);;

    size_t localNameTableSize       = 0;
    int    localNameTableID         = 0;
    int    localNameTableSizeLength = 0;

    sscanf(fileContent->data + (*currentFilePosition), "%lu %d%n", &localNameTableSize, &localNameTableID, &localNameTableSizeLength);
    (*currentFilePosition) += localNameTableSizeLength;

    context->localTables->data[localTableIndex].nameTableID = localNameTableID;

    for (size_t elementIndex = 0; elementIndex < localNameTableSize; elementIndex++) {
        size_t globalNameTableElementID = 0;
        size_t elementType              = 0;
        int    lineLength               = 0;

        sscanf(fileContent->data + (*currentFilePosition), "%lu %lu%n", &globalNameTableElementID, &elementType, &lineLength);
        (*currentFilePosition) += lineLength;

        addLocalIdentifier(localTableIndex, context->localTables, 
            localNameTableElement {.type = static_cast<localNameType>(elementType), .globalNameID = globalNameTableElementID}, 1);
    }

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //