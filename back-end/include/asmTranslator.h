#ifndef ASM_TRANSLATOR_H_
#define ASM_TRANSLATOR_H_

#include <stdlib.h>
#include "core.h"

struct codeBlock {
    size_t blockID    = {};

    char *blockName   = {};
    char *blockSource = {};
};

const size_t MAX_NUMBER_LENGTH = 32;

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError translateToASM(translationContext *context, const char *fileName);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeProgram(translationContext *context, Buffer<char> *outputData);
translationError traverseAST      (translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // ASM_TRANSLATOR_H_