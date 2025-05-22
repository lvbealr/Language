#include <vector>

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
#define NEW_LABEL(NAME, INDEX) LABEL(NAME, INDEX); WRITE(":\n")

#define BINARY_OPERATION(OPERATION) do {                                    \
    traverseAST(context, currentNode->left, outputData, nameTableIndex);    \
    WRITE("\tmov rbx, rax\n");                                              \
    traverseAST(context, currentNode->right, outputData, nameTableIndex);   \
    WRITE("\t" OPERATION " rax, rbx\n");                                    \
    if (!(currentNode->parent && currentNode->parent->data.type == nodeType::KEYWORD \
        && currentNode->parent->data.data.keyword == Keyword::ASSIGNMENT)) {\
        WRITE("\tpush rax\n");                                              \
    }                                                                       \
} while (0)

#define UNARY_OPERATION(OPERATION) do {                                     \
    traverseAST(context, currentNode->left, outputData, nameTableIndex);    \
    WRITE("\t" OPERATION " rax\n");                                         \
    WRITE("\tpush rax\n");                                                  \
} while (0)

#define MEMORY(NODE) pointMemoryCell(context, NODE, outputData, nameTableIndex);

#define JUMP(JUMP_TYPE) do {                                                \
    traverseAST(context, currentNode->left, outputData, nameTableIndex);    \
    traverseAST(context, currentNode->right, outputData, nameTableIndex);   \
    WRITE("\tpop rbx\n");                                                   \
    WRITE("\tpop rax\n");                                                   \
    WRITE("\tcmp rax, rbx\n");                                              \
    LOGIC_JUMP(JUMP_TYPE);                                                  \
} while (0)

#define LOGIC_JUMP(JUMP_TYPE) do {                                          \
    context->counters->logicCount++;                                        \
    WRITE("\t" JUMP_TYPE " ");                                              \
    LABEL("TRUE_BRANCH", context->counters->logicCount);                    \
    WRITE("\n\tpush 0\n");                                                  \
    WRITE("\tjmp ");                                                        \
    LABEL("JUMP_END", context->counters->logicCount);                       \
    WRITE("\n");                                                            \
    NEW_LABEL("TRUE_BRANCH", context->counters->logicCount);                \
    WRITE("\n\tpush 1\n");                                                  \
    NEW_LABEL("JUMP_END", context->counters->logicCount);                   \
    WRITE("\n");                                                            \
} while (0)

#define LOGIC_EXPRESSION(NODE) do {                                         \
    traverseAST(context, NODE, outputData, nameTableIndex);                 \
    WRITE("\tpop rax\n");                                                   \
    WRITE("\tcmp rax, 0\n");                                                \
    LOGIC_JUMP("jne");                                                      \
} while (0)

#define LOGIC_OPERATION(OPERATION) do {                                     \
    LOGIC_EXPRESSION(currentNode->left);                                    \
    LOGIC_EXPRESSION(currentNode->right);                                   \
    WRITE("\tpop rbx\n");                                                   \
    WRITE("\tpop rax\n");                                                   \
    WRITE("\t" OPERATION " rax, rbx\n");                                    \
    WRITE("\tpush rax\n");                                                  \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static translationError writeConstant(Buffer<char> *outputData, int number);

