#include <cstdio>

#include "AST.h"
#include "binaryTreeDef.h"
#include "customWarning.h"

node<astNode> *mountNode (node<astNode> currentNode) {
    node<astNode> *newNode = NULL;

    nodeInitialize(&newNode);

    customWarning(newNode != NULL, NULL);

    *newNode = currentNode;

    if (newNode->right) {
        newNode->right->parent = newNode;
    }

    if (newNode->left) {
        newNode->left->parent = newNode;
    }

    return newNode;
}
