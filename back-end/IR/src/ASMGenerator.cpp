#include <fstream>

#include "ASMGenerator.h"
#include "customWarning.h"
#include "IRBasics.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

const char *IR_OperatorToString(IR_Operator op) {
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

const char *IR_RegisterToString(IR_Register reg) {
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

std::string IR_OperandToString(const IR_Operand &operand) {
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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error generateASM(IR_Context *IRContext, IR *IR, const char *filename) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(IR, IR_Error::IR_BAD_POINTER);

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        return IR_Error::IO_ERROR;
    }

    customPrint(blue, bold, bgDefault, "Generating ASM file: %s\n", filename);

    outFile << "; Generated ASM from IR\n";
    outFile << "section .text\n";
    outFile << "global " << IR->entryPoint->label << "\n\n";

    ssize_t currentBlockIndex = IR->basicBlocks->next[0];
    while (currentBlockIndex != 0) {
        IR_BasicBlock *block = IR->basicBlocks->data[currentBlockIndex];

        std::string blockLabel = block->label ? block->label : "block_" + std::to_string(currentBlockIndex);
        outFile << blockLabel << ":\n";

        ssize_t currentInstIndex = block->instructions->next[0];
        while (currentInstIndex != 0) {
            IR_Instruction &inst = block->instructions->data[currentInstIndex];
            std::string instStr = IR_OperatorToString(inst.op);

            if (inst.op == IR_Operator::IR_CALL) {
                if (inst.firstOperand.type == IR_OperandType::FUNC_INDEX) {
                    size_t funcIndex = inst.firstOperand.functionIndex;
                    std::string funcName;

                    for (const auto &pair : IRContext->functionNameToIndex) {
                        if (pair.second == funcIndex) {
                            funcName = pair.first;
                            break;
                        }
                    }

                    if (funcName.empty()) {
                        outFile.close();
                        return IR_Error::AST_BAD_STRUCTURE;
                    }

                    outFile << "    call " << funcName << "\n";
                } else {
                    outFile << "    call " << IR_OperandToString(inst.firstOperand) << "\n";
                }

            } else if (inst.op == IR_Operator::IR_JMP || inst.op == IR_Operator::IR_JE || inst.op == IR_Operator::IR_JNE ||
                       inst.op == IR_Operator::IR_JL || inst.op == IR_Operator::IR_JLE ||
                       inst.op == IR_Operator::IR_JG || inst.op == IR_Operator::IR_JGE) {

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
                outFile << "    " << instStr;

                if (inst.operandCount > 0) {
                    std::string firstOp = IR_OperandToString(inst.firstOperand);
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