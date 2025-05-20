#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "IRBasics.h"

// TODO: add all instruction to the instruction list
// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define INITIALIZE_MEM_(_REG_) {.type = IR_OperandType::MEM_REG, .reg = _REG_, .isImmediate = false}

#define INITIALIZE_REG_(_REG_) {.type = IR_OperandType::REG, .reg    = _REG_, .isImmediate = false}
#define INITIALIZE_IMM_(_NUM_) {.type = IR_OperandType::IMM, .number = _IMM_, .isImmediate = true}

#define INITIALIZE_MEM_PLUS_IMM_(_REG_, _IMM_)  {.type = IR_OperandType::MEM_REG_PLUS_NUM,  .value.reg = _REG_, .value.number = _IMM_, .isImmediate = false}
#define INITIALIZE_MEM_MINUS_IMM_(_REG_, _IMM_) {.type = IR_OperandType::MEM_REG_MINUS_NUM, .value.reg = _REG_, .value.number = _IMM_, .isImmediate = false}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define INITIALIZE_FUNC_INDEX_(_FUNC_INDEX_) {.type = IR_OperandType::FUNC_INDEX, .value.functionIndex = _FUNC_INDEX_, .isImmediate = false}

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
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_REG_MEM_REG_MINUS_IMM(_REG1_, _REG2_, _IMM_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_MEM_MINUS_IMM_(_REG2_, _IMM_),   \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_REG_MEM(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_MEM_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

// ----------------------------------------------------------------  MEM <- REG  --------------------------------------------------------------------- //

#define MOV_MEM_REG_MINUS_IMM_REG(_REG1_, _IMM_, _REG2_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_MINUS_IMM_(_REG1_, _IMM_),   \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_MEM_REG_PLUS_IMM_REG(_REG1_, _IMM_, _REG2_) do {                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_PLUS_IMM_(_REG1_, _IMM_),    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_MEM_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

// ----------------------------------------------------------------  REG <- REG  --------------------------------------------------------------------- //

#define MOV_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0) 

// ----------------------------------------------------------------  MEM <- IMM  --------------------------------------------------------------------- //

#define MOV_MEM_IMM(_REG1_, _IMM_) do {                                         \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_(_REG1_),                    \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_MEM_REG_PLUS_IMM_IMM(_REG1_, _IMM1_, _IMM2_) do {                   \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_PLUS_IMM_(_REG1_, _IMM1_),   \
            .secondOperand        = INITIALIZE_IMM_(_IMM2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

#define MOV_MEM_REG_MINUS_IMM_IMM(_REG1_, _IMM1_, _IMM2_) do {                  \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_MEM_MINUS_IMM_(_REG1_, _IMM1_),  \
            .secondOperand        = INITIALIZE_IMM_(_IMM2_),                    \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

// ----------------------------------------------------------------  REG <- IMM  --------------------------------------------------------------------- //

#define MOV_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MOV,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### PUSH ###################################################################### //

#define PUSH_REG(_REG_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_PUSH,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
        insertNode(block->instructions, instruction);                           \
} while (0)

#define PUSH_IMM(_IMM_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_PUSH,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_IMM_(_IMM_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction                    \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### POP ####################################################################### //

#define POP_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_POP,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### ADD ####################################################################### //

#define ADD_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_ADD,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

#define ADD_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_ADD,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### SUB ####################################################################### //

#define SUB_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SUB,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

#define SUB_REG_IMM(_REG_, _IMM_) do {                                          \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SUB,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = INITIALIZE_IMM_(_IMM_),                     \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### MUL ####################################################################### //

#define MUL_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_MUL,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### DIV ####################################################################### //

#define DIV_REG(_REG_) do {                                                     \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_DIV,                        \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

#define CQO() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_CQO);     \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### SIN ####################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### COS ####################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ##################################################################### SQRT ####################################################################### //

#define SQRT_REG(_REG_) do {                                                    \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_SQRT,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG_),                     \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ##################################################################### FLOOR ###################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### CMP ####################################################################### //

#define CMP_REG_REG(_REG1_, _REG2_) do {                                        \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_CMP,                        \
            .operandCount         = 2,                                          \
            .firstOperand         = INITIALIZE_REG_(_REG1_),                    \
            .secondOperand        = INITIALIZE_REG_(_REG2_),                    \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### AND ####################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ####################################################################### OR ####################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### NOT ####################################################################### //

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### JMP ####################################################################### //

#define JMP() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_JMP);     \
} while (0)

#define JMP_CONDITION(_JMP_) do {                                               \
    IR_Instruction instruction = INITIALIZE_OPERATION(_JMP_);                   \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### CALL ####################################################################### //

#define CALL(_FUNC_INDEX_) do {                                                 \
    IR_Instruction instruction =                                                \
        {   .op                   = IR_Operator::IR_CALL,                       \
            .operandCount         = 1,                                          \
            .firstOperand         = INITIALIZE_FUNC_INDEX_(_FUNC_INDEX_),       \
            .secondOperand        = {},                                         \
            .currentFunctionIndex = context->currentFunction,                   \
        };                                                                      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### RET ####################################################################### //

#define RET() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_RET);     \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### IN ####################################################################### //

#define IN() do {                                                               \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_IN);      \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// ###################################################################### OUT ####################################################################### //

#define OUT() do {                                                              \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_OUT);     \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
// #################################################################### SYSCALL ##################################################################### //

#define SYSCALL() do {                                                          \
    IR_Instruction instruction = INITIALIZE_OPERATION(IR_Operator::IR_SYSCALL); \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // CODEGEN_H_