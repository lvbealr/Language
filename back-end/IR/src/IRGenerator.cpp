#include "IRGenerator.h"
#include "codegen.h"

IR_Error generateIR(IR_Context *IRContext) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

    IR *IR = IRContext->representation;
    linkedList<IR_BasicBlock> *blocks = (linkedList<IR_BasicBlock> *)calloc(1, sizeof(linkedList<IR_BasicBlock>));
    customWarning(blocks, IR_Error::BLOCKS_LIST_BAD_POINTER);
    initializeLinkedList(blocks, 0);
    customPrint(red, bold, bgDefault, "IR blocks list initialized\n");

    IR_BasicBlock *entryPoint = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
    customWarning(entryPoint, IR_Error::BASIC_BLOCK_BAD_POINTER);

    initializeBasicBlock(entryPoint, IRContext->ASTContext->nameTable->data[IRContext->ASTContext->entryPoint].name);
    customPrint(red, bold, bgDefault, "IR entry point initialized\n");

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
            
            node->data.rbpOffset = offset;

            localNameTable *localTable = &IRContext->ASTContext->localTables->data[IRContext->currentFunction];
            for (size_t i = 0; i < localTable->size; i++) {
                if (localTable->elements.data[i].globalNameID == node->data.data.nameTableIndex) {
                    localTable->elements.data[i].rbpOffset = offset;
                    break;
                }
            }

            if (node->right) {
                IR_Register *resultReg = NULL;

                IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, (*resultReg));
                freeRegister(IRContext->regAllocator, (*resultReg));
            }

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

                case Keyword::WHILE: {
                    IR_BasicBlock *condBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(condBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(condBlock, "while_cond");

                    IR_BasicBlock *bodyBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(bodyBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(bodyBlock, "while_body");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "while_merge");

                    JMP();
                    insertNode(block->successors,        (*condBlock) );
                    insertNode(condBlock->predecessors,  (*block)     );

                    block = condBlock;
                    IR_Register *conditionReg = NULL;
                    IR_Error error = generateExpressionIR(IRContext, node->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM((*conditionReg), 0);
                    JMP_CONDITION(IR_Operator::IR_JE);

                    insertNode(block->successors,        (*bodyBlock) );
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(bodyBlock->predecessors,  (*block)     );
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = bodyBlock;
                    if (node->right && node->right->data.type == nodeType::STRING) {
                        generateStatementIR(IRContext, node->right->left, block);
                    }

                    JMP();
                    insertNode(block->successors,        (*condBlock));
                    insertNode(condBlock->predecessors,  (*block)     );

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, (*conditionReg));
                    break;
                }

                case Keyword::RETURN: {
                    if (node->right) {
                        IR_Register *resultReg = NULL;

                        IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                        MOV_REG_REG(IR_Register::RAX, (*resultReg));
                        freeRegister(IRContext->regAllocator, (*resultReg));
                    }

                    MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
                    POP_REG    (IR_Register::RBP);
                    RET        ();
                    break;
                }

                case Keyword::BREAK: {
                    // TODO:
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::CONTINUE: {
                    // TODO:
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::ABORT: {
                    // syscall exit(1)
                    MOV_REG_IMM(IR_Register::RDI, 1);
                    MOV_REG_IMM(IR_Register::RAX, 60);
                    SYSCALL();
                    break;
                }

                case Keyword::OUT: {
                    IR_Register *resultReg = NULL;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    // TODO:
                    freeRegister(IRContext->regAllocator, (*resultReg));
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::FUNCTION_CALL: {
                    if (node->left && node->right) {
                        // TODO:
                        return IR_Error::NOT_IMPLEMENTED;
                    }

                    break;
                }

                case Keyword::IN: {
                    // TODO:
                    break;
                }

                default: {
                    return IR_Error::AST_BAD_STRUCTURE;
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
            (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
            MOV_REG_IMM((*resultReg), node->data.data.number);
            break;
        }

        case nodeType::STRING: {
            (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
            size_t offset = getVariableOffset(IRContext, node->data.data.nameTableIndex);
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

                case Keyword::SUB: {
                    SUB_REG_REG((*leftReg), (*rightReg));
                    (*resultReg) = (*leftReg);
                    freeRegister(IRContext->regAllocator, (*rightReg));
                    break;
                }

                case Keyword::MUL: {
                    IMUL_REG_REG((*leftReg), (*rightReg));
                    (*resultReg) = (*leftReg);
                    freeRegister(IRContext->regAllocator, (*rightReg));
                    break;
                }

                case Keyword::DIV: {
                    MOV_REG_REG(IR_Register::RAX, (*leftReg));
                    CQO();
                    IDIV_REG((*rightReg));
                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_REG((*resultReg), IR_Register::RAX);
                    freeRegister(IRContext->regAllocator, (*leftReg));
                    freeRegister(IRContext->regAllocator, (*rightReg));
                    break;
                }

                case Keyword::LESS:
                case Keyword::GREATER:
                case Keyword::LESS_OR_EQUAL:
                case Keyword::GREATER_OR_EQUAL:
                case Keyword::NOT_EQUAL:
                case Keyword::EQUAL: {
                    CMP_REG_REG((*leftReg), (*rightReg));
                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM((*resultReg), 0);

                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    IR_Operator jmpOp;

                    switch (node->data.data.keyword) {
                        case Keyword::EQUAL: jmpOp = IR_Operator::IR_JNE; break;
                        case Keyword::LESS: jmpOp = IR_Operator::IR_JGE; break;
                        case Keyword::GREATER: jmpOp = IR_Operator::IR_JLE; break;
                        case Keyword::LESS_OR_EQUAL: jmpOp = IR_Operator::IR_JG; break;
                        case Keyword::GREATER_OR_EQUAL: jmpOp = IR_Operator::IR_JL; break;
                        case Keyword::NOT_EQUAL: jmpOp = IR_Operator::IR_JE; break;
                        default: return IR_Error::AST_BAD_STRUCTURE;
                    }

                    JMP_CONDITION(jmpOp);
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
                
                case Keyword::AND: {
                    CMP_REG_IMM((*leftReg), 0);
                    IR_BasicBlock *falseBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(falseBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(falseBlock, "false");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors,        (*falseBlock));
                    insertNode(falseBlock->predecessors, (*block)     );

                    error = generateExpressionIR(IRContext, node->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM((*rightReg), 0);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors,        (*falseBlock));
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(falseBlock->predecessors, (*block)     );
                    insertNode(mergeBlock->predecessors, (*block)     );

                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM((*resultReg), 1);
                    JMP();
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = mergeBlock;
                    MOV_REG_IMM((*resultReg), 0);
                    JMP();
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(mergeBlock->predecessors, (*block)     );

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, (*leftReg));
                    freeRegister(IRContext->regAllocator, (*rightReg));

                    break;
                }

                case Keyword::OR: {
                    CMP_REG_IMM((*leftReg), 1);

                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors,        (*trueBlock));
                    insertNode(trueBlock->predecessors,  (*block)    );

                    error = generateExpressionIR(IRContext, node->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM((*rightReg), 1);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors,        (*trueBlock));
                    insertNode(block->successors,        (*mergeBlock));
                    insertNode(trueBlock->predecessors,  (*block)    );
                    insertNode(mergeBlock->predecessors, (*block)    );

                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM((*resultReg), 0);
                    JMP();
                    insertNode(block->successors,        (*mergeBlock));
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

                case Keyword::NOT: {
                    IR_Register *operandReg = NULL;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, operandReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM((*resultReg), 0);
                    CMP_REG_IMM((*operandReg), 0);

                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    JMP_CONDITION(IR_Operator::IR_JE);
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
                    freeRegister(IRContext->regAllocator, (*operandReg));
                    break;
                }

                case Keyword::SIN:
                case Keyword::COS:
                case Keyword::FLOOR:
                case Keyword::SQRT: {
                    IR_Register *argReg = NULL;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, argReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    // TODO: 
                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    freeRegister(IRContext->regAllocator, (*argReg));
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::DIFF: {
                    // TODO:
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::IN: {
                    // TODO:
                    (*resultReg) = allocateRegister(IRContext, IRContext->regAllocator, block);
                    return IR_Error::NOT_IMPLEMENTED;
                }

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
