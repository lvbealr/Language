#include "core.h"
#include "customWarning.h"
#include <stdlib.h>
#include "AST.h"
#include "buffer.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeTranslationContext(translationContext *context) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);

    context->AST = (binaryTree<node<astNode> *> *)calloc(1, sizeof(binaryTree<node<astNode> *>));
    customWarning(context->AST, translationError::AST_BAD_POINTER);

    context->nameTable = (Buffer<nameTableElement> *)calloc(1, sizeof(Buffer<nameTableElement>));
    customWarning(context->nameTable, translationError::AST_BAD_POINTER);

    context->localTables = (Buffer<localNameTable> *)calloc(1, sizeof(Buffer<localNameTable>));
    customWarning(context->localTables, translationError::AST_BAD_POINTER);

    return translationError::NO_ERRORS;
}

translationError destroyTranslationContext(translationContext *context) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);

    if (context->AST) {
        FREE_(context->AST);
    }

    if (context->nameTable) {
        bufferDestruct(context->nameTable);
    }

    if (context->localTables) {
        bufferDestruct(context->localTables);
    }

    context->error = translationError::NO_ERRORS;

    context->entryPoint = 0;

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //