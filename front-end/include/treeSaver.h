#ifndef TREE_SAVER_H_
#define TREE_SAVER_H_

#include "buffer.h"
#include "core.h"
#include "AST.h"
#include <stdlib.h>

enum class saveDataError {
    NO_ERRORS                =  0,
    FILE_OPEN_ERROR          =  1,
    FILE_WRITE_ERROR         =  2,
    FILE_CLOSE_ERROR         =  3,
    CONTEXT_BAD_POINTER      =  4,
    BAD_FILENAME             =  5,
    ALLOCATION_ERROR         =  6,
    TREE_BAD_POINTER         =  7,
    NODE_BAD_POINTER         =  8,
    INITIALIZATION_ERROR     =  9,
    FILE_BAD_POINTER         = 10,
    SAVE_CONTEXT_BAD_POINTER = 11,
    BUFFER_BAD_POINTER       = 12,
    BUFFER_WRITE_ERROR       = 13,
    NAME_TABLE_BAD_POINTER   = 14,
};

struct saveDataContext {
    Buffer<char> *NTBuffer   = {};
    char         *NTFileName = {};

    Buffer<char> *ASTBuffer   = {};
    char         *ASTFileName = {};
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

char *getFileName();

saveDataError initializeSaveDataContext(saveDataContext *saveContext, char *NTfileName, char *ASTfileName);
saveDataError destroySaveDataContext   (saveDataContext *saveContext);

saveDataError saveNameTable(compilationContext *context, saveDataContext *saveContext);

saveDataError saveGlobalNameTable(compilationContext *context, saveDataContext *saveContext, size_t keywordsCount);
saveDataError saveLocalNameTable(compilationContext *context, saveDataContext *saveContext, size_t localTableIndex, size_t keywordsCount);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

saveDataError saveASTTree   (compilationContext *context, saveDataContext *saveContext);
saveDataError saveASTSubtree(compilationContext *context, saveDataContext *saveContext, node<astNode> *node, size_t keywordsCount);
saveDataError saveASTNode   (compilationContext *context, saveDataContext *saveContext, node<astNode> *node, size_t keywordsCount);

#endif // TREE_SAVER_H_