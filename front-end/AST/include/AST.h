#ifndef AST_H_
#define AST_H_

#include <cmath>

#include "binaryTree.h"
#include "nameTable.h"

static const size_t POISON_VALUE = -666;

enum class nodeType {
    CONSTANT             = 1,
    IDENTIFIER           = 2,
    KEYWORD              = 3,
    FUNCTION_DEFINITION  = 4,
    PARAMETERS           = 5,
    VARIABLE_DECLARATION = 6,
    FUNCTION_CALL        = 7
};

union nodeData {
    size_t  nameTableIndex;
    Keyword keyword;
    int     number = POISON_VALUE;
};

struct astNode {
    nodeType type  = nodeType::CONSTANT;
    nodeData data  = {.number = POISON_VALUE};
    int      line  = 0;
    char    *file  = NULL;
};

// TODO
#define CONST_(NUMBER)
#define NAME_(INDEX)
#define FUNCTION_DEFINITION_(LEFT, RIGHT, ID_INDEX)
#define VARIABLE_DECLARATION_(LEFT, RIGHT, ID_INDEX)
#define PARAMETERS_(LEFT, RIGHT)
#define FUNCTION_CALL_(LEFT, RIGHT)
#define OPERATORS_SEPARATOR_(LEFT, RIGHT)
#define PARAMETERS_SEPARATOR_(LEFT, RIGHT)

#endif // AST_H_