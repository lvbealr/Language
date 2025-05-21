#include "IRGenerator.h"
#include "codegen.h"
#include <stdio.h>
#include <string>
#include <vector>

static const char *IR_OperatorToString(IR_Operator op) {
    switch (op) {
        case IR_Operator::IR_MOV:       return "mov";
        case IR_Operator::IR_PUSH:      return "push";
        case IR_Operator::IR_POP:       return "pop";
        case IR_Operator::IR_ADD:       return "add";
        case IR_Operator::IR_SUB:       return "sub";
        case IR_Operator::IR_IMUL:      return "imul";
        case IR_Operator::IR_IDIV:      return "idiv";
        case IR_Operator::IR_SIN:       return "sin";
        case IR_Operator::IR_COS:       return "cos";
        case IR_Operator::IR_SQRT:      return "sqrt";
        case IR_Operator::IR_FLOOR:     return "floor";
        case IR_Operator::IR_AND:       return "and";
        case IR_Operator::IR_OR:        return "or";
        case IR_Operator::IR_NOT:       return "not";
        case IR_Operator::IR_CMP:       return "cmp";
        case IR_Operator::IR_JMP:       return "jmp";
        case IR_Operator::IR_JE:        return "je";
        case IR_Operator::IR_JNE:       return "jne";
        case IR_Operator::IR_JL:        return "jl";
        case IR_Operator::IR_JLE:       return "jle";
        case IR_Operator::IR_JG:        return "jg";
        case IR_Operator::IR_JGE:       return "jge";
        case IR_Operator::IR_CALL:      return "call";
        case IR_Operator::IR_RET:       return "ret";
        case IR_Operator::IR_IN:        return "in";
        case IR_Operator::IR_OUT:       return "out";
        case IR_Operator::IR_SYSCALL:   return "syscall";
        case IR_Operator::IR_CQO:       return "cqo";
        default:                        return "unknown operation";
    }
}

static const char *IR_RegisterToString(IR_Register reg) {
    switch (reg) {
        case IR_Register::RAX:  return "rax";
        case IR_Register::RBX:  return "rbx";
        case IR_Register::RCX:  return "rcx";
        case IR_Register::RDX:  return "rdx";
        case IR_Register::RSI:  return "rsi";
        case IR_Register::RDI:  return "rdi";
        case IR_Register::RBP:  return "rbp";
        case IR_Register::RSP:  return "rsp";
        case IR_Register::R8:   return "r8";
        case IR_Register::R9:   return "r9";
        case IR_Register::R10:  return "r10";
        case IR_Register::R11:  return "r11";
        case IR_Register::R12:  return "r12";
        case IR_Register::R13:  return "r13";
        case IR_Register::R14:  return "r14";
        case IR_Register::R15:  return "r15";
        default:                return "unknown register";
    }
}

static std::string IR_OperandToString(const IR_Operand &operand) {
    std::string result;
    
    switch (operand.type) {
        case IR_OperandType::NONE:
            result = "none";
            break;
        case IR_OperandType::REG:
            result = IR_RegisterToString(operand.reg);
            break;
        case IR_OperandType::IMM:
            result = std::to_string(operand.number);
            break;
        case IR_OperandType::MEM_REG:
            result = "[" + std::string(IR_RegisterToString(operand.reg)) + "]";
            break;
        case IR_OperandType::MEM_REG_PLUS_IMM:
            result = "[" + std::string(IR_RegisterToString(operand.reg)) + " + " + std::to_string(operand.number) + "]";
            break;
        case IR_OperandType::MEM_REG_MINUS_IMM:
            result = "[" + std::string(IR_RegisterToString(operand.reg)) + " - " + std::to_string(operand.number) + "]";
            break;
        case IR_OperandType::FUNC_INDEX:
            result = "func_" + std::to_string(operand.functionIndex);
            break;
        case IR_OperandType::LABEL:
            result = operand.label ? std::string(operand.label) : "null_label";
            break;
        default:
            result = "unknown_operand";
    }

    if (operand.isImmediate && operand.type != IR_OperandType::IMM) {
        result = "imm(" + result + ")";
    }

    return result;
}

void debugPrintInstructions(IR_BasicBlock* block) {
    customPrint(blue, bold, bgDefault, "Debug: Printing instructions for block %s\n", block->label);
    printf("Instruction count: %zd\n", block->instructions->size);

    ssize_t currentInstIndex = block->instructions->next[0];
    size_t instCount = 0;

    while (currentInstIndex != 0) {
        IR_Instruction inst = block->instructions->data[currentInstIndex];
        printf("  %zd: %s", instCount++, IR_OperatorToString(inst.op));

        if (inst.operandCount > 0) {
            printf(" %s", IR_OperandToString(inst.firstOperand).c_str());

            if (inst.operandCount > 1) {
                printf(", %s", IR_OperandToString(inst.secondOperand).c_str());
            }
        }

        printf("\n");

        currentInstIndex = block->instructions->next[currentInstIndex];
    }
}

IR_Error printIR(IR *IR) {
    customWarning(IR, IR_Error::IR_BAD_POINTER);

    printf("\n=== Intermediate Representation ===\n");

    if (IR->entryPoint) {
        printf("Entry Point: %s\n", IR->entryPoint->label ? IR->entryPoint->label : "Unknown");
    } else {
        printf("Entry Point: absent\n");
    }

    printf("Total Blocks Count: %zd\n", IR->basicBlocks->size);
    
    size_t totalInstructions = 0;
    ssize_t currentBlockIndex = IR->basicBlocks->next[0];

    while (currentBlockIndex != 0) {
        IR_BasicBlock *block = IR->basicBlocks->data[currentBlockIndex];
        printf("\nBase Block: %s (Function index: %zd)\n", block->label ? block->label : "Unknown", block->functionIndex);

        size_t instCount = 0;
        ssize_t currentInstIndex = block->instructions->next[0];

        while (currentInstIndex != 0) {
            instCount++;
            currentInstIndex = block->instructions->next[currentInstIndex];
        }
        
        printf("Total Instructions Count (%zd):\n", instCount);
        totalInstructions += instCount;

        customPrint(blue, bold, bgDefault, "::: instruction->size = %zd, next[0] = %zd\n", 
                    block->instructions->size, block->instructions->next[0]);

        if (instCount == 0) {
            customPrint(red, bold, bgDefault, "Warning: Instruction List Is Empty\n");
        }

        currentInstIndex = block->instructions->next[0];
        size_t printCount = 0;

        while (currentInstIndex != 0) {
            IR_Instruction &inst = block->instructions->data[currentInstIndex];
            printf("  %zd: %s", printCount++, IR_OperatorToString(inst.op));

            if (inst.operandCount > 0) {
                printf(" %s", IR_OperandToString(inst.firstOperand).c_str());

                if (inst.operandCount > 1) {
                    printf(", %s", IR_OperandToString(inst.secondOperand).c_str());
                }
            }

            printf("\n");

            currentInstIndex = block->instructions->next[currentInstIndex];
        }

        printf("  Predecessors: ");
        ssize_t predIndex = block->predecessors->next[0];

        while (predIndex != 0) {
            printf("%s ", block->predecessors->data[predIndex].label ? block->predecessors->data[predIndex].label : "Unknown");
            predIndex = block->predecessors->next[predIndex];
        }

        printf("\n");
        printf("  Successors: ");

        ssize_t succIndex = block->successors->next[0];

        while (succIndex != 0) {
            printf("%s ", block->successors->data[succIndex].label ? block->successors->data[succIndex].label : "Unknown");
            succIndex = block->successors->next[succIndex];
        }

        printf("\n");

        currentBlockIndex = IR->basicBlocks->next[currentBlockIndex];
    }

    printf("Total Instructions: %zd\n", totalInstructions);
    printf("==================================\n\n");

    return IR_Error::NO_ERRORS;
}

