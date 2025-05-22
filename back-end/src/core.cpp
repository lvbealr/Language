#include <stdlib.h>

#include "AST.h"
#include "buffer.h"
#include "customWarning.h"
#include "core.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeTranslationContext(translationContext *context) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);

    context->AST = (binaryTree<astNode> *)calloc(1, sizeof(binaryTree<node<astNode> *>));
    customWarning(context->AST, translationError::AST_BAD_POINTER);

    context->nameTable = (Buffer<nameTableElement> *)calloc(1, sizeof(Buffer<nameTableElement>));
    customWarning(context->nameTable, translationError::BUFFER_BAD_POINTER);

    context->localTables = (Buffer<localNameTable> *)calloc(1, sizeof(Buffer<localNameTable>));
    customWarning(context->localTables, translationError::BUFFER_BAD_POINTER);

    context->counters = (operatorCounters *)calloc(1, sizeof(operatorCounters));
    customWarning(context->counters, translationError::COUNTERS_BAD_POINTER);

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

    if (context->counters) {
        FREE_(context->counters);
    }
    
    context->error = translationError::NO_ERRORS;

    context->entryPoint = 0;

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //