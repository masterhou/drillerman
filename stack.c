#include "stack.h"

#include <stdlib.h>
#include <string.h>

/*
  Generic dynamically allocated stack.
  Pushing automatically allocates and copies
  the item so you can pass pointer to a variable
  allocated on local function stack.

  When popping immediately copy the item because
  the pointer can be invalidated by future pushes.
*/

Stack stackAlloc(size_t itemSize, int capacityOverhead)
{
    Stack st;

    if(capacityOverhead <= 0)
        capacityOverhead = 1;

    st.capacityOverheadSz = (size_t)capacityOverhead * itemSize;
    st.capacitySz = st.capacityOverheadSz;
    st.itemSz = itemSize;
    st.sz = 0;
    st.itemPtr = NULL;

    st.index = -1;

    if(capacityOverhead != 0)
        st.items = malloc(st.capacitySz);
    else
        st.items = NULL;

    return st;
}

void stackPush(Stack *stack, void *itemPtr)
{
    if(stack->sz == stack->capacitySz)
    {
        stack->capacitySz += stack->capacityOverheadSz;
        stack->items = realloc(stack->items, stack->capacitySz);
    }

    stack->sz += stack->itemSz;
    stack->index++;

    stack->itemPtr = stack->items + (stack->itemSz * stack->index);

    memcpy(stack->itemPtr, itemPtr, stack->itemSz);
}

void *stackPop(Stack *stack)
{
    if(stack->sz == 0)
        return NULL;

    stack->index--;
    stack->sz -= stack->itemSz;

    void *itemPtr = stack->itemPtr;
    stack->itemPtr -= stack->itemSz;

    return itemPtr;
}

void stackFree(Stack stack)
{
    free(stack.items);
}

void stackClear(Stack *stack)
{
    stack->index = -1;
    stack->sz = 0;
    stack->itemPtr = NULL;
}

inline int stackGetItemCount(Stack *stack)
{
    return stack->index + 1;
}

inline bool stackNotEmpty(Stack *stack)
{
    return stack->sz != 0;
}