void collectFunctionNodes(node<astNode>* current, std::vector<node<astNode>*>& functions) {
    while (current) {
        if (current->data.type == nodeType::KEYWORD && 
            current->data.data.keyword == Keyword::OPERATOR_SEPARATOR) {
            if (current->left && current->left->data.type == nodeType::FUNCTION_DEFINITION) {
                functions.push_back(current->left);
            }
        }
        
        current = current->right;
    }
}

IR_Error generateIR(IR_Context *IRContext) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

    for (size_t i = 0; i < IRContext->ASTContext->localTables->currentIndex; i++) {
        for (size_t j = 0; j < IRContext->ASTContext->localTables->data[i].size; j++) {
            customPrint(blue, bold, bgDefault, "Local Table %zu: %s\n", i, 
                        IRContext->ASTContext->nameTable->data[IRContext->ASTContext->localTables->data[i].elements.data[j].globalNameID].name);
        }
    }

    IR *IR = IRContext->representation;

    linkedList<IR_BasicBlock*> *blocks = (linkedList<IR_BasicBlock*> *)calloc(1, sizeof(linkedList<IR_BasicBlock*>));
    customWarning(blocks, IR_Error::BLOCKS_LIST_BAD_POINTER);
    initializeLinkedList(blocks, 10);

    IR->basicBlocks = blocks;

    std::vector<node<astNode> *> functions;
    if (IRContext->ASTContext->AST->root->left && 
        IRContext->ASTContext->AST->root->left->data.type == nodeType::FUNCTION_DEFINITION) {
        functions.push_back(IRContext->ASTContext->AST->root->left);
    } else {
        customPrint(red, bold, bgDefault, "Error: root->left is not FUNCTION_DEFINITION\n");
        return IR_Error::AST_BAD_STRUCTURE;
    }

    collectFunctionNodes(IRContext->ASTContext->AST->root->right, functions);

    if (functions.empty()) {
        customPrint(red, bold, bgDefault, "Error: there are no FUNCTION_DEFINITION in AST\n");
        return IR_Error::AST_BAD_STRUCTURE;
    }

    node<astNode> *entryPointNode = nullptr;
    std::string entryPointName = IRContext->ASTContext->nameTable->data[IRContext->ASTContext->entryPoint].name;
    size_t entryPointIndex = 0;

    for (size_t i = 0; i < functions.size(); ++i) {
        std::string funcName = IRContext->ASTContext->nameTable->data[functions[i]->data.data.nameTableIndex].name;
        customPrint(blue, bold, bgDefault, "Function node: %s\n", funcName.c_str());

        if (funcName == entryPointName) {
            entryPointNode = functions[i];
            entryPointIndex = i;
            break;
        }
    }

    if (!entryPointNode) {
        customPrint(red, bold, bgDefault, "Error: Entry Point '%s' not found\n", entryPointName.c_str());
        return IR_Error::AST_BAD_STRUCTURE;
    }

    IR_BasicBlock *entryPointBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
    customWarning(entryPointBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
    initializeBasicBlock(entryPointBlock, entryPointName.c_str());

    IR->entryPoint = entryPointBlock;
    linkedListError llError = insertNode(IR->basicBlocks, entryPointBlock);
    customWarning(llError == linkedListError::NO_ERRORS, IR_Error::BLOCKS_LIST_BAD_POINTER);

    IRContext->currentFunction = IRContext->ASTContext->entryPoint;
    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    customPrint(blue, bold, bgDefault, "Generate IR for entry point, localTableIndex: %zd\n", localTableIndex);
    IR_Error error = generateFunctionIR(IRContext, entryPointNode, entryPointBlock);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_FUNCTION_IR_ERROR);

    for (size_t i = 0; i < functions.size(); ++i) {
        if (i != entryPointIndex) {
            node<astNode> *func = functions[i];
            std::string funcName = IRContext->ASTContext->nameTable->data[func->data.data.nameTableIndex].name;
            IRContext->currentFunction = func->data.data.nameTableIndex;
            localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
            customPrint(blue, bold, bgDefault, "Generate IR for function %s, localTableIndex: %zd\n", 
                        IRContext->ASTContext->nameTable->data[IRContext->currentFunction].name, localTableIndex);

            IR_BasicBlock *funcBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
            customWarning(funcBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
            initializeBasicBlock(funcBlock, funcName.c_str());

            llError = insertNode(IR->basicBlocks, funcBlock);
            customWarning(llError == linkedListError::NO_ERRORS, IR_Error::BLOCKS_LIST_BAD_POINTER);

            error = generateFunctionIR(IRContext, func, funcBlock);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_FUNCTION_IR_ERROR);
        }
    }

    IR->totalInstructionCount = 0;
    ssize_t currentBlockIndex = IR->basicBlocks->next[0];

    while (currentBlockIndex != 0) {
        IR_BasicBlock *block = IR->basicBlocks->data[currentBlockIndex];
        block->instructionCount = block->instructions->size;
        IR->totalInstructionCount += block->instructionCount;
        currentBlockIndex = IR->basicBlocks->next[currentBlockIndex];
    }

    return IR_Error::NO_ERRORS;
}

