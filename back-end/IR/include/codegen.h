#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "IRBasics.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define INITIALIZE_MEM_(_REG_) {.type = IR_OperandType::MEM_REG, .reg = _REG_, .isImmediate = false}

#define INITIALIZE_REG_(_REG_) {.type = IR_OperandType::REG, .reg = _REG_, .isImmediate = false}
#define INITIALIZE_IMM_(_IMM_) {.type = IR_OperandType::IMM, .number = _IMM_, .isImmediate = true}

#define INITIALIZE_MEM_PLUS_IMM_(_REG_, _IMM_)  {.type = IR_OperandType::MEM_REG_PLUS_IMM,  .reg = _REG_, .number = _IMM_, .isImmediate = false}
#define INITIALIZE_MEM_MINUS_IMM_(_REG_, _IMM_) {.type = IR_OperandType::MEM_REG_MINUS_IMM, .reg = _REG_, .number = _IMM_, .isImmediate = false}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define INITIALIZE_FUNC_INDEX_(_FUNC_INDEX_) {.type = IR_OperandType::FUNC_INDEX, .functionIndex = _FUNC_INDEX_, .isImmediate = false}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define INITIALIZE_OPERATION(_OPERATION_) {.op = _OPERATION_}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### MOV ####################################################################### //

// ----------------------------------------------------------------  REG <- MEM  --------------------------------------------------------------------- //

#define MOV_REG_MEM_REG_PLUS_IMM(_REG1_, _REG2_, _IMM_) do {                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_MEM_PLUS_IMM_(_REG2_, _IMM_),    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_REG_MEM_REG_MINUS_IMM(_REG1_, _REG2_, _IMM_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_MEM_MINUS_IMM_(_REG2_, _IMM_),   \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_REG_MEM(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_MEM_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// ----------------------------------------------------------------  MEM <- REG  --------------------------------------------------------------------- //

#define MOV_MEM_REG_MINUS_IMM_REG(_REG1_, _IMM_, _REG2_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_MINUS_IMM_(_REG1_, _IMM_),   \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_MEM_REG_PLUS_IMM_REG(_REG1_, _IMM_, _REG2_) do {                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_PLUS_IMM_(_REG1_, _IMM_),    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_MEM_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// ----------------------------------------------------------------  REG <- REG  --------------------------------------------------------------------- //

#define MOV_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0) 

// ----------------------------------------------------------------  MEM <- IMM  --------------------------------------------------------------------- //

#define MOV_MEM_IMM(_REG1_, _IMM_) do {                                         \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_(_REG1_),                    \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_MEM_REG_PLUS_IMM_IMM(_REG1_, _IMM1_, _IMM2_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_PLUS_IMM_(_REG1_, _IMM1_),   \
            .secondOperand        = INITIALIZE_IMM_(_IMM2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define MOV_MEM_REG_MINUS_IMM_IMM(_REG1_, _IMM1_, _IMM2_) do {                  \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_MINUS_IMM_(_REG1_, _IMM1_),  \
            .secondOperand        = INITIALIZE_IMM_(_IMM2_),                    \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// ----------------------------------------------------------------  REG <- IMM  --------------------------------------------------------------------- //

#define MOV_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### PUSH ###################################################################### //

#define PUSH_REG(_REG_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_PUSH,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define PUSH_IMM(_IMM_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_PUSH,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_IMM_(_IMM_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction                  \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### POP ####################################################################### //

#define POP_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_POP,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### ADD ####################################################################### //

#define ADD_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_ADD,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define ADD_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_ADD,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### SUB ####################################################################### //

#define SUB_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SUB,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define SUB_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SUB,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### IMUL ###################################################################### //

#define IMUL_REG_REG(_REG1_, _REG2_) do {                                       \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_IMUL,                       \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### IDIV ###################################################################### //

#define IDIV_REG(_REG_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_IDIV,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define CQO() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_CQO);     \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### SIN ####################################################################### //

#define SIN_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SIN,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### COS ####################################################################### //

#define COS_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_COS,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### SQRT ###################################################################### //

#define SQRT_REG(_REG_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SQRT,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### FLOOR ##################################################################### //

#define FLOOR_REG(_REG_) do {                                                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_FLOOR,                      \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### AND ####################################################################### //

#define AND_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_AND,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### OR ######################################################################## //

#define OR_REG_REG(_REG1_, _REG2_) do {                                         \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_OR,                         \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### NOT ####################################################################### //

#define NOT_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_NOT,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### CMP ####################################################################### //

#define CMP_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction = {                                              \
        .op           = IR_Operator::IR_CMP,                                    \
        .operandCount = 2,                                                      \
        .firstOperand  = INITIALIZE_REG_(_REG_),                                \
        .secondOperand = INITIALIZE_IMM_(_IMM_)                                 \
    };                                                                          \
    insertNode(block->instructions, instruction);                               \
} while (0)

#define CMP_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_CMP,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### JMP ####################################################################### //

#define JMP() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_JMP);     \
    insertNode(block->instructions, instruction);                               \
} while (0)

#define JMP_CONDITION(_JMP_) do {                                               \
    IR_Instruction instruction = INITIALIZE_OPERATION(_JMP_);                   \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### CALL ###################################################################### //

#define CALL(_FUNC_INDEX_) do {                                                 \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_CALL,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_FUNC_INDEX_(_FUNC_INDEX_),       \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = IRContext->currentFunction,                 \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### RET ####################################################################### //

#define RET() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_RET);     \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### IN ######################################################################## //

#define IN() do {                                                               \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_IN);      \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### OUT ####################################################################### //

#define OUT() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_OUT);     \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// #################################################################### SYSCALL ##################################################################### //

#define SYSCALL() do {                                                          \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_SYSCALL); \
    insertNode(block->instructions, instruction);                               \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // CODEGEN_H_