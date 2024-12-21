#include "RDP.h"
#include "DSL.h"

keywordIdentifier findKeywordIDbyName(Token token) {
    char lexemeName[MAX_WORD_LENGTH] = {};

    int ID = 0;

    #define KEYWORD(NAME, NUMBER, LEXEME, ...) {                                                \
        sscanf(token.data.pointer, "%s", lexemeName);                                           \
        if (!strncmp(lexemeName, LEXEME, strlen(LEXEME))) {                                     \
            ID = NUMBER;                                                                        \
        }                                                                                       \
    }

    #include "keywords.def"

    return static_cast <keywordIdentifier> (ID);
}

#define $DEBUG_PRINT_() {                                                                       \
    customPrint(red, bold, bgDefault, "FUNCTION: [%s], LINE [%d]: ", __FUNCTION__, __LINE__);   \
    dumpToken(content, content->currentToken);                                                  \
}


#define GET_NEXT_TOKEN_()                                                                       \
    if (content->currentToken < content->tokens->count) {                                       \
        content->currentToken++;                                                                \
    }                                                                                           \
                                                                                                \
    else {                                                                                      \
        customPrint(red, bold, bgDefault, "Can not access to next token: invalid size!\n");     \
        return NULL;                                                                            \
    }

#define SYNTAX_ERROR_() {                                                                       \
    customPrint(red, bold, bgDefault, "FUCKING SYNTAX ERROR [%d]\n", __LINE__);                 \
    exit(-1);                                                                                   \
}

#define CHECK_SEPARATOR_() {                                                                    \
    if (content->tokens->data[content->currentToken].type != nameType::SEPARATOR) {             \
        return NULL;                                                                            \
    }                                                                                           \
}

#define CHECK_NAME_TYPE_() {                                                                    \
    if (content->tokens->data[content->currentToken].type != nameType::NAME_TYPE) {             \
        return NULL;                                                                            \
    }                                                                                           \
}

#define CHECK_NAME_() {                                                                         \
    if (content->tokens->data[content->currentToken].type != nameType::NAME) {                  \
        return NULL;                                                                            \
    }                                                                                           \
}

#define CHECK_BLOCK_OPEN_() {                                                                                              \
    if (strncmp(content->tokens->data[content->currentToken].data.pointer, "пошел_раскумар", strlen("пошел_раскумар"))) {  \
        return NULL;                                                                                                       \
    }                                                                                                                      \
}

#define CHECK_CONDITION_OPERATOR_() {                                                                   \
    if (strncmp(content->tokens->data[content->currentToken].data.pointer, "если", strlen("если")))  {  \
        return NULL;                                                                                    \
    }                                                                                                   \
}

#define CHECK_OPEN_BRACKET_() {                                                                                            \
    if (*content->tokens->data[content->currentToken].data.pointer != '(') {                                               \
        return NULL;                                                                                                       \
    }                                                                                                                      \
}

#define CHECK_CLOSE_BRACKET_() {                                                                                           \
    if (*content->tokens->data[content->currentToken].data.pointer != ')') {                                               \
        return NULL;                                                                                                       \
    }                                                                                                                      \
}

node<astNode> *getGrammar(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode = NULL;

    $DEBUG_PRINT_();

    if (content->currentToken < content->tokens->count) {
        $DEBUG_PRINT_();

        customPrint(green, bold, bgDefault, "CURRENT TOKEN: [%lu], TOKEN COUNT: [%lu]\n\n",  content->currentToken,
                                                                                             content->tokens->count);

        $DEBUG_PRINT_();

        newNode = KEYWORD_(getStatement(content), getGrammar(content), (int) nameType::SEPARATOR);
    }

    return newNode;
}