IR_Error generateFunctionIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(node, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (node->data.type != nodeType::FUNCTION_DEFINITION) {
        customPrint(yellow, bold, bgDefault, "Expected FUNCTION_DEFINITION, got type: %d\n", (int)node->data.type);
        return IR_Error::AST_BAD_STRUCTURE;
    }

    IRContext->hasReturn = false;
    IRContext->regAllocator->stackOffset = 0;

    PUSH_REG(IR_Register::RBP);
    MOV_REG_REG(IR_Register::RBP, IR_Register::RSP);

    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    size_t localSize = IRContext->ASTContext->localTables->data[localTableIndex].size * 8;
    SUB_REG_IMM(IR_Register::RSP, localSize);

    if (!node->right || node->right->data.type != nodeType::PARAMETERS) {
        customPrint(yellow, bold, bgDefault, "No PARAMETERS node in function\n");
        return IR_Error::AST_BAD_STRUCTURE;
    }

    IR_Error error = generateStatementIR(IRContext, node->right, block);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);

    if (!IRContext->hasReturn) {
        MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
        POP_REG(IR_Register::RBP);
        RET();
    }

    customPrint(red, bold, bgDefault, "EntryPoint: %p, Block: %p\n", 
                IRContext->representation->entryPoint, block);

    block->instructionCount = block->instructions->size;
    customPrint(blue, bold, bgDefault, "Set block->instructionCount to %zd\n", block->instructionCount);

    customPrint(blue, bold, bgDefault, "Finished generating IR for function %s with %d instructions\n", 
                IRContext->ASTContext->nameTable->data[IRContext->currentFunction].name, block->instructionCount);

    return IR_Error::NO_ERRORS;
}

