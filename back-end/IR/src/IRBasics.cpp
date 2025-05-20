#include "IRBasics.h"
#include "customWarning.h"
#include "codegen.h"
#include "linkedListAddons.h"

#include <stdlib.h>

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeBasicBlock(IR_BasicBlock *block, const char *label) {
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);
    customWarning(label, IR_Error::LABEL_BAD_POINTER);

    block->label = (char *)label;

    initializeLinkedList(block->instructions, 0);
    block->instructionCount = 0;

    initializeLinkedList(block->successors, 0);
    initializeLinkedList(block->predecessors, 0);

    block->functionIndex = 0;

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeIR(IR *IR, linkedList<IR_BasicBlock> *blocks, IR_BasicBlock *entryPoint) {
    customWarning(IR,     IR_Error::IR_BAD_POINTER);
    customWarning(blocks, IR_Error::BLOCKS_LIST_BAD_POINTER);

    IR->basicBlocks = blocks;

    IR->entryPointIndex = 0;
    IR->entryPoint      = entryPoint;

    IR->totalInstructionCount = 0;

    return IR_Error::NO_ERRORS;
}

IR_Error destroyIR(IR *IR) {
    customWarning(IR, IR_Error::IR_BAD_POINTER);

    if (IR->basicBlocks) {
        destroyLinkedList(IR->basicBlocks);
    }

    if (IR->entryPoint) {
        destroyBasicBlock(IR->entryPoint);
        FREE_(IR->entryPoint);
    }

    IR->totalInstructionCount = 0;

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeRegisterAllocator(registerAllocator *allocator) {
    customWarning(allocator, IR_Error::REGISTER_ALLOCATOR_BAD_POINTER);
    
    allocator->registers = (IR_Register []){
        IR_Register::RAX,
        IR_Register::RBX,
        IR_Register::RCX,
        IR_Register::RDX,
        IR_Register::RSI,
        IR_Register::RDI,
        IR_Register::R8,
        IR_Register::R9,
        IR_Register::R10,
        IR_Register::R11,
        IR_Register::R12,
        IR_Register::R13,
        IR_Register::R14,
        IR_Register::R15
    };

    allocator->count = sizeof(allocator->registers) / sizeof(IR_Register);
    
    allocator->used = (bool *)calloc(allocator->count, sizeof(bool));
    customWarning(allocator->used, IR_Error::ALLOCATION_ERROR);

    allocator->isCalleeSaved = (bool *)calloc(allocator->count, sizeof(bool));
    customWarning(allocator->isCalleeSaved, IR_Error::ALLOCATION_ERROR);

    allocator->stackOffset = 0;

    for (size_t i = 0; i < allocator->count; i++) {
        switch (allocator->registers[i]) {
            case IR_Register::RBX:
            case IR_Register::R10:
            case IR_Register::R11:
            case IR_Register::R12:
            case IR_Register::R13:
            case IR_Register::R14:
            case IR_Register::R15:
                allocator->isCalleeSaved[i] = true;
                break;
            
            default:
                allocator->isCalleeSaved[i] = false;
                break;
        }
    }

    return IR_Error::NO_ERRORS;
}

IR_Error destroyRegisterAllocator(registerAllocator *allocator) {
    customWarning(allocator, IR_Error::REGISTER_ALLOCATOR_BAD_POINTER);

    if (allocator->used) {
        FREE_(allocator->used);
    }

    if (allocator->isCalleeSaved) {
        FREE_(allocator->isCalleeSaved);
    }

    allocator->count       = {};
    allocator->stackOffset = {};

    return IR_Error::NO_ERRORS;
}

IR_Register allocateRegister(IR_Context *IRContext, registerAllocator *allocator, IR_BasicBlock *block) {
    customAssert(allocator, (int) IR_Error::REGISTER_ALLOCATOR_BAD_POINTER);
    customAssert(block,     (int) IR_Error::BASIC_BLOCK_BAD_POINTER);

    for (size_t i = 0; i < allocator->count; i++) {
        if (!allocator->used[i]) {
            allocator->used[i] = true;
            
            return allocator->registers[i];
        }
    }

    size_t spillIndex = 0;

    for (size_t i = 0; i < allocator->count; i++) {
        if (!allocator->isCalleeSaved[i]) {
            spillIndex = i;
            break;
        }
    }

    IR_Register spilledRegister = allocator->registers[spillIndex];
    PUSH_REG(spilledRegister);

    allocator->stackOffset += 8;

    return spilledRegister;
}

IR_Error freeRegister(registerAllocator *allocator, IR_Register reg) {
    customWarning(allocator, IR_Error::REGISTER_ALLOCATOR_BAD_POINTER);

    for (size_t i = 0; i < allocator->count; i++) {
        if (allocator->registers[i] == reg) {
            allocator->used[i] = false;

            return IR_Error::NO_ERRORS;
        }
    }

    return IR_Error::REGISTER_NOT_FOUND;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeIRContext(IR_Context *IRContext, translationContext *context) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

    IRContext->ASTContext = context;
    
    IRContext->regAllocator = (registerAllocator *)calloc(1, sizeof(registerAllocator));
    customWarning(IRContext->regAllocator, IR_Error::ALLOCATION_ERROR);

    initializeRegisterAllocator(IRContext->regAllocator);

    IRContext->representation = (IR *)calloc(1, sizeof(IR));
    customWarning(IRContext->representation, IR_Error::ALLOCATION_ERROR);

    IRContext->currentFunction = 0;

    return IR_Error::NO_ERRORS;
}

IR_Error destroyIRContext(IR_Context *IRContext) {
    customWarning(IRContext, IR_Error::IR_CONTEXT_BAD_POINTER);

    if (IRContext->ASTContext) {
        destroyTranslationContext(IRContext->ASTContext);
    }

    if (IRContext->regAllocator) {
        destroyRegisterAllocator(IRContext->regAllocator);
    }

    if (IRContext->representation) {
        destroyIR(IRContext->representation);
    }

    IRContext->currentFunction = 0;

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //