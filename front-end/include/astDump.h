#ifndef AST_DUMP_H_
#define AST_DUMP_H_

#include <stdio.h>

#include "customWarning.h"
#include "AST.h"
#include "core.h"

enum class dumpError {
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
    DUMP_CONTEXT_BAD_POINTER = 11
};

struct dumpContext {
    char     *fileName = {};
    FILE     *file     = NULL;
    dumpError error    = dumpError::NO_ERRORS;
};

static const size_t MAX_DUMP_SIZE = 128;

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

dumpError initializeDumpContext(dumpContext *dumpContext, const char *fileName);
dumpError destroyDumpContext   (dumpContext *dumpContext);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

char     *getASTFileName(binaryTree<astNode> *tree);

dumpError dumpTree(compilationContext *context, dumpContext *dumpContext, binaryTree<astNode> *tree);
dumpError dumpNode(compilationContext *context, dumpContext *dumpContext, node<astNode> *node);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // AST_DUMP_H_

// TODO DUMP ERROR (errorWriter.h)
// TODO SAVE TREE (treeSaver.h)