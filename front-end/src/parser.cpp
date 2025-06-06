#include "parser.h"
#include "core.h"
#include "nameTable.h"
#include "AST.h"
#include "buffer.h"
#include "binaryTreeDef.h"
#include <cstdio>

static node<astNode> *getBinaryOperation      (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getUnaryOperation       (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getPrimaryExpression    (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getComparison           (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getOperationWithPriority(compilationContext *context, size_t priority, int localNameTableID);

typedef node<astNode> *(* getterFunc) (compilationContext *, size_t, int, bool);

static const size_t MAX_PRIORITY = 5;

static const getterFunc nextFunction[]    = {getPrimaryExpression, getUnaryOperation, getBinaryOperation, getBinaryOperation, getComparison, getBinaryOperation};
static const size_t     operationsCount[] = {0, 6, 2, 2, 6, 2};
static const Keyword    operations[][6]   = {{},
                                             {Keyword::SIN,   Keyword::COS,              Keyword::NOT,           Keyword::FLOOR,   Keyword::SQRT, Keyword::SUB},
                                             {Keyword::MUL,   Keyword::DIV},
                                             {Keyword::ADD,   Keyword::SUB},
                                             {Keyword::EQUAL, Keyword::GREATER_OR_EQUAL, Keyword::LESS_OR_EQUAL, Keyword::GREATER, Keyword::LESS, Keyword::NOT_EQUAL},
                                             {Keyword::AND,   Keyword::OR}};
                                             
// ------------------------------------------------------------------------------------------------------------------------------------------------- //

#define IS_NULL(EXPRESSION, RET_POINTER) {   \
    if (!(EXPRESSION)) {                     \
        return RET_POINTER;                  \
    }                                        \
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

size_t getKeywordIndex(compilationContext *context, Keyword keyword) {
    for (size_t i = 0; i < context->nameTable->currentIndex; i++) {
        if (context->nameTable->data[i].keyword == keyword) {
            return i;
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getExpression(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    return nextFunction[MAX_PRIORITY](context, MAX_PRIORITY, localNameTableID, false);
}

static node<astNode> *getBinaryOperation(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context, NULL);

    node<astNode> *firstValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    IS_NULL(firstValue, NULL);

    while (true) {
        node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
        IS_NULL(operation, firstValue);

        if (onlyArythm) {
            SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
        }

        node<astNode> *secondValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
        IS_NULL(secondValue, NULL);

        operation->left     = firstValue;
        firstValue->parent  = operation;

        operation->right    = secondValue;
        secondValue->parent = operation;

        firstValue = operation;
    }
}

static node<astNode> *getUnaryOperation(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context, NULL);

    node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
    node<astNode> *value     = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);

    IS_NULL(value, NULL);

    if (operation) {
        if (onlyArythm) {
            SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
        }

        operation->right = value;
        value->parent    = operation;

        if (context->nameTable->data[operation->data.data.nameTableIndex].keyword == Keyword::SUB) {
            operation->left         = _CONST_(0);
            operation->left->parent = operation;
        }

        value = operation;
    }

    return value;
}

static node<astNode> *getComparison(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context, NULL);

    node<astNode> *firstValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    IS_NULL(firstValue, NULL);

    node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
    IS_NULL(operation, firstValue);

    if (onlyArythm) {
        SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
    }

    node<astNode> *secondValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    IS_NULL(secondValue, NULL);

    operation->left     = firstValue;
    firstValue->parent  = operation;

    operation->right    = secondValue;
    secondValue->parent = operation;

    return operation;
}

static node<astNode> *getPrimaryExpression(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context, NULL);

    if (getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED)) {
        node<astNode> *expression = nextFunction[MAX_PRIORITY](context, MAX_PRIORITY, localNameTableID, onlyArythm);

        IS_NULL(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED), NULL);

        return expression;
    }

    else {
        context->errorBuffer->currentIndex--;
    }

    node<astNode> *functionCall = getFunctionCall(context, localNameTableID);
    CHECK_FOR_ERROR(functionCall, compilationError::FUNCTION_CALL_EXPECTED);

    if (functionCall) {
        return functionCall;
    }

    node<astNode> *terminalSymbol = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);

    if (terminalSymbol) {
        DECLARATION_ASSERT(terminalSymbol, localNameType::VARIABLE_IDENTIFIER, compilationError::VARIABLE_NOT_DECLARED);

        return terminalSymbol;
    }

    context->errorBuffer->currentIndex--;
    terminalSymbol = getKeyword(context, Keyword::IN, compilationError::IN_EXPECTED);

    if (terminalSymbol) {
        return terminalSymbol;
    }

    context->errorBuffer->currentIndex--;

    terminalSymbol = getConstant(context);

    return terminalSymbol;
}

