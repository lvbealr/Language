#ifndef LINKED_LIST_ADDONS_H_
#define LINKED_LIST_ADDONS_H_

#include "linkedList.h"
#include "IRBasics.h"
#include "customWarning.h"

inline linkedListError destroyOperand(IR_Operand *operand) {
    if (operand->type == IR_OperandType::LABEL && operand->label) {
        FREE_(operand->label);
    }

    operand->type        = IR_OperandType::NONE;
    operand->isImmediate = false;

    return linkedListError::NO_ERRORS;
}

inline linkedListError destroyInstruction(IR_Instruction *instruction) {
    destroyOperand(&instruction->firstOperand);
    destroyOperand(&instruction->secondOperand);

    instruction->nextInstruction = NULL;
    instruction->prevInstruction = NULL;

    return linkedListError::NO_ERRORS;
}

inline linkedListError destroyBasicBlock(IR_BasicBlock *block) {
    if (block->label) {
        FREE_(block->label);
    }

    if (block->instructions) {
        ssize_t current = block->instructions->next[0];

        while (current != 0) {
            destroyInstruction(&block->instructions->data[current]);
            current = block->instructions->next[current];
        }

        destroyLinkedList(block->instructions);
        FREE_(block->instructions);
    }

    if (block->successors) {
        destroyLinkedList(block->successors);
        FREE_(block->successors);
    }

    if (block->predecessors) {
        destroyLinkedList(block->predecessors);
        FREE_(block->predecessors);
    }

    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError insertNode<IR_Instruction>(linkedList<IR_Instruction> *list, IR_Instruction data) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);

    if (list->freeNode == 0 || list->freeNode >= list->capacity) {
        linkedListError resizeError = resizeList(list);
        CHECK_FOR_NULL(resizeError == linkedListError::NO_ERRORS, return resizeError);
    }

    ssize_t newNodeIndex = list->freeNode;

    customWarning(newNodeIndex > 0 && newNodeIndex < list->capacity, linkedListError::BAD_FREE_NODE);
    customWarning(list->prev[newNodeIndex] == -1, linkedListError::BAD_FREE_NODE);

    list->freeNode           = list->next[newNodeIndex];
    list->data[newNodeIndex] = data;

    list->data[newNodeIndex].nextInstruction = NULL;
    list->data[newNodeIndex].prevInstruction = NULL;

    ssize_t insertAfter = list->newIndex;

    list->next[newNodeIndex] = list->next[insertAfter];
    list->prev[newNodeIndex] = insertAfter;

    if (list->next[insertAfter] != 0) {
        list->prev[list->next[insertAfter]] = newNodeIndex;
        list->data[list->next[insertAfter]].prevInstruction = &list->data[newNodeIndex];
    } else {
        list->prev[0] = newNodeIndex;
    }

    list->next[insertAfter] = newNodeIndex;

    if (insertAfter != 0) {
        list->data[insertAfter].nextInstruction = &list->data[newNodeIndex];
    }

    if (insertAfter == 0) {
        list->prev[newNodeIndex] = 0;

        if (list->next[0] == newNodeIndex && list->prev[0] == newNodeIndex) {
            list->next[newNodeIndex] = 0;
        }
    }

    list->size++;
    list->newIndex = newNodeIndex;

#ifndef NDEBUG
    verifyLinkedList(list);
#endif

    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError deleteNode<IR_Instruction>(linkedList<IR_Instruction> *list, ssize_t index) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->next[index] != -1 || list->prev[index] != -1, linkedListError::BAD_INDEX);

    destroyInstruction(&list->data[index]);

    ssize_t prevIndex = list->prev[index];
    ssize_t nextIndex = list->next[index];

    if (prevIndex != 0) {
        list->next[prevIndex] = nextIndex;

        if (nextIndex != 0) {
            list->data[nextIndex].prevInstruction = &list->data[prevIndex];
        } else {
            list->data[prevIndex].nextInstruction = NULL;
        }

    } else {
        list->next[0] = nextIndex;
    }

    if (nextIndex != 0) {
        list->prev[nextIndex] = prevIndex;

        if (prevIndex != 0) {
            list->data[prevIndex].nextInstruction = &list->data[nextIndex];
        } else {
            list->data[nextIndex].prevInstruction = NULL;
        }
    } else {
        list->prev[0] = prevIndex;
    }

    list->prev[index] = -1;
    list->next[index] = list->freeNode;
    list->freeNode    = index;

    list->size--;

    linkedListError resizeError = resizeList(list);
    CHECK_FOR_NULL(resizeError == linkedListError::NO_ERRORS, return resizeError);

