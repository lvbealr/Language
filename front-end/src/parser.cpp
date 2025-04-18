#include "parser.h"
#include "core.h"
#include "nameTable.h"
#include "AST.h"
#include "buffer.h"
#include "binaryTreeDef.h"

static node<astNode> *getBinaryOperation      (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getUnaryOperation       (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getPrimaryExpression    (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getComparison           (compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);
static node<astNode> *getOperationWithPriority(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm);

typedef node<astNode> *(* getter_t) (compilationContext *, size_t, int, bool)

const size_t MAX_PRIORITY = 5;

static const getter_t nextFunction[]    = {getPrimaryExpression, getUnaryOperation, getBinaryOperation, getComparison, getBinaryOperation};
static const size_t   operationsCount[] = {0, 6, 2, 2, 6, 2};
static const Keyword  operations[][6]   = {{},
                                           {Keyword::SIN,   Keyword::COS,              Keyword::NOT,           Keyword::FLOOR,   Keyword::SQRT, Keyword::SUB},
                                           {Keyword::MUL,   Keyword::DIV},
                                           {Keyword::ADD,   Keyword::SUB},
                                           {Keyword::EQUAL, Keyword::GREATER_OR_EQUAL, Keyword::LESS_OR_EQUAL, Keyword::GREATER, Keyword::LESS, Keyword::NOT_EQUAL},
                                           {Keyword::AND,   Keyword::OR}};

node<astNode> *getExpression(compilationContext *context, int localNameTableID) {
    customWarning(context != NULL, NULL);

    return nextFunction[MAX_PRIORITY](context, MAX_PRIORITY, localNameTableID, false);
}

static node<astNode> *getBinaryOperation(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context != NULL, NULL);

    node<astNode> *firstValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    customWarning(firstValue != NULL, NULL);

    while (true) {
        node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
        customWarning(operation != NULL, firstValue);

        if (onlyArythm) {
            SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
        }

        node<astNode> *secondValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
        customWarning(secondValue != NULL, NULL);

        operation->left     = firstValue;
        firstValue->parent  = operation;

        operation->right    = secondValue;
        secondValue->parent = operation;

        firstValue = operation;
    }
}

static node<astNode> *getUnaryOperation(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context != NULL, NULL);

    node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
    node<astNode> *value     = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);

    customWarning(value != NULL, NULL);

    if (operation) {
        if (onlyArythm) {
            SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
        }

        operation->right = value;
        value->parent    = operation;

        if (context->nameTable.data[operation->data.content.nameTableIndex].keyWord == Keyword::SUB) {
            operation->left         = CONST_(0);
            operation->left->parent = operation;
        }

        value = operation;
    }

    return value;
}

static node<astNode> *getComparison(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context != NULL, NULL);

    node<astNode> *firstValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    customWarning(firstValue != NULL, NULL);

    node<astNode> *operation = getOperationWithPriority(context, priority, localNameTableID);
    customWarning(operation != NULL, firstValue);

    if (onlyArythm) {
        SYNTAX_ASSERT(false, compilationError::OPERATION_EXPECTED);
    }

    node<astNode> *secondValue = nextFunction[priority - 1](context, priority - 1, localNameTableID, onlyArythm);
    customWarning(secondValue != NULL, NULL);

    operation->left     = firstValue;
    firstValue->parent  = operation;

    operation->right    = secondValue;
    secondValue->parent = operation;

    return operation;
}

static node<astNode> *getPrimaryExpression(compilationContext *context, size_t priority, int localNameTableID, bool onlyArythm) {
    customWarning(context != NULL, NULL);

    if (getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED)) {
        node<astNode> *expression = nextFunction[MAX_PRIORITY](context, MAX_PRIORITY, localNameTableID, onlyArythm);

        customWarning(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED) != false);
        
        return expression;
    }

    else {
        context->errorBuffer.currentIndex--;
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

    context->errorBuffer.currentIndex--;

    terminalSymbol = getKeyword(context, Keyword::IN, compilationError::IN_EXPECTED);

    if (terminalSymbol) {
        return terminalSymbol;
    }

    context->errorBuffer.currentIndex--;

    terminalSymbol = getConstant(context);

    return terminalSymbol;
}

