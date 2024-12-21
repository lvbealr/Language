#ifndef AST_H_
#define AST_H_

#include <cmath>

#include "binaryTree.h"
#include "nameTable.h"

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
    int number = POISON_VALUE;
    int ID;
};

struct astNode {
    nodeType type  = nodeType::CONSTANT;
    nodeData data  = {.number = POISON_VALUE};
    int      scope = -1;
    int      line  = 0;
    char    *file  = NULL;
};

node<astNode> *mountNode (node<astNode> currentNode);

#endif // AST_H_