#ifndef NDEBUG
    verifyLinkedList(list);
#endif

    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError getNodeValue<IR_Instruction>(linkedList<IR_Instruction> *list, ssize_t index, IR_Instruction *value) {
    customWarning(list,  linkedListError::LIST_BAD_POINTER);
    customWarning(value, linkedListError::DATA_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->prev[index] != -1 || list->next[index] != -1, linkedListError::BAD_INDEX);

    *value = list->data[index];
    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError insertNode<IR_BasicBlock*>(linkedList<IR_BasicBlock*> *list, IR_BasicBlock* data) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);
    customWarning(data, linkedListError::DATA_BAD_POINTER);

    if (list->freeNode == 0 || list->freeNode >= list->capacity) {
        linkedListError resizeError = resizeList(list);
        CHECK_FOR_NULL(resizeError == linkedListError::NO_ERRORS, return resizeError);
    }

    ssize_t newNodeIndex = list->freeNode;

    customWarning(newNodeIndex > 0 && newNodeIndex < list->capacity, linkedListError::BAD_FREE_NODE);
    customWarning(list->prev[newNodeIndex] == -1, linkedListError::BAD_FREE_NODE);

    list->freeNode = list->next[newNodeIndex];
    list->data[newNodeIndex] = data;

    customWarning(list->data[newNodeIndex]->instructions, linkedListError::DATA_BAD_POINTER);
    customWarning(list->data[newNodeIndex]->successors, linkedListError::DATA_BAD_POINTER);
    customWarning(list->data[newNodeIndex]->predecessors, linkedListError::DATA_BAD_POINTER);

    ssize_t insertAfter = list->newIndex;

    list->next[newNodeIndex] = list->next[insertAfter];
    list->prev[newNodeIndex] = insertAfter;

    if (list->next[insertAfter] != 0) {
        list->prev[list->next[insertAfter]] = newNodeIndex;
    } else {
        list->prev[0] = newNodeIndex;
    }

    list->next[insertAfter] = newNodeIndex;

    if (insertAfter == 0) {
        list->prev[newNodeIndex] = 0;
        if (list->next[0] == newNodeIndex && list->prev[0] == newNodeIndex) {
            list->next[newNodeIndex] = 0;
        }
    }

    list->size++;
    list->newIndex = newNodeIndex;

#ifndef NDEBUG
    verifyLinkedList(list);
#endif

    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError deleteNode<IR_BasicBlock>(linkedList<IR_BasicBlock> *list, ssize_t index) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->prev[index] != -1 || list->next[index] != -1, linkedListError::BAD_INDEX);

    destroyBasicBlock(&list->data[index]);

    ssize_t prevIndex = list->prev[index];
    ssize_t nextIndex = list->next[index];

    if (prevIndex != 0) {
        list->next[prevIndex] = nextIndex;
    } else {
        list->next[0] = nextIndex;
    }

    if (nextIndex != 0) {
        list->prev[nextIndex] = prevIndex;
    } else {
        list->prev[0] = prevIndex;
    }

    list->prev[index] = -1;
    list->next[index] = list->freeNode;
    list->freeNode = index;

    list->size--;

    linkedListError resizeError = resizeList(list);
    CHECK_FOR_NULL(resizeError == linkedListError::NO_ERRORS, return resizeError);

#ifndef NDEBUG
    verifyLinkedList(list);
#endif

    return linkedListError::NO_ERRORS;
}

template<>
inline linkedListError getNodeValue<IR_BasicBlock>(linkedList<IR_BasicBlock> *list, ssize_t index, IR_BasicBlock *value) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);
    customWarning(value, linkedListError::DATA_BAD_POINTER);
    customWarning(index > 0 && index < list->capacity, linkedListError::BAD_INDEX);
    customWarning(list->prev[index] != -1 || list->next[index] != -1, linkedListError::BAD_INDEX);

    *value = list->data[index];
    return linkedListError::NO_ERRORS;
}

#endif // LINKED_LIST_ADDONS_H_