static node<astNode> *getOperationWithPriority(compilationContext *context, size_t priority, int localnameTableID) {
    customWarning(context != NULL, NULL);

    node<astNode> *operation = NULL;

    for (size_t operationIndex = 0; operationIndex < OperationsCount[priority]; operationIndex++) {
        operation = getKeyword(context, Operations[priority][operationIndex], compilationError::OPERATION_EXPECTED);

        if (!operation) {
            context->errorBuffer.currentIndex--;
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
static node<astNode> *getFunctionDefinition   (compilationContext *context, int     localNameTableId);
static node<astNode> *getDeclaration          (compilationContext *context, int     localNameTableId);
static node<astNode> *getOperator             (compilationContext *context, int     localNameTableId);
static node<astNode> *getInitializerDeclarator(compilationContext *context, int     localNameTableId);
static node<astNode> *getAssignmentExpression (compilationContext *context, int     localNameTableId);
static node<astNode> *getConditionOperator    (compilationContext *context, Keyword operatorKeyword, compilationError error, int localNameTableId);
static node<astNode> *getOperatorList         (compilationContext *context, int     localNameTableId);
static node<astNode> *getArgumentList         (compilationContext *context, int     localNameTableId);
static node<astNode> *getParameterList        (compilationContext *context, int     localNameTableId);
static node<astNode> *getReturnOperator       (compilationContext *context, int     localNameTableId);
static node<astNode> *getOutOperator          (compilationContext *context, int     localNameTableId);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

compilationError parseCode(compilationContext *context) {
    customWarning(context != NULL, compilationError::CONTEXT_ERROR);

    context->tokenIndex = 0;
    context->AST.root = getGrammar(context);

    return context->error;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static node<astNode> *getGrammar              (compilationContext *context) {
    customWarning(context != NULL, NULL);

    customWarning(getTokenAndDestroy(context, Keyword::INITIAL_OPERATOR, compilationError::INITIAL_OPERATOR_EXPECTED), NULL);

    node<astNode> *entryPointIdentifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    customWarning(entryPointIdentifier != NULL, NULL);

    customWarning(getTokenAndDestroy(context, Keyword::OPERATOR_SEPARATOR, compilationError::OPERATOR_SEPARATOR_EXPECTED), NULL);

    context->entryPoint = entryPointIdentifier->data.content.nameTableIndex;

    node<astNode> *rootNode = getTranslationUnit(context);
    customWarning(rootNode != NULL, NULL);

    DESTROY_CURRENT_NODE();

    int localNameTableID = 0;
    for (size_t callIndex = 0; callIndex < context->functionCalls.currentIndex; callIndex++) {
        DECLARATION_ASSERT(context->functionCalls.data[callIndex], localNameType::FUNCTION_IDENTIFIER, compilationError::FUNCTION_NOT_DECLARED);
    }

    destroySingleNode(entryPointIdentifier);

    return rootNode;
}

static node<astNode> *getTranslationUnit      (compilationContext *context) {
    customWarning(context != NULL, NULL);

    node<astNode> *externalDeclaration = getExternalDeclaration(context);
    customWarning(externalDeclaration != NULL, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING &&
                  context->nameTable.data[currentNameTableIndex].keyword == Keyword::OPERATOR_SERARATOR,
                  compilationError::OPERATOR_SEPARATOR_EXPECTED);

    currentToken->left          = externalDeclaration;
    externalDeclaration->parent = currentToken;

    node<astNode> *root = currentToken;

    ++context->tokenIndex;

    if (currentToken->data.type != nodeType::TERMINATOR) {
        root->right = getTranslationUnit(context);

        if (root->right) {
            root->right->parent = root;
        }
    }

    return root;
}

static node<astNode> *getExternalDeclaration  (compilationContext *context) {
    customWarning(context != NULL, NULL);

    node<astNode> *root = getFunctionDefinition(context, 0);

    if (root) {
        return root;
    }

    CHECK_FOR_ERROR(root, compilationError::FUNCTION_EXPECTED);

    root = getDeclaration(context, 0);

    return root;
}

static node<astNode> *getFunctionDefinition   (compilationContext *context, int     localNameTableId) {
    customWarning(context != NULL, NULL);

    customWarning(getTokenAndDestroy(context, Keyword::FUNCTION_DEFINITION, compilationError::FUNCTION_EXPECTED) != false, NULL);

    node<astNode> *type = getStringToken(context, nameType::TYPE_NAME, compilationError::TYPE_NAME_EXPECTED);
    customWarning(type != NULL, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    customWarning(identifier != NULL, NULL);

    size_t identifierIndex = identifier->data.nodeData.nameTableIndex;

    REDECLARATION_ASSERT(identifier,
                         (localNameType) ((int) localNameType::VARIABLE_IDENTIFIER | (int) localNameType::FUNCTION_IDENTIFIER),
                         compilationError::FUNCTION_REDEFINITION);

    --context->tokenIndex;
    DESTROY_CURRENT_NODE();
    ++context->tokenIndex;

    int newNameTableIndex = addLocalNameTable((int) identifierIndex, &context->localTables);
    addLocalIdentifier(0, &context->localTables, 
                       localNameTableElement {.nameType = localNameType::FUNCTION_IDENTIFIER, .globalNameID = identifierIndex}, 0);
    
    customWarning(getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED) != false, NULL);

    node<astNode> *parameters = getParameterList(context, newNameTableIndex);
    CHECK_FOR_ERROR(parameters, compilationError::TYPE_NAME_EXPECTED);

    customWarning(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED)    != false, NULL);
    customWarning(getTokenAndDestroy(context, Keyword::BLOCK_OPEN,    compilationError::CODE_BLOCK_EXPECTED) != false, NULL);

    node<astNode> *functionContent = getOperatorList(context, newNameTableIndex);

    CHECK_FOR_ERROR(functionContent, compilationError::OPERATOR_NOT_FOUND);

    customWarning(getTokenAndDestroy(context, Keyword::BLOCK_CLOSE, compilationError::CODE_BLOCK_EXPECTED) != false, NULL);

    return FUNCTION_DEFINITION_(type, PARAMETERS_(parameters, functionContent), identifierIndex);
}

static node<astNode> *getDeclaration          (compilationContext *context, int     localNameTableId) {
    customWarning(context != NULL, NULL);

    node<astNode> *type = getStringToken(context, nameType::TYPE_NAME, compilationError::TYPE_NAME_EXPECTED);
    customWarning(type != NULL, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    customWarning(identifier != NULL, NULL);

    context->tokenIndex--;

    size_t identifierIndex = identifier->data.nodeData.nameTableIndex;

    REDECLARATION_ASSERT(identifier,
                        (localNameType) ((int) localNameType::VARIABLE_IDENTIFIER | (int) localNameType::FUNCTION_IDENTIFIER),
                        compilationError::VARIABLE_REDECLARATION);

    addLocalIdentifier(localNameTableID, &context->localTables,
                       localNameTableElement {.nameType = localNameType::VARIABLE_IDENTIFIER, .globalNameID = identifierIndex}, 1);

    node<astNode> *initializerDeclarator = getInitializerDeclarator(context, localNameTableID);
    customWarning(initializerDeclarator != NULL, NULL);

    return VARIABLE_DECLARATION_(type, initializerDeclarator, identifierIndex);
}

static node<astNode> *getInitializerDeclarator(compilationContext *context, int     localNameTableId) {
    customWarning(context != NULL, NULL);

    node<astNode> *initializer = getAssignmentExpression(context, localNameTableID);

    if (initializer) {
        return initializer;
    }

    CHECK_FOR_ERROR(initializer, compilationError::ASSIGNMENT_EXPECTED);
    context->tokenIndex--;

    initializer = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);

    return initializer;
}

static node<astNode> *getAssignmentExpression (compilationContext *context, int     localNameTableId) {
    customWarning(context != NULL, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    customWarning(identifier != NULL, NULL);

    DECLARATION_ASSERT(identifier, localNameType::VARIABLE_IDENTIFIER, compilationError::VARIABLE_NOT_DECLARED);

    node<astNode> *assigmentOperation = getKeyword(context, Keyword::ASSIGNMENT, compilationError::ASSIGNMENT_EXPECTED);
    customWarning(assigmentOperation != NULL, NULL);

    node<astNode> *expression = getExpression(context, localNameTableID);
    customWarning(expression != NULL, NULL);

    assigmentOperation->left  = expression;
    expression->parent        = assigmentOperation;

    assigmentOperation->right = identifier;
    identifier->parent        = assigmentOperation;

    return assigmentOperation;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static node<astNode> *getOperator             (compilationContext *context, int     localNameTableId) {
    customWarning(context != NULL, NULL);

    node<astNode> *expectedOperator = NULL;

    expectedOperator = getConditionOperator(context, Keyword::IF, compilationError::IF_EXPECTED, localNameTableID);
    CHECK_FOR_ERROR(expectedOperator, compilationError::IF_EXPECTED);

    if (expectedOperator) {
        return OPERATOR_SERARATOR_(expectedOperator, NULL);
    }

    expectedOperator = getConditionOperator(context, Keyword::WHILE, compilationError::WHILE_EXPECTED, localNameTableID);
    CHECK_FOR_ERROR(expectedOperator, compilationError::WHILE_EXPECTED);

    if (expectedOperator) {
        return OPERATOR_SEPARATOR_(expectedOperator, NULL);
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
        GET_EXPECTED_OPERATOR(gerFunctionCall,         FUNCTION_CALL_EXPECTED);
        GET_EXPECTED_OPERATOR(getAssignmentExpression, IDENTIFIER_EXPECTED);
        GET_EXPECTED_OPERATOR(getDeclaration,          TYPE_NAME_EXPECTED);

        customWarning(getTokenAndDestroy(context, Keyword::BLOCK_OPEN, compilationError::OPERATOR_NOT_FOUND) != false, NULL);
        expectedOperator = getOperatorList(context, localNameTableID);
        customWarning(expectedOperator != NULL, NULL);
        customWarning(getTokenAndDestroy(context, Keyword::BLOCK_CLOSE, compilationError::OPERATOR_NOT_FOUND) != false, NULL);
    } while (0)

    node<astNode> *separator = getKeyword(context, Keyword::OPERATOR_SEPARATOR, compilationError::OPERATOR_SEPARATOR_EXPECTED);
    customWarning(separator != NULL, NULL);

    separator->left          = expectedOperator;
    expectedOperator->parent = separator;

    return separator;
}

static node<astNode> *getOperatorList         (compilationContext *context, int     localNameTableId){
    customWarning(context != NULL, NULL);

    node<astNode> *firstOperator = getOperator(context, localNameTableID);
    customWarning(firstOperator != NULL, NULL);

    node<astNode> *secondOperator = getOperatorList(context, localNameTableID);

    firstOperator->right = secondOperator;

    CHECK_FOR_ERROR(secondOperator, compilationError::OPERATOR_NOT_FOUND);

    if (secondOperator) {
        secondOperator->parent = firstOperator;
    }

    return firstOperator;
}

static node<astNode> *getConditionOperator    (compilationContext *context, Keyword operatorKeyword, compilationError error, int localNameTableId) {
    customWarning(context != NULL, NULL);

    node<astNode> *conditionOperator = getKeyword(context, operatorKeyword, error);
    customWarning(conditionOperator != NULL, NULL);

    node<astNode> *conditionExpression = getExpression(context, localNameTableID);
    customWarning(conditionExpression != NULL, NULL);

    customWarning(getTokenAndDestroy(context, Keyword::CONDITION_SEPARATOR, compilationError::CONDITION_SEPARATOR_EXPECTED));

    node<astNode> *operatorContent = getOperator(context, localNameTableID);
    customWarning(operatorContent);

    conditionOperator->left     = conditionExpression;
    conditionExpression->parent = conditionOperator;

    conditionOperator->right    = operatorContent;
    operatorContent->parent     = conditionOperator;

    return conditionOperator;
}

static node<astNode> *getReturnOperator       (compilationContext *context, int     localNameTableId){
    customWarning(context != NULL, NULL);

    node<astNode> *returnStatement = getKeyword(context, Keyword::RETURN_OPERATOR, compilationError::RETURN_EXPECTED);
    customWarning(returnStatement != NULL, NULL);

    node<astNode> *expression = getExpression(context, localNameTableID);
    customWarning(expression != NULL, NULL);

    returnStatement->right = expression;
    expression->parent     = returnStatement;

    return returnStatement;
}

static node<astNode> *getOutOperator          (compilationContext *context, int     localNameTableId){
    customWarning(context != NULL, NULL);

    node<astNode> *outOperator = getKeyword(context, Keyword::OUT, compilationError::OUT_EXPECTED);
    customWarning(outOperator);

    node<astNode> *expression = getExpression(context, localNameTableID);
    customWarning(expression != NULL, NULL);

    outOperator->right = expression;
    expression->parent = outOperator;

    return outOperator;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getFunctionCall(compilationContext *context, int localNameTableID) {
    customWarning(context != NULL, NULL);

    customWarning(getTokenAndDestroy(context, Keyword::FUNCTION_CALL, compilationError::FUNCTION_CALL_EXPECTED) != false, NULL);

    node<astNode> *identifier = getStringToken(context, nameType::IDENTIFIER, compilationError::IDENTIFIER_EXPECTED);
    customWarning(identifier);
    writeDataToBuffer(&context->functionCalls, &identifier, 1);

    customWarning(getTokenAndDestroy(context, Keyword::LEFT_BRACKET, compilationError::BRACKET_EXPECTED) != false, NULL);

    node<astNode> *arguments = getArgumentList(context, localNameTableID);
    CHECK_FOR_ERROR(arguments, compilationError::CONSTANT_EXPECTED);

    customWarning(getTokenAndDestroy(context, Keyword::RIGHT_BRACKET, compilationError::BRACKET_EXPECTED) != false, NULL);

    return FUNCTION_CALL_(arguments, identifier);
}

static node<astNode> *getArgumentList         (compilationContext *context, int     localNameTableId){
    customWarning(context != NULL, NULL);

    node<astNode> *argument = getExpression(context, localNameTableID);
    customWarning(argument != NULL, NULL);

    node<astNode> *separator = getKeyword(context, Keyword::ARGUMENT_SEPARATOR, compilationError::ARGUMENT_SEPARATOR_EXPECTED);

    if (!separator) {
        context->errorBuffer.currentIndex--;
        return ARGUMENT_SEPARATOR_(argument, NULL);
    }

    node<astNode> *nextArgument = getArgumentList(context, localNameTableID);
    customWarning(nextArgument != NULL, NULL);

    separator->left      = argument;
    argument->parent     = separator;

    separator->right     = nextArgument;
    nextArgument->parent = separator;

    return separator;
}

static node<astNode> *getParameterList        (compilationContext *context, int     localNameTableId){
    customWarning(context != NULL, NULL);

    node<astNode> *parameter = getDeclaration(context, localNameTableID);
    customWarning(parameter != NULL, NULL);

    node<astNode> *separator = getKeyword(context, Keyword::Argument_SEPARATOR, compilationError::ARGUMENT_SEPARATOR_EXPECTED);

    if (!separator) {
        context->errorBuffer.currentIndex--;
        return ARGUMENT_SEPARATOR_(parameter, NULL);
    }

    node<astNode> *nextParameter = getParameterList(context, localNameTableID);
    customWarning(nextParameter != NULL, NULL);

    separator->left       = parameter;
    parameter->parent     = separator;

    separator->right      = nextParameter;
    nextParameter->parent = separator;

    return separator;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

node<astNode> *getStringToken(compilationContext *context, nameType type, compilationError error) {
    customWarning(context != NULL, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING && context->nameTable.data[currentNameTableIndex].type == type, error);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

node<astNode> *getKeyword(compilationContext *context, Keyword keyword, compilationError error) {
    customWarning(context != NULL, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::STRING && context->nameTable.data[currentNameTableIndex].keyword == keyword, error);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

node<astNode> *getConstant(compilationContext *context) {
    customWarning(context != NULL, NULL);

    SYNTAX_ASSERT(currentToken->data.type == nodeType::CONSTANT, compilationError::CONSTANT_EXPECTED);

    node<astNode> *returnValue = currentToken;
    context->tokenIndex++;

    return returnValue;
}

bool getTokenAndDestroy(compilationContext *context, Keyword keyword, compilationError error) {
    customWarning(context != NULL, false);

    if (!getKeyword(context, keyword, error)) {
        return false;
    }

    context->tokenIndex--;
    DESTROY_CURRENT_NODE();
    context->tokenIndex++;

    return true;
}

bool isIdentifierDeclared(compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType) {
    customWarning(context != NULL, false);

    if (isLocalIdentifierDeclared(context, localNameTableID, identifierIndex, identifierType)) {
        return true;
    }

    int nameIndex = getIndexInLocalTable(0, &context->localTables, identifierIndex, identifierType);

    if (nameIndex >= 0) {
        retrun true;
    }

    return false;
}

bool isLocalIdentifierDeclared(compilationContext *context, int localNameTableID, size_t identifierIndex, localNameType identifierType) {
    customWarning(context != NULL, false);

    int localNameIndex = getIndexInLocalTable(localNameTableID, &context->localTables, identifierIndex, identifierType);

    if (localNameIndex >= 0) {
        return true;
    }

    return false;
}

