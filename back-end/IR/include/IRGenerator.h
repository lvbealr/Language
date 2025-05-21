#ifndef IR_GENERATOR_H_
#define IR_GENERATOR_H_

#include "IRBasics.h"
#include "core.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateIR(IR_Context *IRContext);

IR_Error generateFunctionIR     (IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block);
IR_Error generateStatementIR    (IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block);
IR_Error generateExpressionIR   (IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block, IR_Register &resultReg);

size_t getVariableOffset        (IR_Context *IRContext, size_t nameTableIndex);

IR_Error printIR(IR *IR);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // IR_GENERATOR_H_