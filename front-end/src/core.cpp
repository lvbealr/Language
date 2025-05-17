#include <cstring>

#include "core.h"
#include "buffer.h"
#include "nameTable.h"
#include "AST.h"
#include "binaryTreeDef.h"

static const char *nameTypeToString(nameType type);

static const bool GLOBAL = true;

compilationError compilationContextInitialize(compilationContext *context, char *fileContent) {
    customWarning(context,     compilationError::CONTEXT_ERROR);
    customWarning(fileContent, compilationError::CONTEXT_ERROR);

    if (bufferInitialize(context->localTables) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::CONTEXT_ERROR;
    }

    addLocalNameTable(-1, context->localTables);

    if (initializeNameTable(context->nameTable, GLOBAL) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::CONTEXT_ERROR;
    }

    if (bufferInitialize(context->tokens) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::TOKEN_BUFFER_ERROR;
    }

    if (bufferInitialize(context->errorBuffer) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::CONTEXT_ERROR;
    }

    if (bufferInitialize(context->functionCalls) != bufferError::NO_BUFFER_ERROR) {
        return compilationError::CONTEXT_ERROR;
    }

    context->error = compilationError::NO_ERRORS;

    context->fileContent = fileContent;
    context->fileSize = strlen(fileContent);
    context->currentLine = 1;

    return compilationError::NO_ERRORS;
}

compilationError compilationContextDestruct(compilationContext *context) {
    customWarning(context, compilationError::CONTEXT_ERROR);

    if (context->AST->root) {
        nodeDestruct(context->AST, &context->AST->root);
    } else {
        for (size_t tokenIndex = 0; tokenIndex < context->tokens->currentIndex; tokenIndex++) {
            FREE_(context->tokens->data[tokenIndex]);
        }
    }

    for (size_t nameIndex = 0; nameIndex < context->nameTable->currentIndex; nameIndex++) {
        if (context->nameTable->data[nameIndex].type == nameType::IDENTIFIER) {
            FREE_(context->nameTable->data[nameIndex].name)
        }
    }

    for (size_t localTableIndex = 0; localTableIndex < context->localTables->currentIndex; localTableIndex++) {
        bufferDestruct(&context->localTables->data[localTableIndex].elements);
    }

    bufferDestruct(context->localTables);
    bufferDestruct(context->nameTable);
    bufferDestruct(context->errorBuffer);
    bufferDestruct(context->tokens);
    bufferDestruct(context->functionCalls);

    FREE_(context->fileContent);

    return compilationError::NO_ERRORS;
}

compilationError dumpTokenTable(compilationContext *context) {
    customWarning(context, compilationError::CONTEXT_ERROR);

    for (size_t tokenIndex = 0; tokenIndex < context->tokens->currentIndex; tokenIndex++) {
        dumpToken(context, context->tokens->data[tokenIndex]);
    }

    return compilationError::NO_ERRORS;
}

compilationError dumpToken(compilationContext *context, node<astNode> *token) {
    customWarning(context, compilationError::CONTEXT_ERROR);
    customWarning(token,   compilationError::CONTEXT_ERROR);

    if (token->data.type == nodeType::CONSTANT) {
        customPrint(green, bold, bgDefault, "Constant: %d\n", token->data.data.number);
    } else if (token->data.type == nodeType::STRING) {
        customPrint(blue, bold, bgDefault, "Name: (type: \"%-10s\") <%s>\n",
                    nameTypeToString(context->nameTable->data[token->data.data.nameTableIndex].type),
                    context->nameTable->data[token->data.data.nameTableIndex].name);
    } else {
        customPrint(purple, bold, bgDefault, "Service Node\n");
    }

    return compilationError::NO_ERRORS;
}

static const char *nameTypeToString(nameType type) {
    switch (type) {
        case nameType::IDENTIFIER: {
            return "identifier";
        }

        case nameType::OPERATOR: {
            return "operator";
        }

        case nameType::TYPE_NAME: {
            return "type name";
        }

        case nameType::SEPARATOR: {
            return "separator";
        }
    }

    return NULL;
}