static node<astNode> *getOperationWithPriority(compilationContext *context, size_t priority, int localnameTableID) {
    customWarning(context, NULL);

    node<astNode> *operation = NULL;

    for (size_t operationIndex = 0; operationIndex < operationsCount[priority]; operationIndex++) {
        operation = getKeyword(context, operations[priority][operationIndex], compilationError::OPERATION_EXPECTED);

        if (!operation) {
            context->errorBuffer->currentIndex--;
        }

        else {
            break;
        }
    }

    return operation;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static node<astNode> *getGrammar              (compilationContext *context);
static node<astNode> *getTranslationUnit      (compilationContext *context);
static node<astNode> *getExternalDeclaration  (compilationContext *context);
static node<astNode> *getFunctionDefinition   (compilationContext *context, int     localNameTableID);
static node<astNode> *getDeclaration          (compilationContext *context, int     localNameTableID);
static node<astNode> *getOperator             (compilationContext *context, int     localNameTableID);
static node<astNode> *getInitializerDeclarator(compilationContext *context, int     localNameTableID);
static node<astNode> *getAssignmentExpression (compilationContext *context, int     localNameTableID);
static node<astNode> *getConditionOperator    (compilationContext *context, Keyword operatorKeyword, compilationError error, int localNameTableID);
static node<astNode> *getOperatorList         (compilationContext *context, int     localNameTableID);
static node<astNode> *getArgumentList         (compilationContext *context, int     localNameTableID);
static node<astNode> *getParameterList        (compilationContext *context, int     localNameTableID);
static node<astNode> *getReturnOperator       (compilationContext *context, int     localNameTableID);
static node<astNode> *getOutOperator          (compilationContext *context, int     localNameTableID);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

compilationError parseCode(compilationContext *context) {
    customWarning(context != NULL, compilationError::CONTEXT_ERROR);

    context->tokenIndex = 0;

    context->AST->root = getGrammar(context);

    return context->error;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static node<astNode> *getGrammar(compilationContext *context) {
    customWarning(context, NULL);

    IS_NULL(getTokenAndDestroy(context, Keyword::INITIAL_OPERATOR, compilationError::INITIAL_OPERATOR_EXPECTED), NULL);

    node<astNode> *entryPointIdentifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    IS_NULL(entryPointIdentifier, NULL);

    IS_NULL(getTokenAndDestroy(context, Keyword::OPERATOR_SEPARATOR, compilationError::OPERATOR_SEPARATOR_EXPECTED), NULL);

    context->entryPoint = entryPointIdentifier->data.data.nameTableIndex;

    node<astNode> *rootNode = getTranslationUnit(context);
    IS_NULL(rootNode, NULL);

    destroySingleNode(entryPointIdentifier);

    int localNameTableID = 0;

    for (size_t callIndex = 0; callIndex < context->functionCalls->currentIndex; callIndex++) {
        DECLARATION_ASSERT(context->functionCalls->data[callIndex], localNameType::FUNCTION_IDENTIFIER, compilationError::FUNCTION_NOT_DECLARED);
    }

    return rootNode;
}

static node<astNode> *getTranslationUnit(compilationContext *context) {
    customWarning(context, NULL);

    node<astNode> *externalDeclaration = getExternalDeclaration(context);
    IS_NULL(externalDeclaration, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING &&
                  context->nameTable->data[currentNameTableIndex].keyword == Keyword::OPERATOR_SEPARATOR,
                  compilationError::OPERATOR_SEPARATOR_EXPECTED);

    node<astNode> *separator = currentToken;
    ++context->tokenIndex;

    _OPERATOR_SEPARATOR_(separator, externalDeclaration, NULL);

    node<astNode> *root = separator;

    if (currentToken->data.type != nodeType::TERMINATOR) {
        root->right = getTranslationUnit(context);

        if (root->right) {
            root->right->parent = root;
        }
    }

    return root;
}

static node<astNode> *getExternalDeclaration(compilationContext *context) {
    customWarning(context, NULL);

    node<astNode> *root = getFunctionDefinition(context, 0);

    if (root) {
        return root;
    }

    CHECK_FOR_ERROR(root, compilationError::FUNCTION_EXPECTED);

    root = getDeclaration(context, 0);

    return root;
}

static node<astNode> *getFunctionDefinition(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    IS_NULL(getTokenAndDestroy(context, Keyword::FUNCTION_DEFINITION, compilationError::FUNCTION_EXPECTED) != false, NULL);
    
    node<astNode> *type = getStringToken(context, nameType::TYPE_NAME, compilationError::TYPE_NAME_EXPECTED);
    IS_NULL(type, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    IS_NULL(identifier, NULL);

    size_t identifierIndex = identifier->data.data.nameTableIndex;

    REDECLARATION_ASSERT(identifier,
                         (localNameType) ((int) localNameType::VARIABLE_IDENTIFIER | (int) localNameType::FUNCTION_IDENTIFIER),
                         compilationError::FUNCTION_REDEFINITION);

    --context->tokenIndex;
    DESTROY_CURRENT_NODE();
    ++context->tokenIndex;

    int newNameTableIndex = addLocalNameTable((int) identifierIndex, context->localTables);
    
    addLocalIdentifier(0, context->localTables,
                       localNameTableElement {.type = localNameType::FUNCTION_IDENTIFIER, .globalNameID = identifierIndex}, 1);
    
    IS_NULL(getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED), NULL);
    
    node<astNode> *parameters = getParameterList(context, newNameTableIndex);
    CHECK_FOR_ERROR(parameters, compilationError::TYPE_NAME_EXPECTED);

    IS_NULL(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED),    NULL);
    IS_NULL(getTokenAndDestroy(context, Keyword::BLOCK_OPEN,    compilationError::CODE_BLOCK_EXPECTED), NULL);

    node<astNode> *functionContent = getOperatorList(context, newNameTableIndex);
    CHECK_FOR_ERROR(functionContent, compilationError::OPERATOR_NOT_FOUND);

    IS_NULL(getTokenAndDestroy(context, Keyword::BLOCK_CLOSE, compilationError::CODE_BLOCK_EXPECTED), NULL);

    return _FUNCTION_DEFINITION_(type, _PARAMETERS_(parameters, functionContent), identifierIndex);
}

static node<astNode> *getDeclaration(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *type = getStringToken(context, nameType::TYPE_NAME, compilationError::TYPE_NAME_EXPECTED);
    IS_NULL(type, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    IS_NULL(identifier, NULL);

    context->tokenIndex--;

    size_t identifierIndex = identifier->data.data.nameTableIndex;

    REDECLARATION_ASSERT(identifier,
                        (localNameType) ((int) localNameType::VARIABLE_IDENTIFIER | (int) localNameType::FUNCTION_IDENTIFIER),
                        compilationError::VARIABLE_REDECLARATION);

    addLocalIdentifier(localNameTableID, context->localTables,
                       localNameTableElement {.type = localNameType::VARIABLE_IDENTIFIER, .globalNameID = identifierIndex}, 1);

    node<astNode> *initializerDeclarator = getInitializerDeclarator(context, localNameTableID);
    IS_NULL(initializerDeclarator, NULL);

    return _VARIABLE_DECLARATION_(type, initializerDeclarator, identifierIndex);
}

static node<astNode> *getInitializerDeclarator(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *initializer = getAssignmentExpression(context, localNameTableID);

    if (initializer) {
        return initializer;
    }

    CHECK_FOR_ERROR(initializer, compilationError::ASSIGNMENT_EXPECTED);
    context->tokenIndex--;

    initializer = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);

    return initializer;
}

static node<astNode> *getAssignmentExpression (compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    IS_NULL(identifier, NULL);

    DECLARATION_ASSERT(identifier, localNameType::VARIABLE_IDENTIFIER, compilationError::VARIABLE_NOT_DECLARED);

    node<astNode> *assignmentOperation = getKeyword(context, Keyword::ASSIGNMENT, compilationError::ASSIGNMENT_EXPECTED);
    IS_NULL(assignmentOperation, NULL);

    node<astNode> *expression = getExpression(context, localNameTableID);
    IS_NULL(expression, NULL);

    assignmentOperation->left  = expression;
    expression->parent         = assignmentOperation;

    assignmentOperation->right = identifier;
    identifier->parent         = assignmentOperation;

    return assignmentOperation;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static node<astNode> *getOperator(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *expectedOperator = NULL;

    expectedOperator = getConditionOperator(context, Keyword::IF, compilationError::IF_EXPECTED, localNameTableID);
    CHECK_FOR_ERROR(expectedOperator, compilationError::IF_EXPECTED);

    if (expectedOperator) {
        size_t separatorIndex = getKeywordIndex(context, Keyword::OPERATOR_SEPARATOR);
        node<astNode> *separator = emplaceNode(node<astNode>
            {
                .data = {
                    .type = nodeType::STRING,
                    .data = {.nameTableIndex = separatorIndex},
                    .line = context->currentLine
                },
                .left = expectedOperator,
                .right = NULL,
                .parent = NULL
            });
        if (expectedOperator) expectedOperator->parent = separator;
        return separator;
    }

    expectedOperator = getConditionOperator(context, Keyword::WHILE, compilationError::WHILE_EXPECTED, localNameTableID);
    CHECK_FOR_ERROR(expectedOperator, compilationError::WHILE_EXPECTED);

    if (expectedOperator) {
        size_t separatorIndex = getKeywordIndex(context, Keyword::OPERATOR_SEPARATOR);
        node<astNode> *separator = emplaceNode(node<astNode>
            {
                .data = {
                    .type = nodeType::STRING,
                    .data = {.nameTableIndex = separatorIndex},
                    .line = context->currentLine
                },
                .left = expectedOperator,
                .right = NULL,
                .parent = NULL
            });
        if (expectedOperator) expectedOperator->parent = separator;
        return separator;
    }

    #define GET_EXPECTED_OPERATOR(FUNCTION, ERROR)              \
        expectedOperator = FUNCTION(context, localNameTableID); \
        TRY_GET_OPERATOR(ERROR);

    do {
        expectedOperator = getKeyword(context, Keyword::ABORT, compilationError::ABORT_EXPECTED);
        TRY_GET_OPERATOR(ABORT_EXPECTED);

        expectedOperator = getKeyword(context, Keyword::BREAK, compilationError::BREAK_EXPECTED);
        TRY_GET_OPERATOR(BREAK_EXPECTED);

        expectedOperator = getKeyword(context, Keyword::CONTINUE, compilationError::CONTINUE_EXPECTED);
        TRY_GET_OPERATOR(CONTINUE_EXPECTED);

        GET_EXPECTED_OPERATOR(getOutOperator,          OUT_EXPECTED);
        GET_EXPECTED_OPERATOR(getReturnOperator,       RETURN_EXPECTED);
        GET_EXPECTED_OPERATOR(getFunctionCall,         FUNCTION_CALL_EXPECTED);
        GET_EXPECTED_OPERATOR(getAssignmentExpression, IDENTIFIER_EXPECTED);
        GET_EXPECTED_OPERATOR(getDeclaration,          TYPE_NAME_EXPECTED);
        
        IS_NULL(getTokenAndDestroy(context, Keyword::BLOCK_OPEN, compilationError::OPERATOR_NOT_FOUND), NULL);
        expectedOperator = getOperatorList(context, localNameTableID);
        IS_NULL(expectedOperator, NULL);
        IS_NULL(getTokenAndDestroy(context, Keyword::BLOCK_CLOSE, compilationError::OPERATOR_NOT_FOUND), NULL);
    } while (0);

    node<astNode> *separator = getKeyword(context, Keyword::OPERATOR_SEPARATOR, compilationError::OPERATOR_SEPARATOR_EXPECTED);
    customWarning(separator, NULL);

    if (!separator) {
        size_t separatorIndex = getKeywordIndex(context, Keyword::OPERATOR_SEPARATOR);
        separator = emplaceNode(node<astNode>
            {
                .data = {
                    .type = nodeType::STRING,
                    .data = {.nameTableIndex = separatorIndex},
                    .line = context->currentLine
                },
                .left = expectedOperator,
                .right = NULL,
                .parent = NULL
            });
        if (expectedOperator) expectedOperator->parent = separator;
        return separator;
    }

    _OPERATOR_SEPARATOR_(separator, expectedOperator, NULL);

    return separator;
}

static node<astNode> *getOperatorList(compilationContext *context, int localNameTableID){
    customWarning(context, NULL);

    node<astNode> *firstOperator = getOperator(context, localNameTableID);
    IS_NULL(firstOperator, NULL);

    node<astNode> *secondOperator = getOperatorList(context, localNameTableID);

    firstOperator->right = secondOperator;

    CHECK_FOR_ERROR(secondOperator, compilationError::OPERATOR_NOT_FOUND);

    if (secondOperator) {
        secondOperator->parent = firstOperator;
    }

    return firstOperator;
}

static node<astNode> *getConditionOperator(compilationContext *context, Keyword operatorKeyword, compilationError error, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *conditionOperator = getKeyword(context, operatorKeyword, error);
    IS_NULL(conditionOperator, NULL);

    node<astNode> *conditionExpression = getExpression(context, localNameTableID);
    IS_NULL(conditionExpression, NULL);

    IS_NULL(getTokenAndDestroy(context, Keyword::CONDITION_SEPARATOR, compilationError::CONDITION_SEPARATOR_EXPECTED), NULL);

    node<astNode> *operatorContent = getOperator(context, localNameTableID);
    IS_NULL(operatorContent, NULL);

    conditionOperator->left     = conditionExpression;
    conditionExpression->parent = conditionOperator;

    conditionOperator->right    = operatorContent;
    operatorContent->parent     = conditionOperator;

    return conditionOperator;
}

static node<astNode> *getReturnOperator(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *returnStatement = getKeyword(context, Keyword::RETURN, compilationError::RETURN_EXPECTED);
    IS_NULL(returnStatement, NULL);

    node<astNode> *expression = getExpression(context, localNameTableID);
    IS_NULL(expression, NULL);

    returnStatement->right = expression;
    expression->parent     = returnStatement;

    return returnStatement;
}

static node<astNode> *getOutOperator(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *outOperator = getKeyword(context, Keyword::OUT, compilationError::OUT_EXPECTED);
    IS_NULL(outOperator, NULL);

    node<astNode> *expression = getExpression(context, localNameTableID);
    IS_NULL(expression, NULL);

    outOperator->right = expression;
    expression->parent = outOperator;

    return outOperator;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getFunctionCall(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    IS_NULL(getTokenAndDestroy(context, Keyword::FUNCTION_CALL, compilationError::FUNCTION_CALL_EXPECTED), NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    IS_NULL(identifier, NULL);
    writeDataToBuffer(context->functionCalls, &identifier, 1);

    IS_NULL(getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED), NULL);

    node<astNode> *arguments = getArgumentList(context, localNameTableID);
    CHECK_FOR_ERROR(arguments, compilationError::CONSTANT_EXPECTED);

    IS_NULL(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED), NULL);

    return _FUNCTION_CALL_(arguments, identifier);
}

static node<astNode> *getArgumentList(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *argument = getExpression(context, localNameTableID);
    IS_NULL(argument, NULL);

    node<astNode> *separator = getKeyword(context, Keyword::ARGUMENT_SEPARATOR, compilationError::ARGUMENT_SEPARATOR_EXPECTED);

    if (!separator) {
        context->errorBuffer->currentIndex--;
        size_t separatorIndex = getKeywordIndex(context, Keyword::ARGUMENT_SEPARATOR);
        separator = emplaceNode(node<astNode>
            {
                .data = {
                    .type = nodeType::STRING,
                    .data = {.nameTableIndex = separatorIndex},
                    .line = context->currentLine
                },
                .left = argument,
                .right = NULL,
                .parent = NULL
            });
        if (argument) argument->parent = separator;
        return separator;
    }

    node<astNode> *nextArgument = getArgumentList(context, localNameTableID);
    IS_NULL(nextArgument, NULL);

    _ARGUMENTS_SEPARATOR_(separator, argument, nextArgument);

    return separator;
}

static node<astNode> *getParameterList(compilationContext *context, int localNameTableID) {
    customWarning(context, NULL);

    node<astNode> *parameter = getDeclaration(context, localNameTableID);
    IS_NULL(parameter, NULL);

    node<astNode> *separator = getKeyword(context, Keyword::ARGUMENT_SEPARATOR, compilationError::ARGUMENT_SEPARATOR_EXPECTED);

    if (!separator) {
        context->errorBuffer->currentIndex--;
        size_t separatorIndex = getKeywordIndex(context, Keyword::ARGUMENT_SEPARATOR);
        separator = emplaceNode(node<astNode>
            {
                .data = {
                    .type = nodeType::STRING,
                    .data = {.nameTableIndex = separatorIndex},
                    .line = context->currentLine
                },
                .left = parameter,
                .right = NULL,
                .parent = NULL
            });
        if (parameter) parameter->parent = separator;
        return separator;
    }

    node<astNode> *nextParameter = getParameterList(context, localNameTableID);
    IS_NULL(nextParameter, NULL);

    _ARGUMENTS_SEPARATOR_(separator, parameter, nextParameter);

    return separator;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getStringToken(compilationContext *context, nameType type, compilationError error) {
    customWarning(context, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING && context->nameTable->data[currentNameTableIndex].type == type, error);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

node<astNode> *getKeyword(compilationContext *context, Keyword keyword, compilationError error) {
    customWarning(context, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING && context->nameTable->data[currentNameTableIndex].keyword == keyword, error);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

node<astNode> *getConstant(compilationContext *context) {
    customWarning(context, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::CONSTANT, compilationError::CONSTANT_EXPECTED);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

bool getTokenAndDestroy(compilationContext *context, Keyword keyword, compilationError error) {
    customWarning(context, false);

    if (!getKeyword(context, keyword, error)) {
        return false;
    }

    context->tokenIndex--;
    DESTROY_CURRENT_NODE();
    context->tokenIndex++;

    return true;
}

bool isIdentifierDeclared(compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType) {
    customWarning(context, false);

    if (isLocalIdentifierDeclared(context, localNameTableID, identifierIndex, identifierType)) {
        return true;
    }

    int nameIndex = getIndexInLocalTable(0, context->localTables, identifierIndex, identifierType);

    if (nameIndex >= 0) {
        return true;
    }

    return false;
}

bool isLocalIdentifierDeclared(compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType) {
    customWarning(context != NULL, false);

    int localNameIndex = getIndexInLocalTable(localNameTableID, context->localTables, identifierIndex, identifierType);

    if (localNameIndex >= 0) {
        return true;
    }

    return false;
}
