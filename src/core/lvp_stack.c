#include <stdlib.h>
#include "lvp_stack.h"


void lvp_stack_add_cap(LVPStack* stack)
{

#define Stack_Default_Cap 8
    int64_t newCap = 0;
    if (stack->cap<Stack_Default_Cap)
    {
        newCap = Stack_Default_Cap;
    }
    else
    {
        newCap = stack->cap * 2;
    }
    
    void **datas = (void**)lvp_mem_malloc(sizeof(void*)*newCap);
    void **usrDatas = (void**)lvp_mem_malloc(sizeof(void*)*newCap);
    lvp_custom_free *freeFuncs = (lvp_custom_free*)lvp_mem_malloc(sizeof(lvp_custom_free)*newCap);
    LVP_BOOL *needFrees = (LVP_BOOL*)lvp_mem_malloc(sizeof(LVP_BOOL)*newCap);

    for (int64_t i = stack->top; i < stack->bottom; i++)
    {
        int64_t j = stack->cap + i;
        datas[j] = stack->datas[i];
        usrDatas[j] = stack->usrDatas[i];
        freeFuncs[j] = stack->freeFuncs[i];
        needFrees[j] = stack->needFrees[i];
    }

    lvp_mem_free(stack->datas);
    lvp_mem_free(stack->usrDatas);
    lvp_mem_free(stack->freeFuncs);
    lvp_mem_free(stack->needFrees);

    stack->datas = datas;
    stack->usrDatas = usrDatas;
    stack->freeFuncs = freeFuncs;
    stack->needFrees = needFrees;

    stack->top = stack->top + stack->cap;
    stack->bottom = stack->bottom + stack->cap;
    stack->cap = newCap;

}

LVPStack * lvp_stack_create()
{
    LVPStack* stack = (LVPStack*)lvp_mem_malloc(sizeof(LVPStack));

    stack->datas = LVP_LP_NULL;
    stack->usrDatas = LVP_LP_NULL;
    stack->freeFuncs = LVP_NULL;
    stack->needFrees = LVP_NULL;
    stack->cap = 0;
    stack->top = 0;
    stack->bottom = 0;

    lvp_stack_add_cap(stack);

    return stack;
}

LVP_BOOL lvp_stack_free(LVPStack * stack)
{
    if (stack==LVP_NULL)
    {
        return LVP_FALSE;
    }

    for (int64_t i = stack->top; i < stack->bottom; i++)
    {
        lvp_custom_free freeFunc = stack->freeFuncs[i];
        LVP_BOOL needFree = stack->needFrees[i];
        if (needFree)
        {
            if (freeFunc!=LVP_NULL)
            {
                freeFunc(stack->datas[i], stack->usrDatas[i]);
            }
            else
            {
                lvp_mem_free(stack->datas[i]);
            }
        }
    }

    lvp_mem_free(stack->datas);
    lvp_mem_free(stack->usrDatas);
    lvp_mem_free(stack->freeFuncs);
    lvp_mem_free(stack->needFrees);
    lvp_mem_free(stack);

    return LVP_TRUE;
}

LVP_BOOL lvp_stack_clear(LVPStack * stack)
{
    if (stack == LVP_NULL)
    {
        return LVP_FALSE;
    }

    for (int64_t i = stack->top; i < stack->bottom; i++)
    {
        lvp_custom_free freeFunc = stack->freeFuncs[i];
        LVP_BOOL needFree = stack->needFrees[i];
        if (needFree)
        {
            if (freeFunc != LVP_NULL)
            {
                freeFunc(stack->datas[i], stack->usrDatas[i]);
            }
            else
            {
                lvp_mem_free(stack->datas[i]);
            }
        }
    }

    stack->top = stack->cap;
    stack->bottom = stack->cap;

    return LVP_TRUE;
}

LVP_BOOL lvp_stack_push(LVPStack* stack,void * data, void * usrData, lvp_custom_free freeFunc, LVP_BOOL needFree)
{
    if (stack == LVP_NULL)
    {
        return LVP_FALSE;
    }

    if (stack->top==0)
    {
        lvp_stack_add_cap(stack);
    }

    stack->top--;
    stack->datas[stack->top] = data;
    stack->usrDatas[stack->top] = usrData;
    stack->freeFuncs[stack->top] = freeFunc;
    stack->needFrees[stack->top] = needFree;

    return LVP_TRUE;
}

LVP_BOOL lvp_stack_push_raw(LVPStack * stack, void * data)
{
    return lvp_stack_push(stack, data, LVP_NULL, LVP_NULL, LVP_FALSE);
}

LVP_BOOL lvp_stack_pop(LVPStack * stack)
{
    if (stack == LVP_NULL)
    {
        return LVP_FALSE;
    }

    if (stack->top == stack->bottom)
    {
        return LVP_FALSE;
    }

    stack->top++;

    return LVP_TRUE;
}

LVP_BOOL lvp_stack_top(LVPStack * stack, void** data, void ** usrData)
{
    if (stack == LVP_NULL || data == LVP_NULL)
    {
        return LVP_FALSE;
    }

    if (stack->top == stack->bottom)
    {
        return LVP_FALSE;
    }

    if (LVP_LP_NULL != data)
    {
        *data = stack->datas[stack->top];
    }

    if (usrData != LVP_LP_NULL)
    {
        *usrData = stack->usrDatas[stack->top];
    }

    return LVP_TRUE;
}

LVP_BOOL lvp_stack_size(LVPStack * stack, int64_t * size)
{
    if (stack == LVP_NULL)
    {
        return LVP_FALSE;
    }

    *size = stack->bottom - stack->top;

    return LVP_TRUE;
}
