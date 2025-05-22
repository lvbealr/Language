#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>

#include "ASMGenerator.h"
#include "codegen.h"
#include "IRGenerator.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

static size_t getVariableOffset(IR_Context *IRContext, size_t nameTableIndex) {
    customWarning(IRContext, 0);

    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    localNameTable *localTable = &IRContext->ASTContext->localTables->data[localTableIndex];

    for (size_t i = 0; i < localTable->size; i++) {
        size_t globalID = localTable->elements.data[i].globalNameID;

        if (globalID == nameTableIndex) {
            return localTable->elements.data[i].rbpOffset;
        }
    }

    return 0;
}

static void collectFunctionNodes(node<astNode> *current, std::vector<node<astNode> *> &functions) {
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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateIR(IR_Context *IRContext) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

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
        return IR_Error::AST_BAD_STRUCTURE;
    }

    collectFunctionNodes(IRContext->ASTContext->AST->root->right, functions);
    customWarning(!functions.empty(), IR_Error::AST_BAD_STRUCTURE);

    node<astNode> *entryPointNode = nullptr;
    std::string entryPointName = IRContext->ASTContext->nameTable->data[IRContext->ASTContext->entryPoint].name;
    size_t entryPointIndex = 0;

    for (size_t i = 0; i < functions.size(); i++) {
        size_t globalNameID = functions[i]->data.data.nameTableIndex;
        std::string funcName = IRContext->ASTContext->nameTable->data[globalNameID].name;

        IRContext->functionNameToIndex[funcName] = i;
    }

    for (size_t i = 0; i < functions.size(); ++i) {
        std::string funcName = IRContext->ASTContext->nameTable->data[functions[i]->data.data.nameTableIndex].name;

        if (funcName == entryPointName) {
            entryPointNode  = functions[i];
            entryPointIndex = i;
            break;
        }
    }

    customWarning(entryPointNode, IR_Error::NODE_BAD_POINTER);

    IR_BasicBlock *entryPointBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
    customWarning(entryPointBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
    initializeBasicBlock(entryPointBlock, entryPointName.c_str());

    IR->entryPoint = entryPointBlock;
    linkedListError llError = insertNode(IR->basicBlocks, entryPointBlock);
    customWarning(llError == linkedListError::NO_ERRORS, IR_Error::BLOCKS_LIST_BAD_POINTER);

    IRContext->currentFunction = IRContext->ASTContext->entryPoint;
    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    
    IR_Error error = generateFunctionIR(IRContext, entryPointNode, entryPointBlock);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_FUNCTION_IR_ERROR);

    for (size_t i = 0; i < functions.size(); ++i) {
        if (i != entryPointIndex) {
            node<astNode> *func = functions[i];

            std::string funcName = IRContext->ASTContext->nameTable->data[func->data.data.nameTableIndex].name;
            
            IRContext->currentFunction = func->data.data.nameTableIndex;
            localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];

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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateFunctionIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (currentNode->data.type != nodeType::FUNCTION_DEFINITION) {
        return IR_Error::AST_BAD_STRUCTURE;
    }

    std::string funcName = IRContext->ASTContext->nameTable->data[IRContext->currentFunction].name;

    IRContext->hasReturn = false;
    IRContext->regAllocator->stackOffset = 0;

    PUSH_REG(IR_Register::RBP);
    MOV_REG_REG(IR_Register::RBP, IR_Register::RSP);

    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    size_t localSize = IRContext->ASTContext->localTables->data[localTableIndex].size * 8;

    if (funcName == IRContext->ASTContext->nameTable->data[IRContext->ASTContext->entryPoint].name) {
        localSize += 8; // for return value
    }

    SUB_REG_IMM(IR_Register::RSP, localSize);

    // parameters
    if (!currentNode->right || currentNode->right->data.type != nodeType::PARAMETERS) {
        return IR_Error::AST_BAD_STRUCTURE;
    }

    IR_Error error = generateStatementIR(IRContext, currentNode->right, block);
    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);    

    if (!IRContext->hasReturn) {
        MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
        POP_REG(IR_Register::RBP);
        RET();
    }

    block->instructionCount = block->instructions->size;

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateStatementIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);

    switch (currentNode->data.type) {
        case nodeType::VARIABLE_DECLARATION: {
            size_t offset = IRContext->regAllocator->stackOffset + 8;
            IRContext->regAllocator->stackOffset += 8;
            currentNode->data.rbpOffset = offset;
        
            localNameTable *localTable = &IRContext->ASTContext->localTables->data[IRContext->currentFunction];
            for (size_t i = 0; i < localTable->size; i++) {
                if (localTable->elements.data[i].globalNameID == currentNode->data.data.nameTableIndex) {
                    localTable->elements.data[i].rbpOffset = offset;
                    break;
                }
            }
        
            if (currentNode->right && currentNode->right->data.type == nodeType::KEYWORD && 
                currentNode->right->data.data.keyword == Keyword::ASSIGNMENT) {

                IR_Register resultReg;
                IR_Error error = generateExpressionIR(IRContext, currentNode->right->left, block, resultReg);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
        
                MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, resultReg);
                IRContext->variableRegisterCache[currentNode->data.data.nameTableIndex] = resultReg;
            }
        
            break;
        }
        
        case nodeType::PARAMETERS: {
            node<astNode> *paramNode = currentNode->left;
            size_t paramIndex = 0;

            static const IR_Register argRegisters[] = {
                IR_Register::RDI, IR_Register::RSI, IR_Register::RDX,
                IR_Register::RCX, IR_Register::R8, IR_Register::R9
            };

            while (paramNode && paramNode->data.type == nodeType::VARIABLE_DECLARATION) {
                size_t offset = IRContext->regAllocator->stackOffset + 8;
                IRContext->regAllocator->stackOffset += 8;
                paramNode->data.rbpOffset = offset;
    
                localNameTable *localTable = &IRContext->ASTContext->localTables->data[IRContext->currentFunction];
                bool found = false;

                for (size_t i = 0; i < localTable->size; i++) {
                    if (localTable->elements.data[i].globalNameID == paramNode->data.data.nameTableIndex) {
                        localTable->elements.data[i].rbpOffset = offset;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return IR_Error::VARIABLE_NOT_FOUND;
                }
                
                // store parameter in the register
                if (paramIndex < 6) {
                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, argRegisters[paramIndex]);
                // store parameter on stack
                } else {
                    size_t stackOffset = (paramIndex - 6) * 8 + 16;
                    MOV_REG_MEM_REG_PLUS_IMM(IR_Register::RAX, IR_Register::RBP, stackOffset);
                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, IR_Register::RAX);
                }

                paramNode = paramNode->right;
                paramIndex++;
            }
    
            if (currentNode->right) {
                IR_Error error = generateStatementIR(IRContext, currentNode->right, block);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
            }

            break;
        }

        case nodeType::FUNCTION_CALL: {
            if (!currentNode->right || currentNode->right->data.type != nodeType::STRING ||
                IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].type != nameType::IDENTIFIER) {
                return IR_Error::AST_BAD_STRUCTURE;
            }

            std::string funcName = IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].name;

            auto funcIt = IRContext->functionNameToIndex.find(funcName);
            if (funcIt == IRContext->functionNameToIndex.end()) {
                return IR_Error::AST_BAD_STRUCTURE;
            }

            size_t funcIndex = funcIt->second;

            // arguments 
            std::vector<IR_Register> argRegs;
            
            if (currentNode->left && currentNode->left->data.type == nodeType::KEYWORD && 
                currentNode->left->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) {
                node<astNode> *argNode = currentNode->left->left;

                while (argNode) {
                    IR_Register argReg;
                    IR_Error error = generateExpressionIR(IRContext, argNode, block, argReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    argRegs.push_back(argReg);
                    argNode = (argNode->right && argNode->right->data.type == nodeType::KEYWORD && 
                               argNode->right->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) ? 
                              argNode->right->left : NULL;
                }

            } else if (currentNode->left) {
                IR_Register argReg;
                IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, argReg);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                argRegs.push_back(argReg);
            }
        
            // pass arguments
            static const IR_Register argRegisters[] = {
                IR_Register::RDI, IR_Register::RSI, IR_Register::RDX,
                IR_Register::RCX, IR_Register::R8, IR_Register::R9
            };

            for (size_t i = 0; i < argRegs.size() && i < 6; ++i) {
                if (argRegs[i] != argRegisters[i]) {
                    MOV_REG_REG(argRegisters[i], argRegs[i]);
                }

                freeRegister(IRContext, IRContext->regAllocator, argRegs[i]);
            }

            if (argRegs.size() > 6) {
                for (size_t i = 6; i < argRegs.size(); ++i) {
                    PUSH_REG(argRegs[i]);
                    freeRegister(IRContext, IRContext->regAllocator, argRegs[i]);
                }
            }
        
            // generate call
            IR_Instruction callInst = {
                .op = IR_Operator::IR_CALL,
                .operandCount = 1,
                .firstOperand = { .type = IR_OperandType::FUNC_INDEX, .functionIndex = funcIndex }
            };

            insertNode(block->instructions, callInst);
        
            // return result
            IR_Register resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
            MOV_REG_REG(resultReg, IR_Register::RAX);
            break;
        }

        case nodeType::KEYWORD: {
            switch (currentNode->data.data.keyword) {
                case Keyword::IF: {
                    IR_BasicBlock *thenBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(thenBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(thenBlock, "then");

                    IR_BasicBlock *mergeBlock = (IR_BasicBlock *)calloc(1, sizeof(IR_BasicBlock));
                    customWarning(mergeBlock, IR_Error::BASIC_BLOCK_BAD_POINTER);
                    initializeBasicBlock(mergeBlock, "merge");

                    IR_Register conditionReg;
                    IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    insertNode(IRContext->representation->basicBlocks, thenBlock);
                    insertNode(IRContext->representation->basicBlocks, mergeBlock);

                    CMP_REG_IMM(conditionReg, 0);
                    JMP_CONDITION(IR_Operator::IR_JE);

                    insertNode(block->successors, thenBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(thenBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);

                    block = thenBlock;

                    if (currentNode->right) {
                        generateStatementIR(IRContext, currentNode->right, block);
                    }

                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, conditionReg);
                    break;
                }

                case Keyword::ASSIGNMENT: {
                    IR_Register resultReg;
                    IR_Error error = generateExpressionIR(IRContext, currentNode->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                
                    size_t offset = getVariableOffset(IRContext, currentNode->left->data.data.nameTableIndex);
                    customWarning(offset != 0, IR_Error::VARIABLE_NOT_FOUND);
                
                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, resultReg);
                    IRContext->variableRegisterCache[currentNode->left->data.data.nameTableIndex] = resultReg;
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
                    insertNode(block->successors, condBlock);
                    insertNode(condBlock->predecessors, block);
                
                    block = condBlock;
                    IR_Register conditionReg;

                    IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, conditionReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                
                    IR_Instruction jmpToMerge = {
                        .op = IR_Operator::IR_JE,
                        .operandCount = 1,
                        .firstOperand = { .type = IR_OperandType::LABEL, .label = mergeBlock->label }
                    };

                    insertNode(block->instructions, jmpToMerge);
                    insertNode(block->successors, bodyBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(bodyBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);
                
                    block = bodyBlock;

                    if (currentNode->right) {
                        error = generateStatementIR(IRContext, currentNode->right, block);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
                    }
                
                    IR_Instruction jmpBack = {
                        .op = IR_Operator::IR_JMP,
                        .operandCount = 1,
                        .firstOperand = { .type = IR_OperandType::LABEL, .label = condBlock->label }
                    };

                    insertNode(block->instructions, jmpBack);
                    insertNode(block->successors, condBlock);
                    insertNode(condBlock->predecessors, block);
                
                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, conditionReg);
                    break;
                }

                case Keyword::RETURN: {
                    IRContext->hasReturn = true;
                    if (currentNode->right) {
                        IR_Register resultReg;
                        IR_Error error = generateExpressionIR(IRContext, currentNode->right, block, resultReg);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                        if (resultReg != IR_Register::RAX) {
                            MOV_REG_REG(IR_Register::RAX, resultReg);
                        }

                        freeRegister(IRContext, IRContext->regAllocator, resultReg);
                    }

                    MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
                    POP_REG(IR_Register::RBP);
                    RET();
                    break;
                }

                case Keyword::OUT: {
                    IR_Register resultReg;
                    IR_Error error = generateExpressionIR(IRContext, currentNode->right, block, resultReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    if (resultReg != IR_Register::RDI) {
                        MOV_REG_REG(IR_Register::RDI, resultReg);
                    }

                    // TODO:
                    MOV_REG_IMM(IR_Register::RAX, 1);
                    MOV_REG_IMM(IR_Register::RSI, 1);
                    SYSCALL();

                    freeRegister(IRContext, IRContext->regAllocator, resultReg);

                    break;
                }

                // TODO:
                case Keyword::BREAK:
                case Keyword::CONTINUE:
                case Keyword::IN:
                case Keyword::ABORT: {
                    MOV_REG_IMM(IR_Register::RDI, 1);
                    MOV_REG_IMM(IR_Register::RAX, 60);
                    SYSCALL();

                    break;
                }

                case Keyword::OPERATOR_SEPARATOR: {
                    if (currentNode->left) {
                        IR_Error error = generateStatementIR(IRContext, currentNode->left, block);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
                    }

                    if (currentNode->right) {
                        IR_Error error = generateStatementIR(IRContext, currentNode->right, block);
                        customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_STATEMENT_IR_ERROR);
                    }

                    break;
                }

                default: {
                    return IR_Error::AST_BAD_STRUCTURE;
                }
            }

            break;
        }

        case nodeType::STRING: {
            return IR_Error::AST_BAD_STRUCTURE;
        }

        default:
            return IR_Error::AST_BAD_STRUCTURE;
    }

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateExpressionIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block, IR_Register &resultReg) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (!currentNode) {
        return IR_Error::AST_BAD_STRUCTURE;
    }

    switch (currentNode->data.type) {
        case nodeType::CONSTANT: {
            resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
            MOV_REG_IMM(resultReg, currentNode->data.data.number);

            break;
        }

        case nodeType::STRING: {
            if (!IRContext || !IRContext->ASTContext || !IRContext->ASTContext->nameTable) {
                return IR_Error::IR_CONTEXT_BAD_POINTER;
            }
        
            size_t nameTableIndex = currentNode->data.data.nameTableIndex;

            if (nameTableIndex >= IRContext->ASTContext->nameTable->currentIndex) {
                return IR_Error::AST_BAD_STRUCTURE;
            }
        
            std::string name = IRContext->ASTContext->nameTable->data[nameTableIndex].name;
            nameType identType = IRContext->ASTContext->nameTable->data[nameTableIndex].type;
        
            if (identType != nameType::IDENTIFIER) {
                return IR_Error::AST_BAD_STRUCTURE;
            }
        
            size_t offset = getVariableOffset(IRContext, nameTableIndex);
            if (offset == 0) {
                return IR_Error::VARIABLE_NOT_FOUND;
            }
        
            // check register cache
            auto cacheIt = IRContext->variableRegisterCache.find(nameTableIndex);

            if (cacheIt != IRContext->variableRegisterCache.end()) {
                resultReg = cacheIt->second;
            } else {
                resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                MOV_REG_MEM_REG_MINUS_IMM(resultReg, IR_Register::RBP, offset);
                IRContext->variableRegisterCache[nameTableIndex] = resultReg;
            }
        
            break;
        }

        case nodeType::FUNCTION_CALL: {
            if (!currentNode->right || currentNode->right->data.type != nodeType::STRING) {
                return IR_Error::AST_BAD_STRUCTURE;
            }

            std::string funcName = IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].name;
        
            // handle arguments
            std::vector<IR_Register> argRegs;

            if (currentNode->left && currentNode->left->data.type == nodeType::KEYWORD && 
                currentNode->left->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) {
                node<astNode> *argNode = currentNode->left->left;

                while (argNode) {
                    IR_Register argReg;
                    IR_Error error = generateExpressionIR(IRContext, argNode, block, argReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    argRegs.push_back(argReg);
                    argNode = (argNode->right && argNode->right->data.type == nodeType::KEYWORD && 
                               argNode->right->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) ? 
                              argNode->right->left : NULL;
                }

            } else if (currentNode->left) {
                // single argument
                IR_Register argReg;

                IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, argReg);
                customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);
                
                argRegs.push_back(argReg);
            }
        
            // pass arguments in registers
            static const IR_Register argRegisters[] = {
                IR_Register::RDI, IR_Register::RSI, IR_Register::RDX,
                IR_Register::RCX, IR_Register::R8, IR_Register::R9
            };

            for (size_t i = 0; i < argRegs.size() && i < 6; ++i) {
                if (argRegs[i] != argRegisters[i]) {
                    MOV_REG_REG(argRegisters[i], argRegs[i]);
                }

                freeRegister(IRContext, IRContext->regAllocator, argRegs[i]);
            }

            if (argRegs.size() > 6) {
                for (size_t i = 6; i < argRegs.size(); ++i) {
                    PUSH_REG(argRegs[i]);
                    freeRegister(IRContext, IRContext->regAllocator, argRegs[i]);
                }
            }
        
            // generate call instruction
            IR_Instruction callInst = {
                .op = IR_Operator::IR_CALL,
                .operandCount = 1,
                .firstOperand = { .type = IR_OperandType::FUNC_INDEX, .functionIndex = currentNode->right->data.data.nameTableIndex }
            };

            insertNode(block->instructions, callInst);
        
            // return result in resultReg
            resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
            MOV_REG_REG(resultReg, IR_Register::RAX);

            break;
        }

        case nodeType::KEYWORD: {
            IR_Register leftReg, rightReg;
            IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, leftReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            error = generateExpressionIR(IRContext, currentNode->right, block, rightReg);
            customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

            switch (currentNode->data.data.keyword) {
                case Keyword::ADD: {
                    ADD_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);

                    break;
                }

                case Keyword::SUB: {
                    SUB_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);

                    break;
                }

                case Keyword::MUL: {
                    IMUL_REG_REG(leftReg, rightReg);
                    resultReg = leftReg;
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);

                    break;
                }

                case Keyword::DIV: {
                    IR_Register divReg = rightReg;
                    if (rightReg == IR_Register::RAX) {
                        divReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                        MOV_REG_REG(divReg, rightReg);
                    }
                
                    if (leftReg != IR_Register::RAX) {
                        MOV_REG_REG(IR_Register::RAX, leftReg);
                    }
                
                    CQO();
                    IDIV_REG(divReg);
                
                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_REG(resultReg, IR_Register::RAX);
                
                    if (leftReg != resultReg && leftReg != IR_Register::RAX) {
                        freeRegister(IRContext, IRContext->regAllocator, leftReg);
                    }
                    if (divReg != resultReg && divReg != leftReg) {
                        freeRegister(IRContext, IRContext->regAllocator, divReg);
                    }
                
                    // if assignment => update variable register cache
                    node<astNode> *parent = currentNode;

                    while (parent && parent->data.type != nodeType::VARIABLE_DECLARATION && parent->data.type != nodeType::KEYWORD) {
                        parent = parent->parent;
                    }

                    if (parent && parent->data.type == nodeType::VARIABLE_DECLARATION) {
                        IRContext->variableRegisterCache[parent->data.data.nameTableIndex] = resultReg;
                    } else if (parent && parent->data.type == nodeType::KEYWORD && parent->data.data.keyword == Keyword::ASSIGNMENT) {
                        IRContext->variableRegisterCache[parent->left->data.data.nameTableIndex] = resultReg;
                    }
                
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

                    switch (currentNode->data.data.keyword) {
                        case Keyword::EQUAL:            jmpOp = IR_Operator::IR_JE;     break;
                        case Keyword::LESS:             jmpOp = IR_Operator::IR_JL;     break;
                        case Keyword::GREATER:          jmpOp = IR_Operator::IR_JG;     break;
                        case Keyword::LESS_OR_EQUAL:    jmpOp = IR_Operator::IR_JLE;    break;
                        case Keyword::GREATER_OR_EQUAL: jmpOp = IR_Operator::IR_JGE;    break;
                        case Keyword::NOT_EQUAL:        jmpOp = IR_Operator::IR_JNE;    break;
                        default: return IR_Error::AST_BAD_STRUCTURE;
                    }

                    JMP_CONDITION(jmpOp);
                    insertNode(block->successors, trueBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(trueBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, leftReg);
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);

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
                    insertNode(block->successors, falseBlock);
                    insertNode(falseBlock->predecessors, block);

                    error = generateExpressionIR(IRContext, currentNode->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM(rightReg, 0);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, falseBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(falseBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);

                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = falseBlock;
                    MOV_REG_IMM(resultReg, 0);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, leftReg);
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);
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
                    insertNode(block->successors, trueBlock);
                    insertNode(trueBlock->predecessors, block);

                    error = generateExpressionIR(IRContext, currentNode->right, block, rightReg);
                    customWarning(error == IR_Error::NO_ERRORS, IR_Error::GENERATE_EXPRESSION_IR_ERROR);

                    CMP_REG_IMM(rightReg, 1);
                    JMP_CONDITION(IR_Operator::IR_JE);
                    insertNode(block->successors, trueBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(trueBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);

                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_IMM(resultReg, 0);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, leftReg);
                    freeRegister(IRContext, IRContext->regAllocator, rightReg);
                    break;
                }

                case Keyword::NOT: {
                    IR_Register operandReg;
                    IR_Error error = generateExpressionIR(IRContext, currentNode->right, block, operandReg);
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
                    insertNode(block->successors, trueBlock);
                    insertNode(block->successors, mergeBlock);
                    insertNode(trueBlock->predecessors, block);
                    insertNode(mergeBlock->predecessors, block);

                    block = trueBlock;
                    MOV_REG_IMM(resultReg, 1);
                    JMP();
                    insertNode(block->successors, mergeBlock);
                    insertNode(mergeBlock->predecessors, block);

                    block = mergeBlock;
                    freeRegister(IRContext, IRContext->regAllocator, operandReg);
                    break;
                }

                // TODO:
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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //