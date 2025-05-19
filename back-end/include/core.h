#ifndef CORE_H_
#define CORE_H_

#include "AST.h"
#include "binaryTreeDef.h"
#include "buffer.h"
#include "nameTable.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

enum class translationError {
    NO_ERRORS = 0,
    CONTEXT_BAD_POINTER = 1 << 0,
    AST_BAD_POINTER     = 1 << 1,
    BAD_FILE_NAME       = 1 << 2,
    FILE_READING_ERROR  = 1 << 3,
    BAD_BUFFER_POINTER  = 1 << 4,
    BAD_FILE_CONTENT    = 1 << 5,
};

struct translationContext {
    binaryTree<astNode> *AST = {};

    Buffer<nameTableElement> *nameTable   = {};
    Buffer<localNameTable>   *localTables = {};

    translationError error = translationError::NO_ERRORS;

    size_t entryPoint = 0;
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeTranslationContext(translationContext *context);
translationError destroyTranslationContext   (translationContext *context);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // CORE_H_