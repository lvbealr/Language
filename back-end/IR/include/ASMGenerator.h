#ifndef ASM_GENERATOR_H_
#define ASM_GENERATOR_H_

#include "IRBasics.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

const char *IR_OperatorToString(IR_Operator op);
const char *IR_RegisterToString(IR_Register reg);

std::string IR_OperandToString (const IR_Operand &operand);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error printIR    (IR *IR);
IR_Error generateASM(IR_Context *IRContext, IR *IR, const char *filename);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // ASM_GENERATOR_H_