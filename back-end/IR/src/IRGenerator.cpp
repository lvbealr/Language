#include "IRGenerator.h"
#include "codegen.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>

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

IR_Error generateASM(IR *IR, const char *filename) {
    customWarning(IR, IR_Error::IR_BAD_POINTER);

    // Открываем файл для записи
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        customPrint(red, bold, bgDefault, "Error: Could not open file %s for writing\n", filename);
        return IR_Error::IO_ERROR;
    }

    customPrint(blue, bold, bgDefault, "Generating ASM file: %s\n", filename);

    // Записываем заголовок
    outFile << "; Generated ASM from IR\n";
    outFile << "section .text\n";
    outFile << "global мэйн\n\n"; // Сохраняем исходное имя точки входа

    // Проходим по всем базовым блокам
    ssize_t currentBlockIndex = IR->basicBlocks->next[0];
    while (currentBlockIndex != 0) {
        IR_BasicBlock *block = IR->basicBlocks->data[currentBlockIndex];
        customPrint(blue, bold, bgDefault, "Generating ASM for block: %s\n", block->label ? block->label : "Unknown");

        // Записываем метку блока, сохраняя исходное имя
        std::string blockLabel = block->label ? block->label : "block_" + std::to_string(currentBlockIndex);
        outFile << blockLabel << ":\n";

        // Проходим по инструкциям блока
        ssize_t currentInstIndex = block->instructions->next[0];
        while (currentInstIndex != 0) {
            IR_Instruction &inst = block->instructions->data[currentInstIndex];
            std::string instStr = IR_OperatorToString(inst.op);

            // Специальная обработка инструкций
            if (inst.op == IR_Operator::IR_CALL) {
                // Для CALL используем имя функции из nameTable
                if (inst.firstOperand.type == IR_OperandType::FUNC_INDEX) {
                    // Предполагаем, что functionIndex соответствует nameTableIndex функции
                    // Нужно получить имя функции из IRContext->ASTContext->nameTable
                    // Для этого нужно передать IRContext в generateASM или хранить имена в IR
                    // Пока используем func_<index> для совместимости, но это можно улучшить
                    outFile << "    call func_" << inst.firstOperand.functionIndex << "\n";
                } else {
                    outFile << "    call " << IR_OperandToString(inst.firstOperand) << "\n";
                }
            } else if (inst.op == IR_Operator::IR_JMP || inst.op == IR_Operator::IR_JE || inst.op == IR_Operator::IR_JNE ||
                       inst.op == IR_Operator::IR_JL || inst.op == IR_Operator::IR_JLE ||
                       inst.op == IR_Operator::IR_JG || inst.op == IR_Operator::IR_JGE) {
                // Для переходов используем метку
                if (inst.firstOperand.type == IR_OperandType::LABEL) {
                    outFile << "    " << instStr << " " << inst.firstOperand.label << "\n";
                } else {
                    outFile << "    " << instStr << " " << IR_OperandToString(inst.firstOperand) << "\n";
                }
            } else if (inst.op == IR_Operator::IR_SYSCALL) {
                outFile << "    syscall\n";
            } else if (inst.op == IR_Operator::IR_CQO) {
                outFile << "    cqo\n";
            } else {
                // Обычные инструкции
                outFile << "    " << instStr;
                if (inst.operandCount > 0) {
                    std::string firstOp = IR_OperandToString(inst.firstOperand);
                    // Удаляем префикс "imm(" для немедленных значений
                    if (inst.firstOperand.isImmediate && inst.firstOperand.type != IR_OperandType::IMM) {
                        firstOp = firstOp.substr(4, firstOp.size() - 5);
                    }
                    outFile << " " << firstOp;
                    if (inst.operandCount > 1) {
                        std::string secondOp = IR_OperandToString(inst.secondOperand);
                        if (inst.secondOperand.isImmediate && inst.secondOperand.type != IR_OperandType::IMM) {
                            secondOp = secondOp.substr(4, secondOp.size() - 5);
                        }
                        outFile << ", " << secondOp;
                    }
                }
                outFile << "\n";
            }

            currentInstIndex = block->instructions->next[currentInstIndex];
        }

        outFile << "\n";
        currentBlockIndex = IR->basicBlocks->next[currentBlockIndex];
    }

    outFile.close();
    customPrint(blue, bold, bgDefault, "ASM file %s generated successfully\n", filename);
    return IR_Error::NO_ERRORS;
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
            printf("%s ", block->predecessors->data[predIndex]->label ? block->predecessors->data[predIndex]->label : "Unknown");
            predIndex = block->predecessors->next[predIndex];
        }

        printf("\n");
        printf("  Successors: ");

        ssize_t succIndex = block->successors->next[0];

        while (succIndex != 0) {
            printf("%s ", block->successors->data[succIndex]->label ? block->successors->data[succIndex]->label : "Unknown");
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

IR_Error generateFunctionIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    if (currentNode->data.type != nodeType::FUNCTION_DEFINITION) {
        customPrint(red, bold, bgDefault, "Expected FUNCTION_DEFINITION, got type: %d\n", (int)currentNode->data.type);
        return IR_Error::AST_BAD_STRUCTURE;
    }

    std::string funcName = IRContext->ASTContext->nameTable->data[IRContext->currentFunction].name;
    customPrint(blue, bold, bgDefault, "Generating IR for function: %s\n", funcName.c_str());

    IRContext->hasReturn = false;
    IRContext->regAllocator->stackOffset = 0;

    PUSH_REG(IR_Register::RBP);
    MOV_REG_REG(IR_Register::RBP, IR_Register::RSP);

    size_t localTableIndex = IRContext->ASTContext->functionToLocalTable[IRContext->currentFunction];
    size_t localSize = IRContext->ASTContext->localTables->data[localTableIndex].size * 8;

    // Убираем частный случай для точки входа, если не требуется дополнительное место
    // Если точка входа требует дополнительного места, это должно быть задано в AST
    customPrint(blue, bold, bgDefault, "Allocating stack: %zd bytes\n", localSize);
    SUB_REG_IMM(IR_Register::RSP, localSize);

    // Process parameters (includes body)
    if (!currentNode->right || currentNode->right->data.type != nodeType::PARAMETERS) {
        customPrint(red, bold, bgDefault, "Expected PARAMETERS node, got type: %d\n", 
                    currentNode->right ? (int)currentNode->right->data.type : -1);
        return IR_Error::AST_BAD_STRUCTURE;
    }

    customPrint(blue, bold, bgDefault, "Processing parameters\n");
    IR_Error error = generateStatementIR(IRContext, currentNode->right, block);
    if (error != IR_Error::NO_ERRORS) {
        customPrint(red, bold, bgDefault, "Error processing parameters: %d\n", (int)error);
        return error;
    }

    if (!IRContext->hasReturn) {
        customPrint(blue, bold, bgDefault, "Adding default return\n");
        MOV_REG_REG(IR_Register::RSP, IR_Register::RBP);
        POP_REG(IR_Register::RBP);
        RET();
    }

    block->instructionCount = block->instructions->size;
    customPrint(blue, bold, bgDefault, "Set block->instructionCount to %zd\n", block->instructionCount);
    customPrint(blue, bold, bgDefault, "Finished generating IR for function %s with %zd instructions\n", 
                funcName.c_str(), block->instructionCount);

    return IR_Error::NO_ERRORS;
}

