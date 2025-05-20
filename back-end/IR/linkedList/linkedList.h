#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>

#include "customWarning.h"

#define FREE_(ptr) do { \
    free(ptr);          \
    ptr = NULL;         \
} while (0)

#define CHECK_FOR_NULL(ptr, ...) do { \
    if (!(ptr)) {                     \
        __VA_ARGS__;                  \
    }                                 \
} while (0)

template<typename T>
struct linkedList {
    T       *data = {};
    ssize_t *prev = {};
    ssize_t *next = {};

    ssize_t  capacity = {};
    ssize_t  size     = {};
    ssize_t  freeNode = {};
    ssize_t  newIndex = {};
};

enum class linkedListError {
    NO_ERRORS                     =  0,
    LIST_BAD_POINTER              =  1,
    PREVIOUS_BAD_POINTER          =  2,
    NEXT_BAD_POINTER              =  3,
    DATA_BAD_POINTER              =  4,
    BAD_CAPACITY                  =  5,
    BAD_HEAD                      =  6,
    BAD_TAIL                      =  7,
    BAD_INDEX                     =  8,
    BAD_GET_NODE_POINTER          =  9,
    BAD_FREE_NODE                 = 10,
    BAD_DATA_POINTER              = 11,
    RESIZE_ERROR                  = 12,
};

template<typename T>
linkedListError initializeLinkedList(linkedList<T> *list, size_t capacity) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);

    list->capacity = (ssize_t)capacity + 1;

    list->data = (T *)calloc((size_t)list->capacity, sizeof(T));
    customWarning(list->data, linkedListError::DATA_BAD_POINTER);

    list->prev = (ssize_t *)calloc((size_t)list->capacity, sizeof(ssize_t));

    CHECK_FOR_NULL(list->prev, FREE_(list->data); customWarning(list->prev, linkedListError::PREVIOUS_BAD_POINTER));

    list->next = (ssize_t *)calloc((size_t)list->capacity, sizeof(ssize_t));

    CHECK_FOR_NULL(list->next,
                   FREE_(list->data);
                   FREE_(list->prev);
                   customWarning(list->next, linkedListError::NEXT_BAD_POINTER));

    list->prev[0] = 0;
    list->next[0] = 0;

    list->freeNode = 1;
    list->newIndex = 0;
    list->size     = 0;

    for (ssize_t nodeIndex = list->freeNode; nodeIndex < list->capacity - 1; nodeIndex++) {
        list->next[nodeIndex] = nodeIndex + 1;
        list->prev[nodeIndex] = -1;
    }

    list->next[list->capacity - 1] = 0;
    list->prev[list->capacity - 1] = -1;

    return linkedListError::NO_ERRORS;
}

template<typename T>
linkedListError destroyLinkedList(linkedList<T> *list) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);

    list->capacity = -1;
    list->freeNode = -1;
    list->newIndex =  0;
    list->size     =  0;

    customWarning(list->data, linkedListError::DATA_BAD_POINTER);
    FREE_(list->data);

    customWarning(list->prev, linkedListError::PREVIOUS_BAD_POINTER);
    FREE_(list->prev);

    customWarning(list->next, linkedListError::NEXT_BAD_POINTER);
    FREE_(list->next);

    return linkedListError::NO_ERRORS;
}

#ifndef NDEBUG
    template<typename T>
    linkedListError verifyLinkedList(linkedList<T> *list) {
        customWarning(list, linkedListError::LIST_BAD_POINTER);
    
        customWarning(list->data, linkedListError::DATA_BAD_POINTER);
        customWarning(list->prev, linkedListError::PREVIOUS_BAD_POINTER);
        customWarning(list->next, linkedListError::NEXT_BAD_POINTER);

        customWarning(list->capacity >= 0, linkedListError::BAD_CAPACITY);

        customWarning(list->next[0] >= 0 && list->next[0] < list->capacity, linkedListError::BAD_HEAD);
        customWarning(list->prev[0] >= 0 && list->prev[0] < list->capacity, linkedListError::BAD_TAIL);

        ssize_t freeIndex = list->freeNode;

        while (freeIndex) {
            customWarning(list->prev[freeIndex] == -1, linkedListError::BAD_FREE_NODE);
            freeIndex = list->next[freeIndex];
        }

        return linkedListError::NO_ERRORS;
    }
#endif

template<typename T>
linkedListError resizeList(linkedList<T> *list) {
    customWarning(list, linkedListError::LIST_BAD_POINTER);

    ssize_t newCapacity = -1;

    if (list->size >= list->capacity - 1) {
        newCapacity = list->capacity * 2;
    } else if (list->size < (list->capacity - 1) / 4 && list->capacity > 1) {
        newCapacity = list->capacity / 2;
    } else {
        return linkedListError::NO_ERRORS;
    }

    T       *newData = (T *)      realloc(list->data, (size_t)newCapacity * sizeof(T));
    ssize_t *newPrev = (ssize_t *)realloc(list->prev, (size_t)newCapacity * sizeof(ssize_t));
    ssize_t *newNext = (ssize_t *)realloc(list->next, (size_t)newCapacity * sizeof(ssize_t));

    CHECK_FOR_NULL(newData && newPrev && newNext,
                   FREE_(newData);
                   FREE_(newPrev);
                   FREE_(newNext);
                   return linkedListError::DATA_BAD_POINTER);

    list->data = newData;
    list->prev = newPrev;
    list->next = newNext;

    if (newCapacity > list->capacity) {
        for (ssize_t nodeIndex = list->capacity; nodeIndex < newCapacity - 1; nodeIndex++) {
            list->next[nodeIndex] = nodeIndex + 1;
            list->prev[nodeIndex] = -1;
        }

        list->next[newCapacity - 1] = 0;
        list->prev[newCapacity - 1] = -1;

        if (list->freeNode == 0) {
            list->freeNode = list->capacity;
        } else {
            ssize_t lastFree = list->freeNode;

            while (list->next[lastFree] != 0) {
                lastFree = list->next[lastFree];
            }

            list->next[lastFree] = list->capacity;
        }
    }

    list->capacity = newCapacity;

    #ifndef NDEBUG
        verifyLinkedList(list);
    #endif

    return linkedListError::NO_ERRORS;
}

template<typename T>
linkedListError destroyNode(T node) {
    if (node) {
        FREE_(node->data);
        FREE_(node);
    }

    return linkedListError::NO_ERRORS;
}

template<typename T>
linkedListError insertNode(linkedList<T> *list, T data) {
    customPrint(yellow, bold, bgDefault,
                "[%s] Please use the specialization of this function for type T!\n", __FUNCTION__);
    return linkedListError::NO_ERRORS;
}

template<typename T>
linkedListError deleteNode(linkedList<T> *list, ssize_t index) {
    customPrint(yellow, bold, bgDefault,
                "[%s] Please use the specialization of this function for type T!\n", __FUNCTION__);
    return linkedListError::NO_ERRORS;
}

template<typename T>
linkedListError getNodeValue(linkedList<T> *list, ssize_t index, T *value) {
    customPrint(yellow, bold, bgDefault,
                "[%s] Please use the specialization of this function for type T!\n", __FUNCTION__);
    return linkedListError::NO_ERRORS;
}

#endif // LINKED_LIST_H_