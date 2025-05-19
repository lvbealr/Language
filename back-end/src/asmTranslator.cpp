#include "asmTranslator.h"
#include "buffer.h"
#include "core.h"
#include "nameTable.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define WRITE(STRING) do {                    \
    writeStringToBuffer(outputData, STRING);  \
} while (0)

#define BLOCK(BLOCK_INDEX, BLOCK_NAME, BLOCK_SOURCE) do {                                   \
    writeBlockStatement(context, outputData, codeBlock                                      \
        {.blockID = BLOCK_INDEX, .blockName = BLOCK_NAME, .blockSource = BLOCK_SOURCE});    \
} while (0)

#define LABEL(NAME, INDEX) writeLabel(context, outputData, NAME, INDEX)
#define NEW_LABEL(NAME, INDEX) LABEL(NAME, INDEX)

#define UNARY_OPERATION(OPERATION) traverseAST(context, node->left, outputData, nameTableIndex); \
                                   WRITE("\t" OPERATION "\n");

#define BINARY_OPERATION(OPERATION)  traverseAST(context, node->left, outputData, nameTableIndex);  \
                                     traverseAST(context, node->right, outputData, nameTableIndex); \
                                     WRITE("\t" OPERATION "\n");

#define MEMORY(NODE) pointMemoryCell(context, NODE, outputData, nameTableIndex);

#define JUMP(JUMP_TYPE) traverseAST(context, node->left, outputData, nameTableIndex);   \
                        traverseAST(context, node->right, outputData, nameTableIndex);  \
                        LOGIC_JUMP(JUMP_TYPE);

#define LOGIC_JUMP(JUMP_TYPE) context->counters->logicCount++;                      \
                        WRITE("\t" JUMP_TYPE "\n");                                 \
                        LABEL("TRUE BRANCH", context->counters->logicCount);        \
                        WRITE("\n\tpush 0\n");                                      \
                        WRITE("\tjmp ");                                            \
                        LABEL("JUMP END", context->counters->logicCount);           \
                        WRITE("\n");                                                \
                        NEW_LABEL("TRUE BRANCH", context->counters->logicCount);    \
                        WRITE("\tpush 1\n");                                        \
                        NEW_LABEL("JUMP END", context->counters->logicCount);       \

#define LOGIC_EXPRESSION(NODE)  traverseAST(context, NODE, outputData, nameTableIndex); \
                                WRITE("\tpush 0\n");                                    \
                                LOGIC_JUMP("jne");                                      \

#define LOGIC_OPERATION(OPERATION)  LOGIC_EXPRESSION(node->left);   \
                                    LOGIC_EXPRESSION(node->right);  \
                                    WRITE("\t" OPERATION "\n");

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static translationError writeConstant(Buffer<char> *outputData, int number);

