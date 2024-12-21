#ifndef RDP_H_
#define RDP_H_

#include "AST.h"
#include "core.h"

// FUNCTION PROTOTYPES //
compilationError parseBuffer              (compilationBuffer *content);

node<astNode>   *getGrammar               (compilationBuffer *content);
node<astNode>   *getTranslationUnit       (compilationBuffer *content);
node<astNode>   *getExternalDeclaration   (compilationBuffer *content);
node<astNode>   *getFunctionDefinition    (compilationBuffer *content, int localNameTableID);
node<astNode>   *getDeclaration           (compilationBuffer *content, int localNameTableID);
node<astNode>   *getOperator              (compilationBuffer *content, int localNameTableID);
node<astNode>   *getDeclarationInitializer(compilationBuffer *content, int localNameTableID);
node<astNode>   *getAssignmentExpression  (compilationBuffer *content, int localNameTableID);
node<astNode>   *getConditionOperator     (compilationBuffer *content, int localNameTableID, keywordIdentifier keyword);
node<astNode>   *getOperatorList          (compilationBuffer *content, int localNameTableID);
node<astNode>   *gegArgumentList          (compilationBuffer *content, int localNameTableID);
node<astNode>   *getParameterList         (compilationBuffer *content, int localNameTableID);
node<astNode>   *getReturnOperator        (compilationBuffer *content, int localNameTableID);
node<astNode>   *getOutOperator           (compilationBuffer *content, int localNameTableID);
// FUNCTION PROTOTYPES //

#endif // RDP_H_
