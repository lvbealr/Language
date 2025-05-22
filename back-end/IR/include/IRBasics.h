#ifndef IR_BASICS_H_
#define IR_BASICS_H_

#include <stdlib.h>
#include "linkedList.h"
#include "binaryTree.h"
#include "buffer.h"
#include "AST.h"
#include "asmTranslator.h"
#include "core.h"
#include <string>
#include <unordered_map>

enum class IR_Error {
    NO_ERRORS                      = 0,
    IR_CONTEXT_BAD_POINTER         = 1,
    REGISTER_ALLOCATOR_BAD_POINTER = 2,
    ALLOCATION_ERROR               = 3,
    BASIC_BLOCK_BAD_POINTER        = 4,
    REGISTER_NOT_FOUND             = 5,
    IR_BAD_POINTER                 = 6,
    BLOCKS_LIST_BAD_POINTER        = 7,
    LABEL_BAD_POINTER              = 8,
    GENERATE_EXPRESSION_IR_ERROR   = 9,
    GENERATE_FUNCTION_IR_ERROR     = 10,
    GENERATE_STATEMENT_IR_ERROR    = 11,
    AST_BAD_STRUCTURE              = 12,
    NODE_BAD_POINTER               = 13,
    REGISTER_BAD_POINTER           = 14,
    VARIABLE_NOT_FOUND             = 15,
    NOT_IMPLEMENTED                = 16,
    LOCAL_TABLE_NOT_FOUND          = 17,
    IO_ERROR                       = 18,
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

enum class IR_Operator {
    IR_NOP,
    IR_MOV,
    IR_PUSH,
    IR_POP,
    IR_ADD,
    IR_SUB,
    IR_IMUL,
    IR_IDIV,
    IR_CQO,
    IR_SIN,
    IR_COS,
    IR_SQRT,
    IR_FLOOR,
    IR_CMP,
    IR_AND,
    IR_OR,
    IR_NOT,
    IR_JMP,
    IR_JE,
    IR_JNE,
    IR_JG,
    IR_JGE,
    IR_JL,
    IR_JLE,
    IR_CALL,
    IR_RET,
    IR_IN,
    IR_OUT,   
    IR_ABORT,
    IR_BREAK,
    IR_CONTINUE,
    IR_HLT,  
    IR_SYSCALL
};

enum class IR_OperandType {
    NONE,                   // 
    REG,                    // R[A-D]X, RSP, RBP, R[8-15]
    IMM,                    // [0-9]+
    MEM_IMM,                // [[0-9]+]
    MEM_REG,                // [R[A-D]X, RSP, RBP, R[8-15]]
    MEM_REG_PLUS_IMM,       // [R[A-D]X, RSP, RBP, R[8-15] + [0-9]+]
    MEM_REG_MINUS_IMM,      // [R[A-D]X, RSP, RBP, R[8-15] - [0-9]+]
    LABEL,                  // [a-zA-Z_][a-zA-Z0-9_]+ <- jump
    FUNC_INDEX,             // [0-9]+ <- call
};

enum class IR_Register {
    RAX,
    RBX, 
    RCX, 
    RDX, 
    RSP, 
    RBP, 
    RSI, 
    RDI, 
    R8, R9, R10, R11, R12, R13, R14, R15
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

struct registerAllocator {
    IR_Register *registers   = {};
    bool        *used        = {};
    size_t       count       = {};
    size_t       stackOffset = {};

    bool        *isCalleeSaved = {};
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

struct IR_Operand {
    IR_OperandType type = {};
    
    IR_Register reg           = {};  // type = {REG | MEM_REG  | MEM_REG_PLUS_NUM | MEM_REG_MINUS_NUM}
    int         number        = {};  // type = {NUM | MEM_NUM, | MEM_REG_PLUS_NUM | MEM_REG_MINUS_NUM}
    char       *label         = {};  // type = {LABEL}
    size_t      functionIndex = {};  // type = {FUNC_INDEX}

    bool isImmediate; 
};

struct IR_Instruction {
    IR_Operator op           = {};
    size_t      operandCount = {};

    IR_Operand  firstOperand  = {};
    IR_Operand  secondOperand = {};

    size_t currentFunctionIndex = {};   // (0 if out of function)

    IR_Instruction *nextInstruction = {};
    IR_Instruction *prevInstruction = {};

    bool isFirstInFunc = {};            // prologue generation
    bool isLastInScope = {};            // jumps correct generation

    size_t elfPosition = {};            // instruction offset in .text section
};

struct IR_BasicBlock {
    char *label = {};                               // block name

    linkedList<IR_Instruction> *instructions = {};  // instructions in the block
    size_t instructionCount = {};                   // instructions count in the block 

    linkedList<IR_BasicBlock *> *successors   = {};   // next blocks     (control flow graph)
    linkedList<IR_BasicBlock *> *predecessors = {};   // previous blocks (control flow graph)

    size_t functionIndex = {};                      // function index
};

struct IR {
    linkedList<IR_BasicBlock *> *basicBlocks = {};    // basic blocks

    size_t         entryPointIndex = {};            // entry point index
    IR_BasicBlock *entryPoint = {};                 // entry point basic block

    size_t totalInstructionCount = {};              // total instructions count
};

struct IR_Context {
    translationContext *ASTContext = {};

    IR *representation = {};

    registerAllocator *regAllocator = {};

    size_t currentFunction = {};
    bool hasReturn = {};
    std::map<std::string, size_t> functionNameToIndex;
    std::unordered_map<size_t, IR_Register> variableRegisterCache;
};

struct IR_Comment {
    char *comment = {};
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeBasicBlock(IR_BasicBlock *block, const char *label);

IR_Error initializeIR(IR *IR, linkedList<IR_BasicBlock> *blocks, IR_BasicBlock *entryPoint);
IR_Error destroyIR   (IR *IR);

IR_Error initializeIRContext(IR_Context *IRContext, translationContext *ASTContext);
IR_Error destroyIRContext   (IR_Context *IRContext);

IR_Error initializeRegisterAllocator(registerAllocator *allocator);
IR_Error destroyRegisterAllocator   (registerAllocator *allocator);

IR_Register allocateRegister(IR_Context *IRContext, registerAllocator *allocator, IR_BasicBlock *block);
IR_Error    freeRegister    (IR_Context *context, registerAllocator *allocator, IR_Register    reg);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // IR_BASICS_H_