#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include <cstdlib>
#include <ctime>
#include <cinttypes>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "binaryTreeDef.h"
#include "customWarning.h"
#include "binaryTreeDump.h"
#include "consoleParser.h"

#ifndef _NDEBUG
    #define DUMP_(treePtr) {                                                                       \
      strncpy((treePtr)->infoData->lastUsedFileName,     __FILE__,            MAX_FILE_NAME_SIZE); \
      strncpy((treePtr)->infoData->lastUsedFunctionName, __PRETTY_FUNCTION__, MAX_FILE_NAME_SIZE); \
      (treePtr)->infoData->lastUsedLine  =               __LINE__                                ; \
      binaryTreeDump(treePtr);                                                                     \
    }

    #define SAVE_DUMP_IMAGE(treePtr) {                                      \
      DUMP_(tree);                                                          \
                                                                            \
      char *buffer = (char *)calloc(MAX_CMD_BUFFER_SIZE, sizeof(char));     \
      customWarning(buffer != NULL, BAD_BUFFER_POINTER);                    \
                                                                            \
      char *newFileName = (char *)calloc(MAX_FILE_NAME_SIZE, sizeof(char)); \
      customWarning(newFileName != NULL, BAD_FILE_NAME_POINTER);            \
                                                                            \
      strncpy(newFileName, (treePtr)->infoData->htmlDumpPath,               \
      strlen((treePtr)->infoData->htmlDumpPath) - strlen(".html"));         \
                                                                            \
      snprintf(buffer, MAX_CMD_BUFFER_SIZE, "dot -Tsvg %s -o %s.svg",       \
              (treePtr)->infoData->dumpFileName, newFileName);              \
      system(buffer);                                                       \
                                                                            \
      FREE_(buffer);                                                        \
      FREE_(newFileName);                                                   \
    }
#else
    #define DUMP_(treePtr)
    #define SAVE_DUMP_IMAGE(treePtr)
#endif

#define BINARY_TREE_INITIALIZE(treePtr) {                                 \
    treeInitialize(treePtr);                                              \
    treeInfoInitialize(treePtr, __FILE__, __PRETTY_FUNCTION__, __LINE__); \
}

