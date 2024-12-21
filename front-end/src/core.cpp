#include <cctype>

#include "core.h"

static const char  *tokensNames[]   = {"LATIN", "CYRILLIC", "OPERATOR", "SEPARATOR", "NUMBER", "NAME_TYPE", "NAME", "UNDEFINED"};
static const size_t MAX_WORD_LENGTH = 100;

compilationError dumpTokenList(compilationBuffer *content) {
    customWarning(content != NULL, compilationError::COMPILATION_BUFFER_ERROR);

    for (size_t tokenIndex = 0; tokenIndex < content->tokens->count; tokenIndex++) {
        dumpToken(content, tokenIndex);
    }

    return compilationError::NO_COMPILATION_ERRORS;
}

compilationError dumpToken(compilationBuffer *content, size_t tokenIndex) {
    customWarning(content         != NULL, compilationError::COMPILATION_BUFFER_ERROR);
    customWarning(content->tokens != NULL, compilationError::TOKENS_BUFFER_ERROR);

    // customPrint(green, bold, bgDefault, "TREE ROOT: %p\n\n\n", content->AST);

    DUMP_(content->AST);
    
    // customWarning(tokenIndex < content->tokens->count, compilationError::TOKEN_INDEX_IS_OUT_OF_RANGE);

    if (tokenIndex >= content->tokens->count) {
        
        exit(-1);
        return compilationError::TOKEN_INDEX_IS_OUT_OF_RANGE;
    }

    if (content->tokens->data[tokenIndex].type == nameType::NUMBER) {
        customPrint(green, bold, bgDefault, "[TOKEN #%lu]::[%s] <=> [%d]\n\n", tokenIndex,
        tokensNames[(int) content->tokens->data[tokenIndex].type], content->tokens->data[tokenIndex].data.value);
    }

    else if (content->tokens->data[tokenIndex].type == nameType::SEPARATOR && ispunct(content->tokens->data[tokenIndex].data.pointer[0])) {
        customPrint(lightblue, bold, bgDefault, "[TOKEN #%lu]::[%s] <=> [%c]\n\n", tokenIndex,
        tokensNames[(int) content->tokens->data[tokenIndex].type], content->tokens->data[tokenIndex].data.pointer[0]);
    }

    else if (content->tokens->data[tokenIndex].type != nameType::UNDEFINED) {
        char string[MAX_WORD_LENGTH] = {};
        sscanf(content->tokens->data[tokenIndex].data.pointer, "%s", string);
        customPrint(purple, bold, bgDefault, "[TOKEN #%lu]::[%s] <=> [%s]\n\n", tokenIndex,
        tokensNames[(int) content->tokens->data[tokenIndex].type], string);
    }

    else {
        customPrint(yellow, bold, bgDefault, "[TOKEN #%lu] ", tokenIndex);
        customPrint(white, bold, bgDefault, "is ");
        customPrint(red, bold, bgDefault, "UNDEFINED\n\n");
    }

    return compilationError::NO_COMPILATION_ERRORS;
}

node<astNode> *ASTnodeCreate (node<astNode> nodeData) {
    node<astNode> *newNode = NULL;
    nodeInitialize(&newNode);

    customWarning(newNode != NULL, NULL);

    newNode->data   = nodeData.data;
    newNode->left   = nodeData.left;
    newNode->right  = nodeData.right;
    newNode->parent = nodeData.parent;

    if (newNode->left) {
        newNode->left->parent = newNode;
    }

    if (newNode->right) {
        newNode->right->parent = newNode;
    }

    return newNode;
}
