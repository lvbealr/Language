#ifndef TREE_READER_H_
#define TREE_READER_H_

#include "core.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError readAST            (translationContext *context, const char *fileName);
node<astNode>   *readASTInternal    (translationContext *context, Buffer<char> *fileContent, size_t *currentFilePosition);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError readNameTable      (translationContext *context, const char *fileName);
translationError readGlobalNameTable(translationContext *context, Buffer<char > *fileContent, size_t *currentFilePosition);
translationError readLocalNameTable (translationContext *context, size_t localTableIndex, Buffer<char > *fileContent, size_t *currentFilePosition);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // TREE_READER_H_