template<typename DT>
inline binaryTreeError treeInitialize(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    if (nodeInitialize(&tree->root) != binaryTreeError::NO_ERRORS) {
        return binaryTreeError::ROOT_NULL_POINTER;
    }

    tree->root->left  = NULL;
    tree->root->right = NULL;

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError treeDestruct(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    if (tree->root) {
        nodeDestruct(tree, &(tree->root));
    }

    binaryTreeInfoDestruct(tree);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError nodeInitialize(node<DT> **currentNode) {
    *currentNode = (node<DT> *)calloc(1, sizeof(node<DT>));

    customWarning(*currentNode != NULL, binaryTreeError::NODE_NULL_POINTER);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError nodeLink(binaryTree<DT> *tree, node<DT> *currentNode, linkDirection direction) {
    customWarning(tree        != NULL, binaryTreeError::TREE_NULL_POINTER);
    customWarning(currentNode != NULL, binaryTreeError::NODE_NULL_POINTER);

    node<DT> **childNode = NULL;

    switch (direction) {
        case linkDirection::LEFT:
        {
            childNode = &(currentNode->left);
            break;
        }

        case linkDirection::RIGHT:
        {
            childNode = &(currentNode->right);
            break;
        }

        case linkDirection::PARENT:
        default:
        {
            return binaryTreeError::NO_ERRORS;
            break;
        }
    }

    if (*childNode) {
        return binaryTreeError::NODE_USED;
    }

    node<DT> *newNode = {};
    nodeInitialize(&newNode);

    newNode->parent = currentNode;
    newNode->left   =        NULL;
    newNode->right  =        NULL;
    *childNode      =     newNode;

    DUMP_(tree);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError nodeDestruct(binaryTree<DT> *tree, node<DT> **currentNode) {
    customWarning(tree         != NULL, binaryTreeError::TREE_NULL_POINTER);
    customWarning(*currentNode != NULL, binaryTreeError::NODE_NULL_POINTER);

    if ((*currentNode)->left) {
        nodeDestruct(tree, &((*currentNode)->left));
    }

    if ((*currentNode)->right) {
        nodeDestruct(tree, &((*currentNode)->right));
    }

    DUMP_(tree);

    FREE_(*currentNode);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError callPrintBinaryTree(binaryTree<DT> *tree, printType type, FILE *stream) {
    customWarning(tree   != NULL, binaryTreeError::TREE_NULL_POINTER);
    customWarning(stream != NULL, binaryTreeError::BAD_STREAM_POINTER);

    printBinaryTree(tree->root, type, stream);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError printBinaryTree(node<DT> *currentNode, printType type, FILE *stream) {
    customWarning(currentNode   != NULL, binaryTreeError::NODE_NULL_POINTER);
    customWarning(stream        != NULL, binaryTreeError::BAD_STREAM_POINTER);

    fprintf(stream, "(");

    if (type == printType::PREFIX) {
        printNode(currentNode, stream);
    }

    if (currentNode->left) {
        printBinaryTree(currentNode->left, type, stream);
    }

    if (type == printType::INFIX) {
        printNode(currentNode, stream);
    }

    if (currentNode->right) {
        printBinaryTree(currentNode->right, type, stream);
    }

    if (type == printType::POSTFIX) {
        printNode(currentNode, stream);
    }

    fprintf(stream, ")");

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError printNode(node<DT> *currentNode, FILE *stream) {
    customWarning(currentNode != NULL, binaryTreeError::NODE_NULL_POINTER);

    if (currentNode->data) {
        fprintf(stream, "%d", currentNode->data);
    }

    else {
        fprintf(stream, "nul");
    }

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError treeInfoInitialize(binaryTree<DT> *tree, const char *fileName,
                                                                const char *funcName,
                                                                int         line) {
    customWarning(tree     != NULL, binaryTreeError::TREE_NULL_POINTER);
    customWarning(fileName != NULL, binaryTreeError::BAD_FILE_NAME_POINTER);
    customWarning(funcName != NULL, binaryTreeError::BAD_FUNC_NAME_POINTER);
    customWarning(line      > 0,    binaryTreeError::BAD_BORN_LINE_VALUE);

    tree->infoData = (binaryTreeInfo *)calloc(1, sizeof(binaryTreeInfo));

    customWarning(tree->infoData != NULL, binaryTreeError::BAD_INFO_POINTER);

    tree->infoData->bornFileName         = (char *)calloc(MAX_FILE_NAME_SIZE,   sizeof(char));
    tree->infoData->bornFunctionName     = (char *)calloc(MAX_BORN_FUNC_NAME,   sizeof(char));
    tree->infoData->bornLine             = line;

    tree->infoData->lastUsedFileName     = (char *)calloc(MAX_FILE_NAME_SIZE,   sizeof(char));
    tree->infoData->lastUsedFunctionName = (char *)calloc(MAX_BORN_FUNC_NAME,   sizeof(char));
    tree->infoData->lastUsedLine         = line;

    tree->infoData->dumpFileName         = (char *)calloc(MAX_FILE_NAME_SIZE,   sizeof(char));
    tree->infoData->htmlDumpPath         = (char *)calloc(MAX_DUMP_FILE_NAME,   sizeof(char));

    customWarning(tree->infoData->bornFileName         != NULL, binaryTreeError::CALLOC_ERROR);
    customWarning(tree->infoData->bornFunctionName     != NULL, binaryTreeError::CALLOC_ERROR);

    customWarning(tree->infoData->lastUsedFileName     != NULL, binaryTreeError::CALLOC_ERROR);
    customWarning(tree->infoData->lastUsedFunctionName != NULL, binaryTreeError::CALLOC_ERROR);

    customWarning(tree->infoData->dumpFileName         != NULL, binaryTreeError::CALLOC_ERROR);
    customWarning(tree->infoData->htmlDumpPath         != NULL, binaryTreeError::CALLOC_ERROR);

    strncpy(tree->infoData->bornFileName,     fileName, MAX_FILE_NAME_SIZE);
    strncpy(tree->infoData->bornFunctionName, funcName, MAX_BORN_FUNC_NAME);

    binaryTreeSetInfo(tree);

    DUMP_(tree);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
inline binaryTreeError binaryTreeInfoDestruct(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    FREE_(tree->infoData->bornFileName        );
    FREE_(tree->infoData->bornFunctionName    );

    FREE_(tree->infoData->lastUsedFileName    );
    FREE_(tree->infoData->lastUsedFunctionName);

    FREE_(tree->infoData->htmlDumpPath        );
    FREE_(tree->infoData->dumpFileName        );

    tree->infoData->bornLine     = 0;
    tree->infoData->lastUsedLine = 0;

    FREE_(tree->infoData);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
binaryTreeError setDumpFileName(binaryTree<DT> *tree) {
    const time_t currentTime = time(NULL);
    tm localTime             = *localtime(&currentTime);

    char *fileName = (char *)calloc(MAX_FILE_NAME_SIZE, sizeof(char));

    customWarning(fileName != NULL, binaryTreeError::CALLOC_ERROR);

    snprintf(fileName, FILENAME_MAX, "%s/%.2d.%.2d.%.4d-%.2d:%.2d:%.2d.html",
             __OPTIONS_DATA__.dumpFolderName, localTime.tm_mday, localTime.tm_mon,
             localTime.tm_year + 1900,        localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    strncpy(tree->infoData->htmlDumpPath, fileName, MAX_DUMP_FILE_NAME);

    FREE_(fileName);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT>
binaryTreeError binaryTreeSetInfo(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    char *buffer = (char *)calloc(MAX_CMD_BUFFER_SIZE, sizeof(char));
    customWarning(buffer != NULL, binaryTreeError::BAD_BUFFER_POINTER);

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "mkdir -p %s", __OPTIONS_DATA__.dumpFolderName);

    system(buffer);

    *buffer = {};

    setDumpFileName(tree);

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "touch %s", tree->infoData->htmlDumpPath);
    system(buffer);

    FREE_(buffer);

    return binaryTreeError::NO_ERRORS;
}

#endif // BINARY_TREE_H_



