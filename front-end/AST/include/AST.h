#ifndef AST_H_
#define AST_H_

#include <cmath>

#include "binaryTree.h"
#include "nameTable.h"

enum class nodeType {
    CONSTANT             = 1,
    STRING               = 2,
    KEYWORD              = 3,
    FUNCTION_DEFENITION  = 4,
    FUNCTION_ARGS        = 5,
    FUNCTION_CALL        = 6,
    VARIABLE_DECLARATION = 7
};

union nodeData {
    double            number = NAN;
    size_t            nameTableID;
    keywordIdentifier keywordID;
};

struct astNode {
    nodeType type = nodeType::CONSTANT;
    nodeData data = {.number = NAN};
    int      line = 0;
    char    *file = NULL;
};

node<astNode> *mountNode (node<astNode> currentNode);

#endif // AST_H_
