#ifndef PARSER_H_
#define PARSER_H_

#include "core.h"
#include "AST.h"
#include "binaryTreeDef.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

compilationError parseCode    (compilationContext *context);
node<astNode>   *getExpression(compilationContext *context, int localNameTableID);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define DECLARATION_ASSERT(IDENTIFIER, IDENTIFIER_TYPE, ERROR) {    \
    SYNTAX_ASSERT(isIdentifierDeclared(context, localNameTableID,   \
    IDENTIFIER->data.data.nameTableIndex, IDENTIFIER_TYPE), ERROR); \
}

#define REDECLARATION_ASSERT(IDENTIFIER, IDENTIFIER_TYPE, ERROR) {      \
    SYNTAX_ASSERT(!isLocalIdentifierDeclared(context, localNameTableID, \
    IDENTIFIER->data.data.nameTableIndex, IDENTIFIER_TYPE), ERROR);     \
}

#define TRY_GET_OPERATOR(ERROR) {                               \
    CHECK_FOR_ERROR(expectedOperator, compilationError::ERROR); \
    if (expectedOperator) {                                     \
        break;                                                  \
    }                                                           \
}

#define SYNTAX_ASSERT(EXPRESSION, ERROR) do {                                \
    if (!(EXPRESSION)) {                                                     \
        errorData newError = errorData {                                     \
            .error = ERROR,                                                  \
            .line  = context->tokens->data[context->tokenIndex]->data.line,  \
            .file  = context->tokens->data[context->tokenIndex]->data.file}; \
                                                                             \
        writeDataToBuffer(context->errorBuffer, &newError, 1);               \
        return NULL;                                                         \
    }                                                                        \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define currentToken context->tokens->data[context->tokenIndex]
#define currentNameTableIndex currentToken->data.data.nameTableIndex

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

template<typename DT>
binaryTreeError destroySingleNode(node<DT> *currentNode) {
    customWarning(currentNode != NULL, binaryTreeError::ROOT_NULL_POINTER);

    FREE_(currentNode);

    return binaryTreeError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define DESTROY_CURRENT_NODE() do {  \
    destroySingleNode(currentToken); \
    currentToken = NULL;             \
} while (0)                 

#define CHECK_FOR_ERROR(NODE, ERROR) do {                                                        \
    if (!(NODE)) {                                                                               \
        if (context->errorBuffer->data[context->errorBuffer->currentIndex - 1].error != ERROR) { \
            return NULL;                                                                         \
        }                                                                                        \
                                                                                                 \
        context->errorBuffer->currentIndex--;                                                    \
    }                                                                                            \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getStringToken (compilationContext *context, nameType type,    compilationError error);
node<astNode> *getKeyword     (compilationContext *context, Keyword  keyword, compilationError error);
node<astNode> *getConstant    (compilationContext *context);
node<astNode> *getFunctionCall(compilationContext *context, int localNameTableID);

bool getTokenAndDestroy       (compilationContext *context, Keyword keyword,      compilationError error);
bool isIdentifierDeclared     (compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType);
bool isLocalIdentifierDeclared(compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // PARSER_H_