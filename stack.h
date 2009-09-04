#ifndef STACK_H
#define STACK_H

#include <inttypes.h>
#include <stddef.h>

#include "common.h"

typedef struct
{

    void *items;
    void *itemPtr;
    int index;
    size_t itemSz;
    size_t capacityOverheadSz;
    size_t capacitySz;
    size_t sz;
} Stack;

Stack stackAlloc(size_t itemSize, int capacityOverhead);
void stackPush(Stack *stack, void *itemPtr);
void *stackPop(Stack *stack);
void stackFree(Stack stack);
int stackGetItemCount(Stack *stack);
void stackClear(Stack *stack);
bool stackNotEmpty(Stack *stack);

#endif
