#ifndef CORE_H_
#define CORE_H_

#include "nameTable.h"
#include "AST.h"
#include "bufferExtensions.h"
#include "lexer.h"

enum class compilationError { // TODO
    NO_COMPILATION_ERRORS       = 0,
    COMPILATION_BUFFER_ERROR    = 1,
    TOKENS_BUFFER_ERROR         = 2,
    NAME_TABLE_ERROR            = 3,
    LOCAL_TABLES_ERROR          = 4,
    AST_ERROR                   = 5,
    FUNCTION_CALLS_BUFFER_ERROR = 6,
    TOKEN_INDEX_IS_OUT_OF_RANGE = 7,
    CONSTANT_EXPECTED           = 8,
    INITIAL_OPERATOR_EXPECTED   = 9,
    IDENTIFIER_EXPECTED         = 10,
    OPERATOR_SEPARATOR_EXPECTED = 11
};

struct compilationBuffer {
    Buffer<nameTableElement> *nameTable     = {};
    Buffer<localNameTable>   *localTables   = {};
    Buffer<Token>            *tokens        = {};

    binaryTree<astNode>      *AST           = {};

    size_t currentToken                     = {};

    int currentScope                        = -1;
}; // TODO

// TODO FUNCTIONS TO INITIALIZE ALL FIELD, NOW FROM MAIN ((

compilationError dumpTokenList(compilationBuffer *content);
compilationError dumpToken    (compilationBuffer *content, size_t tokenIndex);

node<astNode> *ASTnodeCreate(node<astNode> nodeData);

#endif // CORE_H_