static translationError writeIdentifier     (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError pointMemoryCell     (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeKeyword        (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeFunction       (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeFunctionCall   (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
static translationError writeVariable       (translationContext *context, node<astNode> *currentNode,      Buffer<char> *outputData, size_t nameTableIndex);
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
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    writeStringToBuffer(outputData, "section .text\n");
    writeStringToBuffer(outputData, "global main\n");
    return translationError::NO_ERRORS;
}

translationError traverseAST(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    if (!currentNode) {
        return translationError::NO_ERRORS;
    }

    switch (currentNode->data.type) {
        case nodeType::TERMINATOR: {
            return translationError::AST_BAD_STRUCTURE;
        }

        case nodeType::CONSTANT: {
            writeConstant(outputData, currentNode->data.data.number);
            break;
        }

        case nodeType::PARAMETERS: {
            context->callParameters = false;

            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            traverseAST(context, currentNode->right, outputData, nameTableIndex);

            break;
        }

        case nodeType::STRING: {
            writeIdentifier(context, currentNode, outputData, nameTableIndex);
            break;
        }

        case nodeType::KEYWORD: {
            writeKeyword(context, currentNode, outputData, nameTableIndex);
            break;
        }

        case nodeType::FUNCTION_DEFINITION: {
            writeFunction(context, currentNode, outputData, nameTableIndex);
            break;
        }

        case nodeType::VARIABLE_DECLARATION: {
            writeVariable(context, currentNode, outputData, nameTableIndex);
            break;
        }

        case nodeType::FUNCTION_CALL: {
            writeFunctionCall(context, currentNode, outputData, nameTableIndex);
            break;
        }

        default:
            return translationError::AST_BAD_STRUCTURE;
    }

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static translationError writeConstant(Buffer<char> *outputData, int number) {
    char numberString[MAX_NUMBER_LENGTH] = {};
    snprintf(numberString, MAX_NUMBER_LENGTH, "%d", number);
    
    WRITE("\tmov rax, ");
    WRITE(numberString);
    WRITE("\n");
    
    return translationError::NO_ERRORS;
}

static translationError writeIdentifier(translationContext *context, 
                                      node<astNode> *currentNode, 
                                      Buffer<char> *outputData, 
                                      size_t nameTableIndex) {
    size_t identifierIndex = currentNode->data.data.nameTableIndex;
    int64_t offset = context->nameTable->data[identifierIndex].rbpOffset;
    
    WRITE("\tmov rax, qword [rbp - ");

    char offsetStr[MAX_NUMBER_LENGTH] = {};
    snprintf(offsetStr, MAX_NUMBER_LENGTH, "%ld", -offset);
    
    WRITE(offsetStr);
    WRITE("]\n");
    
    return translationError::NO_ERRORS;
}

static translationError pointMemoryCell(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    char indexBuffer[MAX_NUMBER_LENGTH] = {};

    int64_t rbpOffset = currentNode->data.type == nodeType::STRING ?
        context->nameTable->data[currentNode->data.data.nameTableIndex].rbpOffset :
        currentNode->data.rbpOffset;

    if (rbpOffset == 0) {
        return translationError::AST_BAD_STRUCTURE;
    }

    if (rbpOffset < 0) {
        snprintf(indexBuffer, MAX_NUMBER_LENGTH, "%lu", -rbpOffset);
        WRITE("[rbp - ");
        WRITE(indexBuffer);
        WRITE("]");
    } else {
        snprintf(indexBuffer, MAX_NUMBER_LENGTH, "%lu", rbpOffset);
        WRITE("[rbp + ");
        WRITE(indexBuffer);
        WRITE("]");
    }

    return translationError::NO_ERRORS;
}

static translationError writeKeyword(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(currentNode, translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    switch (currentNode->data.data.keyword) {
        case Keyword::WHILE: {
            size_t blockID = ++context->counters->whileCount;
            NEW_LABEL("WHILE_BEGIN", blockID);
            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            WRITE("\tpop rax\n");
            WRITE("\tcmp rax, 0\n");
            WRITE("\tje ");
            LABEL("WHILE_END", blockID);
            WRITE("\n");
            traverseAST(context, currentNode->right, outputData, nameTableIndex);
            WRITE("\tjmp ");
            LABEL("WHILE_BEGIN", blockID);
            WRITE("\n");
            NEW_LABEL("WHILE_END", blockID);
            WRITE("\n");
            break;
        }

        case Keyword::IF: {
            size_t blockID = ++context->counters->ifCount;
            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            WRITE("\tpop rax\n");
            WRITE("\tcmp rax, 0\n");
            WRITE("\tje ");
            LABEL("IF_END", blockID);
            WRITE("\n");
            traverseAST(context, currentNode->right, outputData, nameTableIndex);
            NEW_LABEL("IF_END", blockID);
            WRITE("\n");
            break;
        }

        case Keyword::ASSIGNMENT: {
            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            WRITE("\tmov qword ");
            MEMORY(currentNode->right);
            WRITE(", rax\n");
            break;
        }

        case Keyword::SIN:
            UNARY_OPERATION("sin");
            break;
        case Keyword::COS:
            UNARY_OPERATION("cos");
            break;
        case Keyword::FLOOR:
            UNARY_OPERATION("floor");
            break;
        case Keyword::SQRT:
            UNARY_OPERATION("sqrt");
            break;

        case Keyword::ADD:
            BINARY_OPERATION("add");
            break;

        case Keyword::SUB: {
            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            WRITE("\tmov rbx, rax\n");
            traverseAST(context, currentNode->right, outputData, nameTableIndex);
            WRITE("\tsub rbx, rax\n");
            WRITE("\tmov rax, rbx\n");
                
            if (currentNode->parent && currentNode->parent->data.type == nodeType::KEYWORD &&
                currentNode->parent->data.data.keyword == Keyword::ASSIGNMENT) {
                WRITE("\tmov qword ");
                MEMORY(currentNode->parent->right);
                WRITE(", rax\n");
            } else {
                WRITE("\tpush rax\n");
            }

            break;
        }

        case Keyword::MUL:
            BINARY_OPERATION("imul");
            break;
        case Keyword::DIV:
            BINARY_OPERATION("idiv");
            break;

        case Keyword::EQUAL:
            JUMP("je");
            break;
        case Keyword::LESS:
            JUMP("jl");
            break;
        case Keyword::GREATER:
            JUMP("jg");
            break;
        case Keyword::LESS_OR_EQUAL:
            JUMP("jle");
            break;
        case Keyword::GREATER_OR_EQUAL:
            JUMP("jge");
            break;
        case Keyword::NOT_EQUAL:
            JUMP("jne");
            break;

        case Keyword::AND:
            LOGIC_OPERATION("and");
            break;
        case Keyword::OR:
            LOGIC_OPERATION("or");
            break;

        case Keyword::NOT:
            LOGIC_EXPRESSION(currentNode->left);
            break;

        case Keyword::ABORT:
            WRITE("\thlt\n");
            break;

        case Keyword::RETURN: {
            if (currentNode->right) {
                if (currentNode->right->data.type == nodeType::STRING) {
                    WRITE("\tmov rax, qword ");
                    MEMORY(currentNode->right);
                    WRITE("\n");
                } else {
                    traverseAST(context, currentNode->right, outputData, nameTableIndex);
                    WRITE("\tpop rax\n");
                }
            }

            WRITE("\tmov rsp, rbp\n");
            WRITE("\tpop rbp\n");
            WRITE("\tret\n");

            break;
        }

        case Keyword::BREAK:
            WRITE("\tjmp ");
            LABEL("WHILE_END", context->counters->whileCount);
            WRITE("\n");
            break;

        case Keyword::CONTINUE:
            WRITE("\tjmp ");
            LABEL("WHILE_BEGIN", context->counters->whileCount);
            WRITE("\n");
            break;

        case Keyword::IN:
            WRITE("\tin\n");
            break;

        case Keyword::OUT:
            traverseAST(context, currentNode->right, outputData, nameTableIndex);

            WRITE("\tpop rax\n");
            WRITE("\tout\n");

            break;

        case Keyword::OPERATOR_SEPARATOR:
            traverseAST(context, currentNode->left, outputData, nameTableIndex);
            traverseAST(context, currentNode->right, outputData, nameTableIndex);
            break;

        case Keyword::ARGUMENT_SEPARATOR:
            if (context->callParameters) {
                traverseAST(context, currentNode->left, outputData, nameTableIndex);
                traverseAST(context, currentNode->right, outputData, nameTableIndex);
            } else {
                traverseAST(context, currentNode->right, outputData, nameTableIndex);
                traverseAST(context, currentNode->left, outputData, nameTableIndex);

                if (currentNode->left) {
                    WRITE("\tpop rax\n");
                    WRITE("\tmov ");
                    MEMORY(currentNode->left);
                    WRITE(", rax\n");
                }
            }

            break;

        default:
            return translationError::AST_BAD_STRUCTURE;
    }

    return translationError::NO_ERRORS;
}

static translationError writeFunction(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(currentNode, translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    int tableIndex = getLocalNameTableIndex(currentNode->data.data.nameTableIndex, context->localTables);

    if (tableIndex < 0) {
        return translationError::NAME_TABLE_ERROR;
    }

    if (currentNode->data.data.nameTableIndex == context->entryPoint) {
        WRITE("main:\n");
    } else {
        WRITE(context->nameTable->data[currentNode->data.data.nameTableIndex].name);
        WRITE(":\n");
    }

    size_t localsCount = context->localTables->data[tableIndex].size;
    size_t stackFrameSize = ((localsCount * 8 + 15) / 16) * 16;

    char stackFrameSizeString[MAX_NUMBER_LENGTH] = {};
    snprintf(stackFrameSizeString, MAX_NUMBER_LENGTH, "%lu", stackFrameSize);

    WRITE("\tpush rbp\n");
    WRITE("\tmov rbp, rsp\n");
    WRITE("\tsub rsp, ");
    WRITE(stackFrameSizeString);
    WRITE("\n");

    size_t paramOffset = 16;

    node<astNode> *paramNode = currentNode->left;

    while (paramNode && paramNode->data.type == nodeType::PARAMETERS) {
        if (paramNode->right) {
            paramNode->right->data.rbpOffset = paramOffset;
            paramOffset += 8;
        }

        paramNode = paramNode->left;
    }

    if (paramNode && paramNode->data.type != nodeType::KEYWORD) {
        paramNode->data.rbpOffset = paramOffset;
    }

    if (currentNode->right) {
        traverseAST(context, currentNode->right, outputData, tableIndex);
    } else {
        return translationError::AST_BAD_STRUCTURE;
    }

    return translationError::NO_ERRORS;
}

static translationError writeFunctionCall(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    size_t blockID = ++context->counters->callCount;

    if (!currentNode->right) {
        return translationError::AST_BAD_STRUCTURE;
    }
    
    context->callParameters = true;

    std::vector<node<astNode>*> arguments;
    node<astNode> *current = currentNode->left;

    while (current && current->data.type == nodeType::KEYWORD && current->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) {
        arguments.push_back(current->right);
        current = current->left;
    }

    if (current) {
        arguments.push_back(current);
    }

    for (auto it = arguments.rbegin(); it != arguments.rend(); ++it) {
        traverseAST(context, *it, outputData, nameTableIndex);
    }

    WRITE("\tand rsp, -16\n");
    WRITE("\tcall ");
    WRITE(context->nameTable->data[currentNode->right->data.data.nameTableIndex].name);
    WRITE("\n");

    if (!arguments.empty()) {
        char stackFrameSizeString[MAX_NUMBER_LENGTH] = {};
        snprintf(stackFrameSizeString, MAX_NUMBER_LENGTH, "%lu", arguments.size() * 8);
        
        WRITE("\tadd rsp, ");
        WRITE(stackFrameSizeString);
        WRITE("\n");
    }

    if (currentNode->parent && !(currentNode->parent->data.type == nodeType::KEYWORD && currentNode->parent->data.data.keyword == Keyword::OPERATOR_SEPARATOR)) {
        WRITE("\tpush rax\n");
    }

    return translationError::NO_ERRORS;
}

static translationError writeVariable(translationContext *context, node<astNode> *currentNode, Buffer<char> *outputData, size_t nameTableIndex) {
    customWarning(context, translationError::CONTEXT_BAD_POINTER);
    customWarning(currentNode, translationError::AST_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);

    if (nameTableIndex >= context->localTables->currentIndex) {
        return translationError::NAME_TABLE_ERROR;
    }

    if (context->localTables->data[nameTableIndex].elements.currentIndex == 0) {
        context->localTables->data[nameTableIndex].elements.currentIndex = context->localTables->data[nameTableIndex].size;
    }

    size_t size = context->localTables->data[nameTableIndex].size;
    size_t currentIndex = context->localTables->data[nameTableIndex].elements.currentIndex;

    if (currentIndex == 0 || currentIndex > size) {
        return translationError::AST_BAD_STRUCTURE;
    }

    currentNode->data.rbpOffset = -(currentIndex * 8);
    context->localTables->data[nameTableIndex].elements.currentIndex--;

    size_t stackFrameSize = ((size * 8 + 15) / 16) * 16;

    if (-currentNode->data.rbpOffset > stackFrameSize) {
        return translationError::AST_BAD_STRUCTURE;
    }

    size_t identifierIndex = currentNode->data.data.nameTableIndex;
    context->nameTable->data[identifierIndex].rbpOffset = currentNode->data.rbpOffset;

    if (currentNode->right && currentNode->right->data.type == nodeType::KEYWORD &&
        currentNode->right->data.data.keyword == Keyword::ASSIGNMENT) {
        traverseAST(context, currentNode->right, outputData, nameTableIndex);
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

    return translationError::NO_ERRORS;
}

static translationError writeBlockStatement(translationContext *context, Buffer<char> *outputData, codeBlock block) {
    customWarning(context,    translationError::CONTEXT_BAD_POINTER);
    customWarning(outputData, translationError::BUFFER_BAD_POINTER);
    
    char blockIDString[MAX_NUMBER_LENGTH] = {};
    snprintf(blockIDString, MAX_NUMBER_LENGTH, "%lu", block.blockID);

    return translationError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //