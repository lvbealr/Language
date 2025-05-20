#include "IRGenerator.h"
#include "codegen.h"

IR_Error generateIR(IR_Context *IRContext) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

    IR *IR = IRContext->representation;
    linkedList<IR_BasicBlock> *blocks = (linkedList<IR_BasicBlock> *)calloc(1, sizeof(linkedList<IR_BasicBlock>));
    customWarning(blocks, IR_Error::BLOCKS_LIST_BAD_POINTER);
    initializeLinkedList(blocks, 0);

    IR_BasicBlock *entryPoint = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
    customWarning(entryPoint, IR_Error::BASIC_BLOCK_BAD_POINTER);

    initializeBasicBlock(entryPoint, IRContext->ASTContext->nameTable->data[IRContext->ASTContext->entryPoint].name);

    initializeIR(IR, blocks, entryPoint);

    IRContext->currentFunction = IRContext->ASTContext->entryPoint;

    IR_Error error = generateFunctionIR(IRContext, IRContext->ASTContext->AST->root, entryPoint);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_FUNCTION_IR_ERROR);

    return IR_Error::NO_ERRORS;
}

IR_Error generateFunctionIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(node,      IR_Error::NODE_BAD_POINTER);
    customWarning(block,     IR_Error::BASIC_BLOCK_BAD_POINTER);

    // prologue generation
    PUSH_REG   (IR_Register::RBP);
    MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);

    size_t localSize = IRContext->ASTContext->localTables->data[IRContext->currentFunction].size;
    SUB_REG_IMM(IR_Register::RSP, localSize);

    IR_Error error = generateStatementIR(IRContext, node->left, block);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);

    // epilogue generation
    MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
    POP_REG    (IR_Register::RBP);
    RET        ();

    return IR_Error::NO_ERRORS;
}


IR_Error generateStatementIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(node,      IR_Error::NODE_BAD_POINTER);
    customWarning(block,     IR_Error::BASIC_BLOCK_BAD_POINTER);

    switch (node->data.type) {
        case nodeType::VARIABLE_DECLARATION: {
            size_t offset = IRContext->regAllocator->stackOffset;
            IRContext->regAllocator->stackOffset += 8;
            // TODO: ?
            break;
        }

        case nodeType::KEYWORD: {
            switch (node->data.data.keyword) {
                case Keyword::IF: {
                    IR_BasicBlock *thenBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(thenBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(thenBlock, "then");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    IR_Register *conditionReg = NULL;

                    IR_Error error = generateExpressionIR(IRContext, node->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM((*conditionReg), 0);
                    JMP_CONDITION(IR_Operator::IR_JE);

                    insertNode(block->successors,        (*thenBlock) );
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(thenBlock->predecessors,  (*block)     );
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = thenBlock;
                    if (node->right && node->right->data.type == nodeType::STRING) {
                        generateStatementIR(IRContext, node->right->left, block);
                    }
                    JMP();
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, (*conditionReg));

                    break;
                }

                case Keyword::ASSIGNMENT: {
                    IR_Register *resultReg = NULL;

                    IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    size_t offset = getVariableOffset(IRContext, node->left->data.data.nameTableIndex);
                    customWarning(offset != 0, IR_Error::VARIABLE_NOT_FOUND);

                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, (*resultReg));
                    freeRegister(IRContext->regAllocator, (*resultReg));
                    break;
                }

                // TODO:

                default: {
                    break;
                }
            }

            break;
        }

        case nodeType::STRING: {
            IR_Error error = generateStatementIR(IRContext, node->left, block);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);

            return generateStatementIR(IRContext, node->right, block);
        }

        default: {
            break;
        }
    }

    return IR_Error::NO_ERRORS;
}


IR_Error generateExpressionIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block, IR_Register *resultReg) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(node,      IR_Error::NODE_BAD_POINTER);
    customWarning(block,     IR_Error::BASIC_BLOCK_BAD_POINTER);
    customWarning(resultReg, IR_Error::REGISTER_BAD_POINTER);

    switch (node->data.type) {
        case nodeType::CONSTANT: {
            (*resultReg) = allocateRegister(IRContext->regAllocator, block);
            MOV_REG_IMM((*resultReg), node->data.data.number);
            break;
        }

        case nodeType::STRING: {
            (*resultReg) = allocateRegister(IRContext->regAllocator, block);
            size_t offset = getVariableOffset(IRContext, node->data.data.nameTableIndex); // TODO:
            customWarning(offset != 0, IR_Error::VARIABLE_NOT_FOUND);

            MOV_REG_MEM_REG_MINUS_IMM((*resultReg), IR_Register::RBP, offset);
            break;
        }

        case nodeType::KEYWORD: {
            IR_Register *leftReg, *rightReg = NULL;
            IR_Error error = generateExpressionIR(IRContext, node->left, block, leftReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            error = generateExpressionIR(IRContext, node->right, block, rightReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            switch (node->data.data.keyword) {
                case Keyword::ADD: {
                    ADD_REG_REG((*leftReg), (*rightReg));
                    (*resultReg) = (*leftReg);
                    freeRegister(IRContext->regAllocator, (*rightReg));
                    break;
                }

                case Keyword::EQUAL: {
                    CMP_REG_REG((*leftReg), (*rightReg));
                    (*resultReg) = allocateRegister(IRContext->regAllocator, block);
                    MOV_REG_IMM((*resultReg), 0);
                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    JMP_CONDITION(IR_Operator::IR_JNE);
                    insertNode(block->successors,        (*trueBlock) );
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(trueBlock->predecessors,  (*block)     );
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = trueBlock;
                    MOV_REG_IMM((*resultReg), 1);
                    JMP();
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, (*leftReg));
                    freeRegister(IRContext->regAllocator, (*rightReg));
                    break;
                }
                
                // TODO:

                default: {
                    return IR_Error::AST_BAD_STRUCTURE;
                }
            }

            break;
        }

        default: {
            return IR_Error::AST_BAD_STRUCTURE;
        }
    }

    return IR_Error::NO_ERRORS;
}

size_t getVariableOffset(IR_Context *IRContext, size_t nameTableIndex) {
    customWarning(IRContext, 0);

    localNameTable *localTable = &IRContext->ASTContext->localTables->data[IRContext->currentFunction];

    for (size_t i = 0; i < localTable->size; i++) {
        if (localTable->elements.data[i].globalNameID == nameTableIndex) {
            return localTable->elements.data[i].rbpOffset;
        }
    }

    return 0;
}
