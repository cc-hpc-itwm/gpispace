#include <stddef.h>
#include <string.h>

#include "list.h"


typedef struct stack
{
  list_t *stackPtr;
  size_t numItems;

} stack_t;


int stackPush(stack_t *stack,int tag, void *object, size_t);
int stackPop(void *object, stack_t *stack);
void * stackGetData(stack_t *stack, int *tag, size_t *size);
size_t stackCount(stack_t *stack);