node<astNode> *getStatement(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode = NULL;

    $DEBUG_PRINT_();

    if (newNode = getFunctionDefinition(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }

    if (newNode = getVarDeclaration(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }

    if (newNode = getAssignment(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }

    if (newNode = getFunctionCall(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }
    
    if (newNode = getFunctionDefinition(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }
    
    if (newNode = getIfStatement(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }
    
    if (newNode = getWhileStatement(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }
    
    if (newNode = getInStatement(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }
    
    if (newNode = getOutStatement(content)) {
        $DEBUG_PRINT_();

        return newNode;
    }

    SYNTAX_ERROR_();

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getFunctionDefinition(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode = NULL;

    $DEBUG_PRINT_();

    if (content->tokens->data[content->currentToken].type == nameType::NAME_TYPE) {
        $DEBUG_PRINT_();

        node<astNode> *returnTypeNode = getReturnType(content);

        $DEBUG_PRINT_();
        
        GET_NEXT_TOKEN_(); // FUNCTION IDENTIFIER

        $DEBUG_PRINT_();
        // ADD TO NAME TABLE // TODO
        GET_NEXT_TOKEN_(); // IS BRACKET?

        $DEBUG_PRINT_();

        CHECK_SEPARATOR_();

        content->currentToken += 2;

        if (!getParameter(content)) {
            content->currentToken -= 2;
        }

        return NULL;

        $DEBUG_PRINT_();

        newNode = FUNCTION_DEFINITION_(returnTypeNode, getParameters(content), content->nameTable->count);

        $DEBUG_PRINT_();
    }

    return newNode;
}

node<astNode> *getVarDeclaration(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getAssignment(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    // check name table

    node<astNode> *assignNode = KEYWORD_(NULL, NULL, (int) keywordIdentifier::ASSIGNMENT);
    assignNode->left  = IDENTIFIER_(-666);

    content->currentToken += 2;

    assignNode->right = getExpression(content);

    content->currentToken += 1;

    return assignNode;
}

node<astNode> *getFunctionCall(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getIfStatement(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    node<astNode> *newNode = NULL;
    nodeInitialize(&newNode);

    CHECK_CONDITION_OPERATOR_();

    GET_NEXT_TOKEN_();

    if (*content->tokens->data[content->currentToken].data.pointer == '(') {
        
        $DEBUG_PRINT_();

        GET_NEXT_TOKEN_();

        newNode->left = getExpression(content);

        if (*content->tokens->data[content->currentToken].data.pointer != ')') {
            $DEBUG_PRINT_();

            SYNTAX_ERROR_();
        }

        GET_NEXT_TOKEN_();

        $DEBUG_PRINT_();
    }

    CHECK_BLOCK_OPEN_();

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getWhileStatement(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getParameters(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    node<astNode> *newNode = PARAMETER_LIST_(NULL, NULL);

    $DEBUG_PRINT_();

    if (*content->tokens->data[content->currentToken].data.pointer == '(') {

        $DEBUG_PRINT_();

        GET_NEXT_TOKEN_();
        newNode->left = getParameter(content);

        if (*content->tokens->data[content->currentToken].data.pointer != ')') {
            $DEBUG_PRINT_();

            SYNTAX_ERROR_();
        }

        GET_NEXT_TOKEN_();

        $DEBUG_PRINT_();
    }

    $DEBUG_PRINT_();

    CHECK_BLOCK_OPEN_();

    GET_NEXT_TOKEN_();

    newNode->right = getGrammar(content);

    return newNode;
}

node<astNode> *getParameter(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode = NULL;

    $DEBUG_PRINT_();

    CHECK_NAME_TYPE_();

    $DEBUG_PRINT_();

    GET_NEXT_TOKEN_();

    $DEBUG_PRINT_();
    
    CHECK_NAME_();

    // ADD TO NAME TABLE // TODO

    $DEBUG_PRINT_();

    GET_NEXT_TOKEN_();

    CHECK_SEPARATOR_();

    $DEBUG_PRINT_();

    if (*content->tokens->data[content->currentToken].data.pointer == ')') {
        return newNode;
    }

    else {
        GET_NEXT_TOKEN_();

        $DEBUG_PRINT_();

        newNode = KEYWORD_(IDENTIFIER_(content->nameTable->count), getParameter(content), (int) keywordIdentifier::ARGUMENT_SEPARATOR);
    }

    $DEBUG_PRINT_();

    return newNode;
}

node<astNode> *getReturnType(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    keywordIdentifier ID = findKeywordIDbyName(content->tokens->data[content->currentToken]);

    if ((int) ID != 0) {
        customPrint(green, bold, bgDefault, "FUNCTION: [%s], LINE: [%d]: FOUND keywordID: %d\n\n", __FUNCTION__, __LINE__, ID);
    }

    else {
        customPrint(yellow, bold, bgDefault, "FUNCTION: [%s], LINE: [%d]: keywordID: %d\n\n", __FUNCTION__, __LINE__, ID);
    }

    return NULL;
}

node<astNode> *getArgumentList(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getExpression(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return getPriority5(content);
}

node<astNode> *getPriority5(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode_4 = getPriority4(content);

    OPERATION_(как_и, 4, 5);
    OPERATION_(или,   4, 5);

    return newNode_4;
}

node<astNode> *getPriority4(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode_3 = getPriority3(content);

    OPERATION_(корефанится_с,  3, 4);
    OPERATION_(сосет_у,        3, 4);
    OPERATION_(петушит,        3, 4);
    OPERATION_(почти_петушит,  3, 4);
    OPERATION_(почти_сосет_у,  3, 4);
    OPERATION_(путает_рамсы_с, 3, 4);

    return newNode_3;
}

node<astNode> *getPriority3(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode_2 = getPriority2(content);
    
    size_t subCount = 0;

    while (content->tokens->data[content->currentToken].type == nameType::OPERATOR && 
            ((!strncmp(content->tokens->data[content->currentToken].data.pointer, "плюс",  strlen("плюс"))) ||
             (!strncmp(content->tokens->data[content->currentToken].data.pointer, "минус", strlen("минус"))))) {
        
        if (!strncmp(content->tokens->data[content->currentToken].data.pointer, "минус", strlen("минус"))) {
            subCount++;
            content->currentToken++;
        }
    }

    if (subCount % 2 == 0) {
        content->currentToken++;

        return плюс_(newNode_2, getPriority3(content));
    }

    else {
        content->currentToken++;

        return минус_(newNode_2, getPriority3(content));
    }

    return newNode_2;
}

node<astNode> *getPriority2(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode_1 = getPriority1(content);

    OPERATION_(помножить_на, 1, 2);
    OPERATION_(рассечь,      1, 2);

    return newNode_1;
}

node<astNode> *getPriority1(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    if (content->tokens->data[content->currentToken].type == nameType::OPERATOR) {
        if (!strncmp(content->tokens->data[content->currentToken].data.pointer, "синус", strlen("синус"))) {
            MAIN_PRIORITY_FUNC_(синус);
        }

        if (!strncmp(content->tokens->data[content->currentToken].data.pointer, "косинус", strlen("косинус"))) {
            MAIN_PRIORITY_FUNC_(косинус);
        }

        if (!strncmp(content->tokens->data[content->currentToken].data.pointer, "сквирт", strlen("сквирт"))) {
            MAIN_PRIORITY_FUNC_(сквирт);
        }

        if (!strncmp(content->tokens->data[content->currentToken].data.pointer, "не", strlen("не"))) {
            MAIN_PRIORITY_FUNC_(не);
        }
    }

    printf("CURRENT TOKEN: %lu\n", content->currentToken);

    $DEBUG_PRINT_();

    return getValue(content);
}

node<astNode> *getValue(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    if ((content->tokens->data[content->currentToken].type == nameType::SEPARATOR) && (*content->tokens->data[content->currentToken].data.pointer == '(')) {

        $DEBUG_PRINT_();

        content->currentToken++;

        if (content->tokens->data[content->currentToken].type == nameType::OPERATOR && 
        (!strncmp(content->tokens->data[content->currentToken].data.pointer, "минус", strlen("минус")))) {
            content->currentToken += 1;
            node<astNode> *returnValue = getNumber(content);

            returnValue->data.data.number = (-1) * returnValue->data.data.number;

            content->currentToken++;

            CHECK_CLOSE_BRACKET_();

            return returnValue;
        }

        $DEBUG_PRINT_();

        content->currentToken++;

        $DEBUG_PRINT_();

        node<astNode> *returnValue = getPriority5(content);

        $DEBUG_PRINT_();

        CHECK_CLOSE_BRACKET_();

        $DEBUG_PRINT_();

        content->currentToken++;

        return returnValue;
    }

    if (content->tokens->data[content->currentToken].type == nameType::NUMBER) {
        $DEBUG_PRINT_();

        return getNumber(content);
    }

    if (content->tokens->data[content->currentToken].type == nameType::IDENTIFIER) {
        $DEBUG_PRINT_();

        if (content->tokens->data[content->currentToken + 1].type == nameType::SEPARATOR &&
            (*content->tokens->data[content->currentToken + 1].data.pointer == '(')) {
                content->currentToken++;
                node<astNode> *returnValue = getFunctionCall(content);

                if (!returnValue) {
                    SYNTAX_ERROR_();
                }

                return returnValue;
            }

        return getIdentifier(content);
    }

    return NULL;
}

node<astNode> *getNumber(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *newNode = CONST_(content->tokens->data[content->currentToken].data.value);

    return newNode;
}

node<astNode> *getIdentifier(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    node<astNode> *returnValue = IDENTIFIER_(666);

    content->currentToken++;

    return returnValue;
}

node<astNode> *getInStatement(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}

node<astNode> *getOutStatement(compilationBuffer *content) {
    customWarning(content != NULL, NULL);

    $DEBUG_PRINT_();

    return NULL;
}
