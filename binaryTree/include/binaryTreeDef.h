#ifndef BINARY_TREE_DEF_H_
#define BINARY_TREE_DEF_H_

#include <cstdio>
#include <cstdlib>
#include <cinttypes>

#define FREE_(ptr) do { \
    free(ptr);          \
    ptr = NULL;         \
} while (0)

struct binaryTreeInfo {
    char *bornFileName         = {};
    char *bornFunctionName     = {};
    int   bornLine             = {};

    char *lastUsedFileName     = {};
    char *lastUsedFunctionName = {};
    int   lastUsedLine         = {};

    char *htmlDumpPath         = {};
    char *dumpFileName         = {};
};

enum class binaryTreeError {
    NO_ERRORS             =        0,
    TREE_NULL_POINTER     =  1 <<  0,
    NODE_NULL_POINTER     =  1 <<  1,
    ROOT_NULL_POINTER     =  1 <<  2,
    NODE_USED             =  1 <<  3,
    BAD_FILE_NAME_POINTER =  1 <<  4,
    BAD_FUNC_NAME_POINTER =  1 <<  5,
    BAD_BORN_LINE_VALUE   =  1 <<  6,
    BAD_INFO_POINTER      =  1 <<  7,
    INFO_NULL_POINTER     =  1 <<  8,
    BAD_BUFFER_POINTER    =  1 <<  9,
    BAD_HTML_NAME_POINTER =  1 << 10,
    DUMP_FILE_BAD_POINTER =  1 << 11,
    NO_SUCH_FILE          =  1 << 12,
    DOUBLE_FREE           =  1 << 13,
    BAD_STREAM_POINTER    =  1 << 14,
    CALLOC_ERROR          =  1 << 15
};

enum class linkDirection {
    LEFT   = 1 << 0,
    RIGHT  = 1 << 1,
    PARENT = 1 << 2
};

enum class printType {
    PREFIX  = 0,
    INFIX   = 1,
    POSTFIX = 2
};

static inline const size_t MAX_FILE_NAME_SIZE   =  100;
static inline const size_t MAX_BORN_FUNC_NAME   =  100;
static inline const int    MAX_LINE_LENGTH      =    4;
static inline const size_t MAX_DUMP_FOLDER_NAME =   20;
static inline const size_t MAX_DUMP_FILE_NAME   =   50;
static inline const size_t MAX_PATH_TO_FILE     =  100;
static inline const size_t MAX_CMD_BUFFER_SIZE  =  100;
static inline const size_t MAX_HEADER_SIZE      =  500;

template<typename DT>
struct node {
    DT        data   =   {};
    node<DT> *left   = NULL;
    node<DT> *right  = NULL;
    node<DT> *parent = NULL;
};

template<typename DT>
struct binaryTree {
    node<DT> *root           = NULL;
    binaryTreeInfo *infoData = NULL;
};

template<typename DT>
binaryTreeError treeInitialize         (binaryTree<DT> *tree);
template<typename DT>
binaryTreeError treeDestruct           (binaryTree<DT> *tree);
template<typename DT>
binaryTreeError nodeInitialize         (node<DT>      **currentNode);
template<typename DT>
binaryTreeError nodeLink               (binaryTree<DT> *tree, node<DT> *currentNode, linkDirection direction);
template<typename DT>
binaryTreeError nodeDestruct           (binaryTree<DT> *tree, node<DT> **node);

template<typename DT>
binaryTreeError callPrintBinaryTree    (binaryTree<DT> *tree,        printType type, FILE *stream);
template<typename DT>
binaryTreeError printBinaryTree        (node<DT>       *currentNode, printType type, FILE *stream);
template<typename DT>
binaryTreeError printNode              (node<DT>       *currentNode,                 FILE *stream);

template<typename DT>
binaryTreeError treeInfoInitialize     (binaryTree<DT> *tree, const char *fileName,
                                                              const char *funcName,
                                                              int         line);
template<typename DT>
binaryTreeError binaryTreeInfoDestruct (binaryTree<DT> *tree);
template<typename DT>
binaryTreeError binaryTreeSetInfo      (binaryTree<DT> *tree);

template<typename DT>
node<DT>       *emplaceNode            (node<DT> currentNode);

#endif // BINARY_TREE_DEF_H_