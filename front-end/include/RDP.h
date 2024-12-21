#ifndef RDP_H_
#define RDP_H_

#include "AST.h"
#include "core.h"

#define CURRENT_TOKEN_ content->tokens->data[content->currentToken]
#define CURRENT_NAME_TABLE_ID_ CURRENT_TOKEN_.localTableID

// TODO

#define SYNTAX_ASSERT_(expression, error) do {                \
    if (!(expression)) {                                      \
        customPrint(red, bold, bgDefault, "Syntax Error!\n"); \
        return NULL;                                          \
    }                                                         \
} while (0)

#define NULL_(expression) do {  \
    if ((!expression)) {        \
        return NULL;            \
    }                           \
} while (0)

static const size_t MAX_WORD_LENGTH = 100;

#define как_и_(leftNode, rightNode)           ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::AND},              .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define или_(leftNode, rightNode)             ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::OR},               .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define корефанится_с_(leftNode, rightNode)   ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::EQUAL},            .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define сосет_у_(leftNode, rightNode)         ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::LESS},             .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define петушит_(leftNode, rightNode)         ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::GREATER},          .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define почти_петушит_(leftNode, rightNode)   ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::LESS_OR_EQUAL},    .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define почти_сосет_у_(leftNode, rightNode)   ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::GREATER_OR_EQUAL}, .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define путает_рамсы_с_(leftNode, rightNode)  ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::NOT_EQUAL},        .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})

#define плюс_(leftNode, rightNode)            ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::ADD},              .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define минус_(leftNode, rightNode)           ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::SUB},              .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})

#define помножить_на_(leftNode, rightNode)    ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::MUL},              .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})
#define рассечь_(leftNode, rightNode)         ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::DIV},              .scope = content->currentScope}, .left = leftNode, .right = rightNode, .parent = NULL})

#define синус_(currNode)                      ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::SIN},              .scope = content->currentScope}, .left = currNode, .right = NULL,      .parent = NULL})
#define косинус_(currNode)                    ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::COS},              .scope = content->currentScope}, .left = currNode, .right = NULL,      .parent = NULL})
#define сквирт_(currNode)                     ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::SQRT},             .scope = content->currentScope}, .left = currNode, .right = NULL,      .parent = NULL})
#define не_(currNode)                         ASTnodeCreate({.data = {.type = nodeType::KEYWORD, .data = {.ID = (int) keywordIdentifier::NOT},              .scope = content->currentScope}, .left = currNode, .right = NULL,      .parent = NULL})

#define MAIN_PRIORITY_FUNC_(func)                                 \
    content->currentToken++;                                      \
    CHECK_OPEN_BRACKET_();                                        \
    content->currentToken++;                                      \
    node<astNode> *returnNode = func##_(getPriority5(content));   \
    CHECK_CLOSE_BRACKET_();                                       \
    content->currentToken++;                                      \
    return returnNode;

#define OPERATION_(operator, firstNum, secondNum)                                                              \
    if (content->tokens->data[content->currentToken].type == nameType::OPERATOR &&                             \
        (!strncmp(content->tokens->data[content->currentToken].data.pointer, #operator, strlen(#operator)))) { \
            content->currentToken++;                                                                           \
            return operator##_(newNode_##firstNum, getPriority##secondNum(content));                           \
        }                                                                                                      \

// FUNCTION PROTOTYPES //
node<astNode>    *ASTnodeCreate         (node<astNode>      nodeData);

node<astNode>    *getGrammar            (compilationBuffer *content);
node<astNode>    *getStatement          (compilationBuffer *content);
node<astNode>    *getFunctionDefinition (compilationBuffer *content);
node<astNode>    *getVarDeclaration     (compilationBuffer *content);
node<astNode>    *getAssignment         (compilationBuffer *content);
node<astNode>    *getExpression         (compilationBuffer *content);
node<astNode>    *getPriority5          (compilationBuffer *content);
node<astNode>    *getPriority4          (compilationBuffer *content);
node<astNode>    *getPriority3          (compilationBuffer *content);
node<astNode>    *getPriority2          (compilationBuffer *content);
node<astNode>    *getPriority1          (compilationBuffer *content);
node<astNode>    *getValue              (compilationBuffer *content);
node<astNode>    *getNumber             (compilationBuffer *content);
node<astNode>    *getIdentifier         (compilationBuffer *content);
node<astNode>    *getFunctionCall       (compilationBuffer *content);
node<astNode>    *getIfStatement        (compilationBuffer *content);
node<astNode>    *getWhileStatement     (compilationBuffer *content);
node<astNode>    *getParameters         (compilationBuffer *content);
node<astNode>    *getParameter          (compilationBuffer *content);
node<astNode>    *getReturnType         (compilationBuffer *content);
node<astNode>    *getArgumentList       (compilationBuffer *content);
node<astNode>    *getExpression         (compilationBuffer *content);
node<astNode>    *getInStatement        (compilationBuffer *content);
node<astNode>    *getOutStatement       (compilationBuffer *content);

keywordIdentifier findKeywordIDbyName   (Token token);
// FUNCTION PROTOTYPES //

#endif // RDP_H_
