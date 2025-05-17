#ifndef CORE_H_
#define CORE_H_

#include "nameTable.h"
#include "buffer.h"
#include "AST.h"

enum class compilationError: long long int {
    NO_ERRORS                    = 0,
    CONTEXT_ERROR                = 1 << 0,
    TOKEN_BUFFER_ERROR           = 1 << 1,
    FUNCTION_CALL_EXPECTED       = 1 << 2,
    IDENTIFIER_EXPECTED          = 1 << 3,
    INITIAL_OPERATOR_EXPECTED    = 1 << 4,
    OPERATOR_SEPARATOR_EXPECTED  = 1 << 5,
    TYPE_NAME_EXPECTED           = 1 << 6,
    BRACKET_EXPECTED             = 1 << 7,
    CODE_BLOCK_EXPECTED          = 1 << 8,
    FUNCTION_EXPECTED            = 1 << 9,
    ASSIGNMENT_EXPECTED          = 1 << 10,
    CONDITION_SEPARATOR_EXPECTED = 1 << 11,
    IF_EXPECTED                  = 1 << 12,
    WHILE_EXPECTED               = 1 << 13,
    OPERATION_EXPECTED           = 1 << 14,
    CONSTANT_EXPECTED            = 1 << 15,
    ARGUMENT_SEPARATOR_EXPECTED  = 1 << 16,
    DUMP_ERROR                   = 1 << 17,
    HTML_ERROR                   = 1 << 18,
    OUTPUT_FILE_ERROR            = 1 << 19,
    RETURN_EXPECTED              = 1 << 20,
    BREAK_EXPECTED               = 1 << 21,
    CONTINUE_EXPECTED            = 1 << 22,
    IN_EXPECTED                  = 1 << 23,
    OUT_EXPECTED                 = 1 << 24,
    ABORT_EXPECTED               = 1 << 25,
    FUNCTION_REDEFINITION        = 1 << 26,
    VARIABLE_REDECLARATION       = 1 << 27,
    FUNCTION_NOT_DECLARED        = 1 << 28,
    VARIABLE_NOT_DECLARED        = 1 << 29,
    OPERATOR_NOT_FOUND           = 1 << 30,
};

struct errorData {
    compilationError error = compilationError::NO_ERRORS;
    int              line  = 0;
    char            *file  = NULL;
};

struct compilationContext {
    Buffer<nameTableElement> *nameTable   = {};
    Buffer<localNameTable>   *localTables = {};
    Buffer<node<astNode> *>  *tokens      = {};

    size_t tokenIndex  = 0;

    int    currentLine = 0;

    compilationError   error       = compilationError::NO_ERRORS;
    Buffer<errorData> *errorBuffer = {};

    binaryTree<astNode> *AST = {};

    char  *fileContent = NULL;
    size_t fileSize    = 0;

    size_t entryPoint  = 0; // IR?

    Buffer<node<astNode> *> *functionCalls = {}; // IR?
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

compilationError compilationContextInitialize(compilationContext *context, char *fileContent);
compilationError compilationContextDestruct  (compilationContext *context);

compilationError dumpTokenTable(compilationContext *context);
compilationError dumpToken     (compilationContext *context, node<astNode> *token);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // CORE_H_