IR_Error generateStatementIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);

    customPrint(blue, bold, bgDefault, "Processing node: type=%d, keyword=%d, nameTableIndex=%zd\n",
        (int)currentNode->data.type, (int)currentNode->data.data.keyword, currentNode->data.data.nameTableIndex);
    if (currentNode->data.type == nodeType::STRING && currentNode->data.data.nameTableIndex > 0) {
        customPrint(blue, bold, bgDefault, "STRING node name: %s\n",
            IRContext->ASTContext->nameTable->data[currentNode->data.data.nameTableIndex].name);
    }

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
                // Освобождаем resultReg, если он не нужен позже
                // freeRegister(IRContext, IRContext->regAllocator, resultReg);
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
                    customPrint(red, bold, bgDefault, "Error: Parameter with nameTableIndex %zd not found in localTable\n",
                                paramNode->data.data.nameTableIndex);
                    return IR_Error::VARIABLE_NOT_FOUND;
                }
    
                // Сохраняем параметр в память
                if (paramIndex < 6) {
                    customPrint(blue, bold, bgDefault, "Storing parameter %zd from %s to [rbp - %zd]\n", 
                                paramIndex, IR_RegisterToString(argRegisters[paramIndex]), offset);
                    MOV_MEM_REG_MINUS_IMM_REG(IR_Register::RBP, offset, argRegisters[paramIndex]);
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
            customPrint(blue, bold, bgDefault, "Processing FUNCTION_CALL node\n");
            if (!currentNode->right || currentNode->right->data.type != nodeType::STRING ||
                IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].type != nameType::IDENTIFIER) {
                customPrint(red, bold, bgDefault, "Invalid function name in FUNCTION_CALL\n");
                return IR_Error::AST_BAD_STRUCTURE;
            }
            std::string funcName = IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].name;
            customPrint(blue, bold, bgDefault, "Function call: %s\n", funcName.c_str());
        
            // Получаем индекс функции
            auto funcIt = IRContext->functionNameToIndex.find(funcName);
            if (funcIt == IRContext->functionNameToIndex.end()) {
                customPrint(red, bold, bgDefault, "Error: Function %s not found in functionNameToIndex\n", funcName.c_str());
                return IR_Error::AST_BAD_STRUCTURE;
            }
            size_t funcIndex = funcIt->second;
        
            // Handle arguments
            std::vector<IR_Register> argRegs;
            if (currentNode->left && currentNode->left->data.type == nodeType::KEYWORD && 
                currentNode->left->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) {
                node<astNode> *argNode = currentNode->left->left;
                while (argNode) {
                    IR_Register argReg;
                    IR_Error error = generateExpressionIR(IRContext, argNode, block, argReg);
                    if (error != IR_Error::NO_ERRORS) {
                        customPrint(red, bold, bgDefault, "Error generating IR for argument\n");
                        return error;
                    }
                    argRegs.push_back(argReg);
                    argNode = (argNode->right && argNode->right->data.type == nodeType::KEYWORD && 
                               argNode->right->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) ? 
                              argNode->right->left : nullptr;
                }
            } else if (currentNode->left) {
                IR_Register argReg;
                IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, argReg);
                if (error != IR_Error::NO_ERRORS) {
                    customPrint(red, bold, bgDefault, "Error generating IR for argument\n");
                    return error;
                }
                argRegs.push_back(argReg);
            }
        
            // Pass arguments
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
        
            // Generate call
            IR_Instruction callInst = {
                .op = IR_Operator::IR_CALL,
                .operandCount = 1,
                .firstOperand = { .type = IR_OperandType::FUNC_INDEX, .functionIndex = funcIndex }
            };
            insertNode(block->instructions, callInst);
        
            // Return result
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
                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error generating IR for RETURN expression: %d\n", (int)error);
                            return error;
                        }
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
                    if (error != IR_Error::NO_ERRORS) {
                        customPrint(red, bold, bgDefault, "Error generating IR for OUT expression: %d\n", (int)error);
                        return error;
                    }
                    // Проверяем, находится ли результат уже в rdi
                    if (resultReg != IR_Register::RDI) {
                        MOV_REG_REG(IR_Register::RDI, resultReg);
                    }
                    MOV_REG_IMM(IR_Register::RAX, 1);
                    MOV_REG_IMM(IR_Register::RSI, 1);
                    SYSCALL();
                    freeRegister(IRContext, IRContext->regAllocator, resultReg);
                    break;
                }

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
                        customPrint(blue, bold, bgDefault, "Processing OPERATOR_SEPARATOR left: type=%d, keyword=%d\n",
                                    (int)currentNode->left->data.type, (int)currentNode->left->data.data.keyword);
                        IR_Error error = generateStatementIR(IRContext, currentNode->left, block);
                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error in OPERATOR_SEPARATOR left: %d\n", (int)error);
                            return error;
                        }
                    }
                    if (currentNode->right) {
                        customPrint(blue, bold, bgDefault, "Processing OPERATOR_SEPARATOR right: type=%d, keyword=%d\n",
                                    (int)currentNode->right->data.type, (int)currentNode->right->data.data.keyword);
                        IR_Error error = generateStatementIR(IRContext, currentNode->right, block);
                        if (error != IR_Error::NO_ERRORS) {
                            customPrint(red, bold, bgDefault, "Error in OPERATOR_SEPARATOR right: %d\n", (int)error);
                            return error;
                        }
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

IR_Error generateExpressionIR(IR_Context *IRContext, node<astNode> *currentNode, IR_BasicBlock *block, IR_Register &resultReg) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(currentNode, IR_Error::NODE_BAD_POINTER);
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);

    customPrint(blue, bold, bgDefault, "Processing expression node: type=%d, keyword=%d, nameTableIndex=%zd\n",
        (int)currentNode->data.type, (int)currentNode->data.data.keyword, currentNode->data.data.nameTableIndex);
    if (currentNode->data.type == nodeType::STRING && currentNode->data.data.nameTableIndex > 0) {
        customPrint(blue, bold, bgDefault, "STRING node name: %s\n",
            IRContext->ASTContext->nameTable->data[currentNode->data.data.nameTableIndex].name);
    }

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
            // Проверка указателей
            if (!IRContext || !IRContext->ASTContext || !IRContext->ASTContext->nameTable) {
                customPrint(red, bold, bgDefault, "Error: Null pointer in IRContext or nameTable\n");
                return IR_Error::IR_CONTEXT_BAD_POINTER;
            }
        
            size_t nameTableIndex = currentNode->data.data.nameTableIndex;
            if (nameTableIndex >= IRContext->ASTContext->nameTable->currentIndex) {
                customPrint(red, bold, bgDefault, "Error: Invalid nameTableIndex %zd, max %zd\n",
                            nameTableIndex, IRContext->ASTContext->nameTable->currentIndex);
                return IR_Error::AST_BAD_STRUCTURE;
            }
        
            std::string name = IRContext->ASTContext->nameTable->data[nameTableIndex].name;
            nameType identType = IRContext->ASTContext->nameTable->data[nameTableIndex].type;
        
            if (identType != nameType::IDENTIFIER) {
                customPrint(red, bold, bgDefault, "Error: Expected variable, got %s with type %d\n",
                            name.c_str(), (int)identType);
                return IR_Error::AST_BAD_STRUCTURE;
            }
        
            size_t offset = getVariableOffset(IRContext, nameTableIndex);
            if (offset == 0) {
                customPrint(red, bold, bgDefault, "Error: Variable %s not found in localTable\n", name.c_str());
                return IR_Error::VARIABLE_NOT_FOUND;
            }
        
            // Проверяем кэш регистров
            auto cacheIt = IRContext->variableRegisterCache.find(nameTableIndex);
            if (cacheIt != IRContext->variableRegisterCache.end()) {
                resultReg = cacheIt->second;
                customPrint(blue, bold, bgDefault, "Using cached value of variable %s in %s\n",
                            name.c_str(), IR_RegisterToString(resultReg));
            } else {
                resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                customPrint(blue, bold, bgDefault, "Loading variable %s from [rbp - %zd] to %s\n",
                            name.c_str(), offset, IR_RegisterToString(resultReg));
                MOV_REG_MEM_REG_MINUS_IMM(resultReg, IR_Register::RBP, offset);
                IRContext->variableRegisterCache[nameTableIndex] = resultReg;
            }
        
            break;
        }

        case nodeType::FUNCTION_CALL: {
            customPrint(blue, bold, bgDefault, "Processing FUNCTION_CALL node\n");
            if (!currentNode->right || currentNode->right->data.type != nodeType::STRING) {
                customPrint(red, bold, bgDefault, "Invalid function name in FUNCTION_CALL\n");
                return IR_Error::AST_BAD_STRUCTURE;
            }
            std::string funcName = IRContext->ASTContext->nameTable->data[currentNode->right->data.data.nameTableIndex].name;
            customPrint(blue, bold, bgDefault, "Function call: %s\n", funcName.c_str());
        
            // Handle arguments
            std::vector<IR_Register> argRegs;
            if (currentNode->left && currentNode->left->data.type == nodeType::KEYWORD && 
                currentNode->left->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) {
                node<astNode> *argNode = currentNode->left->left;
                while (argNode) {
                    IR_Register argReg;
                    IR_Error error = generateExpressionIR(IRContext, argNode, block, argReg);
                    if (error != IR_Error::NO_ERRORS) {
                        customPrint(red, bold, bgDefault, "Error generating IR for argument\n");
                        return error;
                    }
                    argRegs.push_back(argReg);
                    argNode = (argNode->right && argNode->right->data.type == nodeType::KEYWORD && 
                               argNode->right->data.data.keyword == Keyword::ARGUMENT_SEPARATOR) ? 
                              argNode->right->left : nullptr;
                }
            } else if (currentNode->left) {
                // Single argument
                IR_Register argReg;
                IR_Error error = generateExpressionIR(IRContext, currentNode->left, block, argReg);
                if (error != IR_Error::NO_ERRORS) {
                    customPrint(red, bold, bgDefault, "Error generating IR for argument\n");
                    return error;
                }
                argRegs.push_back(argReg);
            }
        
            // Pass arguments in registers
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
        
            // Generate call instruction
            IR_Instruction callInst = {
                .op = IR_Operator::IR_CALL,
                .operandCount = 1,
                .firstOperand = { .type = IR_OperandType::FUNC_INDEX, .functionIndex = currentNode->right->data.data.nameTableIndex }
            };
            insertNode(block->instructions, callInst);
        
            // Return result in resultReg
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
                    // Гарантируем, что rightReg != RAX
                    IR_Register divReg = rightReg;
                    if (rightReg == IR_Register::RAX) {
                        divReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                        MOV_REG_REG(divReg, rightReg);
                    }
                
                    // Перемещаем leftReg в RAX, если нужно
                    if (leftReg != IR_Register::RAX) {
                        MOV_REG_REG(IR_Register::RAX, leftReg);
                    }
                
                    CQO();
                    IDIV_REG(divReg);
                
                    resultReg = allocateRegister(IRContext, IRContext->regAllocator, block);
                    MOV_REG_REG(resultReg, IR_Register::RAX);
                
                    // Освобождаем регистры, избегая двойного освобождения
                    if (leftReg != resultReg && leftReg != IR_Register::RAX) {
                        freeRegister(IRContext, IRContext->regAllocator, leftReg);
                    }
                    if (divReg != resultReg && divReg != leftReg) {
                        freeRegister(IRContext, IRContext->regAllocator, divReg);
                    }
                
                    // Обновляем кэш, если это присваивание
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

    for (size_t i = 0; i < localTable->size; i++) {
        size_t globalID = localTable->elements.data[i].globalNameID;

        if (globalID == nameTableIndex) {
            return localTable->elements.data[i].rbpOffset;
        }
    }

    return 0;
}