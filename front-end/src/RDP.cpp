#include "RDP.h"

compilationError parseBuffer(compilationBuffer *content) {
    customWarning(content != NULL, compilationError::COMPILATION_BUFFER_ERROR);

    content->currentToken = 0;
    content->AST->root    = getGrammar(content);

    return compilationError::NO_COMPILATION_ERRORS;
}

node<astNode> *getGrammar(compilationBuffer *content) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getTranslationUnit(compilationBuffer *content) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getExternalDeclaration(compilationBuffer *content) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getFunctionDefinition(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getDeclaration(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getOperator(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getDeclarationInitializer(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getAssignmentExpression(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getConditionOperator(compilationBuffer *content, int localNameTableID, keywordIdentifier keyword) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getOperatorList(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *gegArgumentList(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getParameterList(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getReturnOperator(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}

node<astNode> *getOutOperator(compilationBuffer *content, int localNameTableID) {
    customWarning(content != NULL, NULL);
}