static translationError writeIdentifier     (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError pointMemoryCell     (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeKeyword        (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeFunction       (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeFunctionCall   (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeVariable       (translationContext *context, node<astNode> *node,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeLabel          (translationContext *context, Buffer<char> *outputData, const char   *labelName,  size_t labelIndex);

static translationError writeBlockStatement (translationContext *context, Buffer<char> *outputData, codeBlock block);


// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError translateToASM(translationContext *context, const char *fileName) {
    customWarning(context,  translationError::CONTEXT_BAD_POINTER);
    customWarning(fileName, translationError::BAD_FILE_NAME);

    Buffer<char> outputData = {};
    bufferInitialize(&outputData);

    initializeProgram(context, &outputData);
    traverseAST(context, context->AST->root, &outputData, 0);

    FILE *outputFile = fopen(fileName, "w");
    customWarning(outputFile, translationError::FILE_OPEN_ERROR);

    fwrite(outputData.data, sizeof(char), outputData.currentIndex, outputFile);
    fclose(outputFile);

    bufferDestruct(&outputData);

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

translationError initializeProgram(translationContext *context, Buffer<char> *outputData) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    writeStringToBuffer(outputData, "section .text\n");
    
    char *entryPointName = context->nameTable->data[context->entryPoint].name;

    writeStringToBuffer(outputData, "global ");
    writeStringToBuffer(outputData, entryPointName);
    writeStringToBuffer(outputData, "\n");

    return translationError::NO_ERRORS;
}

translationError traverseAST(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    if (!node) {
        return translationError::NO_ERRORS;
    }

    #define writeElement(TYPE, FUNCTION, ...) {                     \
        __VA_ARGS__;                                                \
                                                                    \
        case (TYPE): {                                              \
            FUNCTION(context, node, outputData, nameTableIndex);    \
            break;                                                  \
        }                                                           \
    }

    switch (node->data.type) {
        case nodeType::TERMINATOR: {
            return translationError::AST_BAD_STRUCTURE;
        }

        case nodeType::CONSTANT: {
            writeConstant(outputData, node->data.data.number);
            break;
        }

        case nodeType::PARAMETERS: {
            context->callParameters = false;

            traverseAST(context, node->left, outputData, nameTableIndex);
            traverseAST(context, node->right, outputData, nameTableIndex);

            break;
        }

        writeElement(nodeType::STRING,               writeIdentifier  );
        writeElement(nodeType::KEYWORD,              writeKeyword     );
        writeElement(nodeType::FUNCTION_DEFINITION,  writeFunction    );
        writeElement(nodeType::VARIABLE_DECLARATION, writeVariable    );
        writeElement(nodeType::FUNCTION_CALL,        writeFunctionCall);

    }

    #undef writeElement

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static translationError writeConstant(Buffer<char> *outputData, int number) {
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    char numberString[MAX_NUMBER_LENGTH] = {};
    snprintf(numberString, MAX_NUMBER_LENGTH, "%d", number);

    WRITE("\tpush ");
    WRITE(numberString);
    WRITE("\n");

    return translationError::NO_ERRORS;
}

static translationError writeIdentifier(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    char *name = context->nameTable->data[node->data.data.nameTableIndex].name;

    WRITE("\tpush ");
    customWarning(node, translationError::AST_BAD_POINTER);
    pointMemoryCell(context, node, outputData, nameTableIndex);

    return translationError::NO_ERRORS;
}

static translationError pointMemoryCell(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    char indexBuffer[MAX_NUMBER_LENGTH] = {};
    int  identifierIndex                = -1;

    if (!nameTableIndex) {
        identifierIndex = getIndexInLocalTable(nameTableIndex, context->localTables, node->data.data.nameTableIndex, localNameType::VARIABLE_IDENTIFIER);

        if (identifierIndex >= 0) {
            snprintf(indexBuffer, MAX_NUMBER_LENGTH, "%d", identifierIndex);

            WRITE("[rbp + ");
            WRITE(indexBuffer);
            WRITE("]\n");

            return translationError::NO_ERRORS;
        }
    }

    identifierIndex = getIndexInLocalTable(0, context->localTables, node->data.data.nameTableIndex, localNameType::VARIABLE_IDENTIFIER);

    if (identifierIndex < 0) {
        return translationError::AST_BAD_STRUCTURE;
    }

    identifierIndex += context->initialAddress;
    snprintf(indexBuffer, MAX_NUMBER_LENGTH, "%d", identifierIndex);

    WRITE("[");
    WRITE(indexBuffer);
    WRITE("]\n");

    return translationError::NO_ERRORS;
}

static translationError writeKeyword(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    #define OPERATOR(KEYWORD, ...) {    \
        case (KEYWORD): {               \
            __VA_ARGS__;                \
            break;                      \
        }                               \
    }

    switch (node->data.data.keyword) {
        OPERATOR(Keyword::IF, {
            size_t blockID = ++context->counters->ifCount;
            BLOCK(blockID, "IF Condition", "IF Statement");
            traverseAST(context, node->left, outputData, nameTableIndex);
            WRITE("\tpush 0\n");
            WRITE("\tje ");
            
            LABEL("IF_END", blockID);
            WRITE("\n");

            BLOCK(blockID, "IF Body", "IF Statement");

            traverseAST(context, node->right, outputData, nameTableIndex);

            NEW_LABEL("IF_END", blockID);
        })

        OPERATOR(Keyword::WHILE, {
            size_t blockID = ++context->counters->whileCount;
            
            BLOCK(blockID, "WHILE Condition", "WHILE Cycle");
            NEW_LABEL("WHILE_BEGIN", blockID);
            traverseAST(context, node->left, outputData, nameTableIndex);
            WRITE("\tpush 0\n");
            WRITE("\tje ");
            LABEL("WHILE_END", blockID);
            WRITE("\n");

            BLOCK(blockID, "WHILE Body", "WHILE Cycle");
            traverseAST(context, node->right, outputData, nameTableIndex);

            BLOCK(blockID, "WHILE End", "WHILE Cycle");
            WRITE("\tjmp ");
            LABEL("WHILE_BEGIN", blockID);
            WRITE("\n");
            NEW_LABEL("WHILE_END", blockID);
        })

        OPERATOR(Keyword::ASSIGNMENT, {
            size_t blockID = ++context->counters->assignmentCount;
            BLOCK(blockID, "ASSIGNMENT Expression", "ASSIGNMENT Operation");
            traverseAST(context, node->left, outputData, nameTableIndex);
            BLOCK(blockID, "ASSIGNMENT Cell", "ASSIGNMENT Operation");
            WRITE("\tpop ");
            MEMORY(node->right);
        })

        OPERATOR(Keyword::SIN,      UNARY_OPERATION  ("sin");)
        OPERATOR(Keyword::COS,      UNARY_OPERATION  ("cos");)
        OPERATOR(Keyword::FLOOR,    UNARY_OPERATION("floor");)
        OPERATOR(Keyword::SQRT,     UNARY_OPERATION ("sqrt");)

        OPERATOR(Keyword::ADD,      BINARY_OPERATION("add");)
        OPERATOR(Keyword::SUB,      BINARY_OPERATION("sub");)
        OPERATOR(Keyword::MUL,      BINARY_OPERATION("mul");)
        OPERATOR(Keyword::DIV,      BINARY_OPERATION("div");)

        OPERATOR(Keyword::EQUAL,            JUMP ("je");)
        OPERATOR(Keyword::LESS,             JUMP ("jb");)
        OPERATOR(Keyword::GREATER,          JUMP ("ja");)
        OPERATOR(Keyword::LESS_OR_EQUAL,    JUMP("jbe");)
        OPERATOR(Keyword::GREATER_OR_EQUAL, JUMP("jae");)
        OPERATOR(Keyword::NOT_EQUAL,        JUMP("jne");)

        OPERATOR(Keyword::AND, LOGIC_OPERATION("mul");)
        OPERATOR(Keyword::OR,  LOGIC_OPERATION("add");)

        OPERATOR(Keyword::NOT, 
            LOGIC_EXPRESSION(node->left); 
            LOGIC_JUMP("je");)

        OPERATOR(Keyword::ABORT, WRITE("\thlt\n");)

        OPERATOR(Keyword::RETURN, 
            traverseAST(context, node->left, outputData, nameTableIndex); 
            WRITE("\tpop rax\n\tret\n");
        )
    
        OPERATOR(Keyword::BREAK, 
            WRITE("\tjmp "); 
            LABEL("WHILE_END", context->counters->whileCount); 
            WRITE("\n");
        )

        OPERATOR(Keyword::CONTINUE, 
            WRITE("\tjmp "); 
            LABEL("WHILE_BEGIN", context->counters->whileCount); 
            WRITE("\n");
        )

        OPERATOR(Keyword::IN, WRITE("\tin\n");)

        OPERATOR(Keyword::OUT, 
            traverseAST(context, node->right, outputData, nameTableIndex); 
            WRITE("\tout\n");
        )

        OPERATOR(Keyword::OPERATOR_SEPARATOR, 
            traverseAST(context, node->left, outputData, nameTableIndex); 
            traverseAST(context, node->right, outputData, nameTableIndex);
        )

        OPERATOR(Keyword::ARGUMENT_SEPARATOR, 
            if (context->callParameters) {
                traverseAST(context, node->left, outputData, nameTableIndex);
                traverseAST(context, node->right, outputData, nameTableIndex);
            } else {
                traverseAST(context, node->right, outputData, nameTableIndex);
                traverseAST(context, node->left, outputData, nameTableIndex);

                if (node->left) {
                    WRITE("\tpop ");
                    MEMORY(node->left);
                }
            }
        )

        default:
            return translationError::AST_BAD_STRUCTURE;
    }

    #undef OPERATOR

    return translationError::NO_ERRORS;
}

static translationError writeFunction(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    int tableIndex = getLocalNameTableIndex(node->data.data.nameTableIndex, context->localTables);

    BLOCK((size_t) tableIndex, "FUNCTION Label", "FUNCTION Definition");
    customPrint(red, bold, bgDefault, "%p\n", context->nameTable->data[node->data.data.nameTableIndex].name);
    WRITE(context->nameTable->data[node->data.data.nameTableIndex].name);
    WRITE(":\n");

    if (tableIndex < 0) {
        return translationError::NAME_TABLE_ERROR;
    }

    if (node->right) {
        traverseAST(context, node->right, outputData, tableIndex);
    } else {
        return translationError::AST_BAD_STRUCTURE;
    }

    return translationError::NO_ERRORS;
}

static translationError writeFunctionCall(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    size_t blockID = ++context->counters->callCount;

    if (!node->right) {
        return translationError::AST_BAD_STRUCTURE;
    }

    context->callParameters = true;

    BLOCK(blockID, "STACK FRAME SAVING", "CALL OPERATOR");
    WRITE("\tpush rbp\n");

    BLOCK(blockID, "ARGUMENTS", "FUNCTION Call");

    if (node->left) {
        traverseAST(context, node->left, outputData, nameTableIndex);
    }

    char stackFrameSizeString[MAX_NUMBER_LENGTH] = {};
    snprintf(stackFrameSizeString, MAX_NUMBER_LENGTH, "%lu", context->localTables->data[nameTableIndex].size);

    BLOCK(blockID, "STACK FRAME CHANGING", "CALL OPERATOR");
    WRITE("\tpush rbp +");
    WRITE(stackFrameSizeString);
    WRITE("\n\tpop rbp\n");

    BLOCK(blockID, "FUNCTION CALL", "CALL OPERATOR");
    WRITE("\tcall ");
    WRITE(context->nameTable->data[node->right->data.data.nameTableIndex].name);
    WRITE("\n\tpop rbp\n");

    if (node->parent) {
        if (!(node->parent->data.type == nodeType::KEYWORD && node->parent->data.data.keyword == Keyword::OPERATOR_SEPARATOR)) {
            WRITE("\tpush rax\n");
        }
    }

    return translationError::NO_ERRORS;
}

static translationError writeVariable(translationContext *context, node<astNode> *node, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(node,       translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    context->localTables->data[nameTableIndex].size += 1;

    if (node->right) {
        if (node->right->data.type == nodeType::KEYWORD) {
            traverseAST(context, node->right, outputData, nameTableIndex);
        }
    }

    return translationError::NO_ERRORS;
}

static translationError writeLabel(translationContext *context, Buffer<char> *outputData, const char *labelName, size_t labelIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    char labelIDString[MAX_NUMBER_LENGTH] = {};
    snprintf(labelIDString, MAX_NUMBER_LENGTH, "%lu", labelIndex);

    WRITE(labelName);
    WRITE("_");
    WRITE(labelIDString);
    WRITE(":\n");

    return translationError::NO_ERRORS;
}

static translationError writeBlockStatement(translationContext *context, Buffer<char> *outputData, codeBlock block) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);
    
    char blockIDString[MAX_NUMBER_LENGTH] = {};
    snprintf(blockIDString, MAX_NUMBER_LENGTH, "%lu", block.blockID);

    // WRITE("\n; // --------------------  BLOCK NAME: ");
    // WRITE(block.blockName);
    // WRITE(", SRC: ");
    // WRITE(block.blockSource);
    // WRITE("\t(# ");
    // WRITE(blockIDString);
    // WRITE(") -------------------- //\n");

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
