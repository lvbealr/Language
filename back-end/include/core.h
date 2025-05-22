#ifndef CORE_H_
#define CORE_H_

#include <map>
#include <stdbool.h>

#include "AST.h"
#include "binaryTreeDef.h"
#include "buffer.h"
#include "nameTable.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

enum class translationError {
    NO_ERRORS            =       0,
    CONTEXT_BAD_POINTER  = 1 <<  0,
    AST_BAD_POINTER      = 1 <<  1,
    BAD_FILE_NAME        = 1 <<  2,
    FILE_READING_ERROR   = 1 <<  3,
    BUFFER_BAD_POINTER   = 1 <<  4,
    BAD_FILE_CONTENT     = 1 <<  5,
    FILE_OPEN_ERROR      = 1 <<  6,
    NODE_BAD_POINTER     = 1 <<  7,
    AST_BAD_STRUCTURE    = 1 <<  8,
    COUNTERS_BAD_POINTER = 1 <<  9,
    NAME_TABLE_ERROR     = 1 << 10,
};

const size_t INITIAL_ADDRESS = 0x10000;

struct operatorCounters {
    size_t assignmentCount = 0;
    size_t ifCount         = 0;
    size_t whileCount      = 0;
    size_t callCount       = 0;
    size_t logicCount      = 0;
};

struct translationContext {
    binaryTree<astNode> *AST = {};

    Buffer<nameTableElement> *nameTable   = {};
    Buffer<localNameTable>   *localTables = {};

    translationError error = translationError::NO_ERRORS;

    size_t entryPoint = 0;

    operatorCounters *counters = {};

    bool callParameters = false;

    const size_t initialAddress = INITIAL_ADDRESS;

    std::map<size_t, size_t> functionToLocalTable;
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeTranslationContext(translationContext *context);
translationError destroyTranslationContext   (translationContext *context);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // CORE_H_