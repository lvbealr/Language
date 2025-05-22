#include <stdlib.h>
#include <string.h>

#include "customWarning.h"
#include "codegen.h"
#include "IRBasics.h"
#include "linkedListAddons.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeBasicBlock(IR_BasicBlock *block, const char *label) {
    customWarning(block, IR_Error::BASIC_BLOCK_BAD_POINTER);
    customWarning(label, IR_Error::LABEL_BAD_POINTER);

    block->label = strdup(label);

    block->instructions = (linkedList<IR_Instruction> *)calloc(1, sizeof(linkedList<IR_Instruction>));
    customWarning(block->instructions, IR_Error::BASIC_BLOCK_BAD_POINTER);
    initializeLinkedList(block->instructions, 10);

    block->instructionCount = 0;

    block->successors = (linkedList<IR_BasicBlock *> *)calloc(1, sizeof(linkedList<IR_BasicBlock *>));
    customWarning(block->successors, IR_Error::BASIC_BLOCK_BAD_POINTER);
    initializeLinkedList(block->successors, 10);

    block->predecessors = (linkedList<IR_BasicBlock *> *)calloc(1, sizeof(linkedList<IR_BasicBlock *>));
    customWarning(block->predecessors, IR_Error::BASIC_BLOCK_BAD_POINTER);
    initializeLinkedList(block->predecessors, 10);

    block->functionIndex = 0;

    return IR_Error::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

IR_Error initializeIR(IR *IR, linkedList<IR_BasicBlock *> *blocks, IR_BasicBlock *entryPoint) {
    customWarning(IR,     IR_Error::IR_BAD_POINTER);
    customWarning(blocks, IR_Error::BLOCKS_LIST_BAD_POINTER);

    IR->basicBlocks = blocks;

    IR->entryPointIndex = 0;
    IR->entryPoint      = entryPoint;

    insertNode(blocks, entryPoint);

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
    
    IR_Register availableRegs[] = {
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

    allocator->count = sizeof(availableRegs) / sizeof(IR_Register);
    allocator->registers = (IR_Register *)calloc(allocator->count, sizeof(IR_Register));
    customWarning(allocator->registers, IR_Error::ALLOCATION_ERROR);

    memcpy(allocator->registers, availableRegs, allocator->count * sizeof(IR_Register));

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

IR_Error freeRegister(IR_Context *context, registerAllocator *allocator, IR_Register reg) {
    customWarning(context, IR_Error::IR_CONTEXT_BAD_POINTER);
    customWarning(allocator, IR_Error::REGISTER_ALLOCATOR_BAD_POINTER);

    for (auto it = context->variableRegisterCache.begin(); it != context->variableRegisterCache.end();) {
        if (it->second == reg) {
            it = context->variableRegisterCache.erase(it);
        } else {
            ++it;
        }
    }

    return IR_Error::NO_ERRORS;
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