IR_Error generateStatementIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (!node) {
        customPrint(yellow, bold, bgDefault, "Null statement node\n");
        return IR_Error::NO_ERRORS;
    }

    customPrint(blue, bold, bgDefault, "Processing node type: %d\n", (int)node->data.type);

    switch (node->data.type) {
        case nodeType::VARIABLE_DECLARATION: {
            size_t offset = IRContext->regAllocator->stackOffset + 8;
            IRContext->regAllocator->stackOffset += 8;
            node->data.rbpOffset = offset;
        
            std::string varName = IRContext->ASTContext->nameTable->data[node->data.data.nameTableIndex].name;
            customPrint(blue, bold, bgDefault, "Variable Declaration: %s, assigned offset: %zd\n", 
                        varName.c_str(), offset);
        
            localNameTable *localTable = &IRContext->ASTContext->localTables->data[IRContext->currentFunction];
            for (size_t i = 0; i < localTable->size; i++) {
                if (localTable->elements.data[i].globalNameID == node->data.data.nameTableIndex) {
                    localTable->elements.data[i].rbpOffset = offset;
                    break;
                }
            }
        
            if (node->right && node->right->data.type == nodeType::KEYWORD && 
                node->right->data.data.keyword == Keyword::ASSIGNMENT) {
                customPrint(blue, bold, bgDefault, "Processing initializer for %s, right node type: %d, keyword: %d\n", 
                            varName.c_str(), (int)node->right->data.type, (int)node->right->data.data.keyword);
        
                IR_Register resultReg;
                IR_Error error = generateExpressionIR(IRContext, node->right->left, block, resultReg);
                if (error != IR_Error::NO_ERRORS) {
                    customWarning(false, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                    return error;
                }
        
                MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, resultReg);
                freeRegister(IRContext->regAllocator, resultReg);
            }
            break;
        }
        
        case nodeType::PARAMETERS: {
            if (node->right) {
                if (node->right->data.type != nodeType::KEYWORD || 
                    node->right->data.data.keyword != Keyword::OPERATOR_SEPARATOR) {
                    customPrint(yellow, bold, bgDefault, "Expected OPERATOR_SEPARATOR, got type: %d\n", 
                                (int)node->right->data.type);
                    return IR_Error::AST_BAD_STRUCTURE;
                }
                IR_Error error = generateStatementIR(IRContext, node->right, block);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
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

                    IR_Register conditionReg;
                    IR_Error error = generateExpressionIR(IRContext, node->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    insertNode(IRContext->representation->basicBlocks, thenBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

                    CMP_REG_IMM(conditionReg, 0);
                    JMP_CONDITION(IR_Operator::IR_JE);

                    insertNode(block->successors, *thenBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(thenBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);

                    block = thenBlock;

                    if (node->right) {
                        generateStatementIR(IRContext, node->right, block);
                    }

                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, conditionReg);
                    break;
                }

                case Keyword::ASSIGNMENT: {
                    IR_Register resultReg;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    size_t offset = getVariableOffset(IRContext, node->left->data.data.nameTableIndex);
                    customWarning(offset != 0, IR_Error::VARIABLE_NOT_FOUND);

                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, resultReg);
                    freeRegister(IRContext->regAllocator, resultReg);
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
                
                    insertNode(IRContext->representation->basicBlocks, condBlock);
                    insertNode(IRContext->representation->basicBlocks, bodyBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);
                
                    IR_Instruction jmpToCond = {
                        .op = IR_Operator::IR_JMP,
                        .operandCount = 1,
                        .firstOperand = { .type = IR_OperandType::LABEL, .label = condBlock->label }
                    };
                    insertNode(block->instructions, jmpToCond);
                    insertNode(block->successors, *condBlock);
                    insertNode(condBlock->predecessors, *block);
                
                    block = condBlock;
                    IR_Register conditionReg;
                    IR_Error error = generateExpressionIR(IRContext, node->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                
                    IR_Instruction jmpToMerge = {
                        .op = IR_Operator::IR_JE,
                        .operandCount = 1,
                        .firstOperand = { .type = IR_OperandType::LABEL, .label = mergeBlock->label }
                    };
                    insertNode(block->instructions, jmpToMerge);
                    insertNode(block->successors, *bodyBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(bodyBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);
                
                    block = bodyBlock;
                    if (node->right) {
                        error = generateStatementIR(IRContext, node->right, block);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
                    }
                
                    IR_Instruction jmpBack = {
                        .op = IR_Operator::IR_JMP,
                        .operandCount = 1,
                        .firstOperand = { .type = IR_OperandType::LABEL, .label = condBlock->label }
                    };
                    insertNode(block->instructions, jmpBack);
                    insertNode(block->successors, *condBlock);
                    insertNode(condBlock->predecessors, *block);
                
                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, conditionReg);
                    break;
                }

                case Keyword::RETURN: {
                    customPrint(blue, bold, bgDefault, "Processing RETURN statement\n");
                    IRContext->hasReturn = true;

                    if (node->right) {
                        IR_Register resultReg;
                        IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);

                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error in generateExpressionIR for RETURN\n");
                            return error;
                        }

                        if (resultReg != IR_Register::RAX) {
                            MOV_REG_REG(IR_Register::RAX, resultReg);
                        }

                        freeRegister(IRContext->regAllocator, resultReg);

                    }

                    MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
                    POP_REG(IR_Register::RBP);
                    RET();
                    
                    customPrint(blue, bold, bgDefault, "Finished processing RETURN statement\n");
                    break;
                }

                case Keyword::OUT: {
                    IR_Register resultReg;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                    
                    MOV_REG_REG(IR_Register::RDI, resultReg);
                    MOV_REG_IMM(IR_Register::RAX, 1);
                    MOV_REG_IMM(IR_Register::RSI, 1);
                    SYSCALL();
                    freeRegister(IRContext->regAllocator, resultReg);
                    break;
                }

                case Keyword::BREAK:
                case Keyword::CONTINUE:
                case Keyword::IN:
                case Keyword::FUNCTION_CALL: {
                    return IR_Error::NOT_IMPLEMENTED;
                }

                case Keyword::ABORT: {
                    MOV_REG_IMM(IR_Register::RDI, 1);
                    MOV_REG_IMM(IR_Register::RAX, 60);
                    SYSCALL();
                    break;
                }

                case Keyword::OPERATOR_SEPARATOR: {
                    customPrint(blue, bold, bgDefault, "Processing OPERATOR_SEPARATOR\n");
                    if (node->left) {
                        customPrint(blue, bold, bgDefault, "Processing left child of OPERATOR_SEPARATOR\n");
                        IR_Error error = generateStatementIR(IRContext, node->left, block);
                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error in left child of OPERATOR_SEPARATOR\n");
                            return error;
                        }
                    }
                    if (node->right) {
                        customPrint(blue, bold, bgDefault, "Processing right child of OPERATOR_SEPARATOR\n");
                        IR_Error error = generateStatementIR(IRContext, node->right, block);
                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error in right child of OPERATOR_SEPARATOR\n");
                            return error;
                        }
                    }
                    customPrint(blue, bold, bgDefault, "Finished processing OPERATOR_SEPARATOR\n");
                    break;
                }

                default: {
                    customPrint(yellow, bold, bgDefault, "Unknown keyword: %d\n", (int)node->data.data.keyword);
                    return IR_Error::AST_BAD_STRUCTURE;
                }
            }
            break;
        }

        case nodeType::STRING: {
            customPrint(yellow, bold, bgDefault, "Unexpected STRING node in statement context\n");
            return IR_Error::AST_BAD_STRUCTURE;
        }

        default:
            customPrint(yellow, bold, bgDefault, "Unknown node type: %d\n", (int)node->data.type);
            return IR_Error::AST_BAD_STRUCTURE;
    }

    return IR_Error::NO_ERRORS;
}

IR_Error generateExpressionIR(IR_Context *IRContext, node<astNode> *node, IR_BasicBlock *block, IR_Register &resultReg) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(node, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (!node) {
        return IR_Error::AST_BAD_STRUCTURE;
    }

    switch (node->data.type) {
        case nodeType::CONSTANT: {
            resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
            MOV_REG_IMM(resultReg, node->data.data.number);
            break;
        }

        case nodeType::STRING: {
            std::string varName = IRContext->ASTContext->nameTable->data[node->data.data.nameTableIndex].name;
            customPrint(blue, bold, bgDefault, "Expression STRING node: %s, nameTableIndex: %zd\n", 
                        varName.c_str(), node->data.data.nameTableIndex);
        
            resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
            size_t offset = getVariableOffset(IRContext, node->data.data.nameTableIndex);
            if (offset == 0) {
                customPrint(red, bold, bgDefault, "Error: Variable %s not found in localNameTable\n", varName.c_str());
            }
            customWarning(offset != 0, IR_Error::VARIABLE_NOT_FOUND);
            MOV_REG_MEM_REG_MINUS_IMM(resultReg, IR_Register::RBP, offset);
            customPrint(blue, bold, bgDefault, "Generated MOV instruction for variable to register %d\n", (int)resultReg);
            break;
        }

        case nodeType::KEYWORD: {
            IR_Register leftReg, rightReg;
            IR_Error error = generateExpressionIR(IRContext, node->left, block, leftReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            error = generateExpressionIR(IRContext, node->right, block, rightReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            switch (node->data.data.keyword) {
                case Keyword::ADD: {
                    ADD_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::SUB: {
                    SUB_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::MUL: {
                    IMUL_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::DIV: {
                    MOV_REG_REG(IR_Register::RAX, leftReg);
                    CQO();
                    IDIV_REG(rightReg);
                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_REG(resultReg, IR_Register::RAX);
                    freeRegister(IRContext->regAllocator, leftReg);
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::LESS:
                case Keyword::GREATER:
                case Keyword::LESS_OR_EQUAL:
                case Keyword::GREATER_OR_EQUAL:
                case Keyword::NOT_EQUAL:
                case Keyword::EQUAL: {
                    CMP_REG_REG(leftReg, rightReg);
                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 0);

                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    insertNode(IRContext->representation->basicBlocks, trueBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

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
                    insertNode(block->successors, *trueBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(trueBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, leftReg);
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::AND: {
                    CMP_REG_IMM(leftReg, 0);
                    IR_BasicBlock *falseBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(falseBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(falseBlock, "false");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    insertNode(IRContext->representation->basicBlocks, falseBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, *falseBlock);
                    insertNode(falseBlock->predecessors, *block);

                    error = generateExpressionIR(IRContext, node->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM(rightReg, 0);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, *falseBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(falseBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);

                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = falseBlock;
                    MOV_REG_IMM(resultReg, 0);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, leftReg);
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::OR: {
                    CMP_REG_IMM(leftReg, 1);
                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    insertNode(IRContext->representation->basicBlocks, trueBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, *trueBlock);
                    insertNode(trueBlock->predecessors, *block);

                    error = generateExpressionIR(IRContext, node->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM(rightReg, 1);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, *trueBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(trueBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);

                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 0);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, leftReg);
                    freeRegister(IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::NOT: {
                    IR_Register operandReg;
                    IR_Error error = generateExpressionIR(IRContext, node->right, block, operandReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 0);
                    CMP_REG_IMM(operandReg, 0);

                    IR_BasicBlock *trueBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(trueBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(trueBlock, "true");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    insertNode(IRContext->representation->basicBlocks, trueBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, *trueBlock);
                    insertNode(block->successors, *mergeBlock);
                    insertNode(trueBlock->predecessors, *block);
                    insertNode(mergeBlock->predecessors, *block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, *mergeBlock);
                    insertNode(mergeBlock->predecessors, *block);

                    block = mergeBlock;
                    freeRegister(IRContext->regAllocator, operandReg);
                    break;
                }

                case Keyword::SIN:
                case Keyword::COS:
                case Keyword::FLOOR:
                case Keyword::SQRT:
                case Keyword::DIFF:
                case Keyword::IN: {
                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
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

    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    localNameTable *localTable = &IRContext->ASTContext->localTables->data[localTableIndex];

    customPrint(blue, bold, bgDefault, "Searching for variable with nameTableIndex: %zd in localTable[%zd]\n", 
                nameTableIndex, localTableIndex);

    for (size_t i = 0; i < localTable->size; i++) {
        size_t globalID = localTable->elements.data[i].globalNameID;
        customPrint(blue, bold, bgDefault, " localTable[%zd][%zd]: globalNameID=%zd, name=%s, rbpOffset=%zd\n", 
                    localTableIndex, i, globalID, 
                    IRContext->ASTContext->nameTable->data[globalID].name, 
                    localTable->elements.data[i].rbpOffset);
        if (globalID == nameTableIndex) {
            customPrint(blue, bold, bgDefault, "Found variable %s with nameTableIndex %zd, rbpOffset=%zd\n", 
                        IRContext->ASTContext->nameTable->data[nameTableIndex].name, 
                        nameTableIndex, localTable->elements.data[i].rbpOffset);
            return localTable->elements.data[i].rbpOffset;
        }
    }

    customPrint(red, bold, bgDefault, "Variable with nameTableIndex %zd not found in localTable[%zd]\n", 
                nameTableIndex, localTableIndex);
    return 0;
}