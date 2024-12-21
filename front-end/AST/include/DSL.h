#ifndef DSL_H_
#define DSL_H_

#include "AST.h"
#include "binaryTreeDef.h"
#include "nameTable.h"
#include "core.h"

#define KEYWORD_(leftNode, rightNode, keywordID)     ASTnodeCreate({.data = {.type = nodeType::KEYWORD,              .data = {.ID     = keywordID},    .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define CONST_(NUMBER)                               ASTnodeCreate({.data = {.type = nodeType::CONSTANT,             .data = {.number = NUMBER},       .scope = content->currentScope}, .left = NULL, .right = NULL,  .parent = NULL})
#define IDENTIFIER_(identifierID)                    ASTnodeCreate({.data = {.type = nodeType::IDENTIFIER,           .data = {.ID     = identifierID}, .scope = content->currentScope}, .left = NULL, .right = NULL,  .parent = NULL})
#define PARAMETER_LIST_(getParameters, functionBody) ASTnodeCreate({.data = {.type = nodeType::PARAMETERS,           .data = {.ID     = -1},           .scope = content->currentScope}, .left = getParameters,                 \
                                                                                                                                                                                        .right = functionBody, .parent = NULL})

#define FUNCTION_CALL_(getArguments, identifierID)   ASTnodeCreate({.data = {.type = nodeType::FUNCTION_CALL,        .data = {.ID     = -1},           .scope = content->currentScope}, .left  = getArguments,                 \
                                                                                                                                                                                        .right = IDENTIFIER_(identifierID)})
#define VARIABLE_DECLARATION_(typeName, ID_or_getExp, identifierID)                                                                                                                                                            \
                                                     ASTnodeCreate({.data = {.type = nodeType::VARIABLE_DECLARATION, .data = {.ID     = identifierID}, .scope = content->currentScope}, .left  = typeName,                     \
                                                                                                                                                                                        .right = ID_or_getExpr})
#define FUNCTION_DEFINITION_(typeName, getParameters, identifierID)                                                                                                                                                            \
                                                     ASTnodeCreate({.data = {.type = nodeType::FUNCTION_DEFINITION,  .data = {.ID     = identifierID}, .scope = content->currentScope}, .left  = typeName,                     \
                                                                                                                                                                                        .right = getParameters})

#endif // DSL_H_
