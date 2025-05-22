#include <stdio.h>

#include "ASMGenerator.h"
#include "debugUtils.h"
#include "IRBasics.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

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

// -------------------------------------------------------------------------------------------------------------------------------------------------- //