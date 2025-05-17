#ifndef AST_H_
#define AST_H_

#include <cmath>

#include "binaryTree.h"
#include "nameTable.h"

static const int POISON_VALUE = -666;

enum class nodeType {
    TERMINATOR           = 0,
    CONSTANT             = 1,
    STRING               = 2,
    KEYWORD              = 3,
    FUNCTION_DEFINITION  = 4,
    PARAMETERS           = 5,
    VARIABLE_DECLARATION = 6,
    FUNCTION_CALL        = 7
};

union nodeData {
    size_t  nameTableIndex;         // identifier index in global name table
    Keyword keyword;
    int     number = POISON_VALUE;
};

struct astNode {
    nodeType type  = nodeType::CONSTANT;
    nodeData data  = {.number = POISON_VALUE};
    int      line  = 0;
    char    *file  = NULL;
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define _CONST_(NUMBER)                                 \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::CONSTANT,                \
             .data = {.number = NUMBER},                \
             .line = context->currentLine               \
            },                                          \
        .left   = NULL,                                 \
        .right  = NULL,                                 \
        .parent = NULL                                  \
        }                                               \
    )

#define _NAME_(INDEX)                                   \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::STRING,                  \
             .data = {.nameTableIndex = INDEX},         \
             .line = context->currentLine               \
            },                                          \
        .left   = NULL,                                 \
        .right  = NULL,                                 \
        .parent = NULL                                  \
        }                                               \
    )

#define _FUNCTION_DEFINITION_(LEFT, RIGHT, ID_INDEX)    \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::FUNCTION_DEFINITION,     \
             .data = {.nameTableIndex = ID_INDEX},      \
             .line = context->currentLine               \
            },                                          \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

#define _VARIABLE_DECLARATION_(LEFT, RIGHT, ID_INDEX)   \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::VARIABLE_DECLARATION,    \
             .data = {.nameTableIndex = ID_INDEX},      \
             .line = context->currentLine               \
            },                                          \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

#define _PARAMETERS_(LEFT, RIGHT)                       \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::PARAMETERS},             \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

#define _FUNCTION_CALL_(LEFT, RIGHT)                    \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::FUNCTION_CALL},          \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

#define _TERMINATOR_()                                  \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::TERMINATOR},             \
        .left   = NULL,                                 \
        .right  = NULL,                                 \
        .parent = NULL                                  \
        }                                               \
    )

#define _OPERATOR_SEPARATOR_(LEFT, RIGHT)               \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::STRING,                  \
             .data = {.nameTableIndex = 0}              \
            },                                          \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

#define _ARGUMENTS_SEPARATOR_(LEFT, RIGHT)              \
    emplaceNode(node<astNode>                           \
        {.data =                                        \
            {.type = nodeType::STRING,                  \
             .data = {.nameTableIndex = 1}              \
            },                                          \
        .left   = LEFT,                                 \
        .right  = RIGHT,                                \
        .parent = NULL                                  \
        }                                               \
    )

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // AST_H_