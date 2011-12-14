#include "stack.h"

int stackPush(stack_t *stack,int tag, void *object, size_t size)
{
  int result = 0;

  int listResult;

  listResult = listAddFront(&stack->stackPtr, tag, object, size);

  if(listResult == 1)
    {
      result = 1;
      ++stack->numItems;
    }

  return result;
}

int stackPop(void *object, stack_t *stack)
{

  size_t size;
  void *p;
  int result = 1;

  if(stack->numItems > 0)
    {
      p = listGetData(stack->stackPtr, NULL, &size);
      if (p != NULL && object != NULL)
	{
	  memcpy(object, p, size);
	}
      else
	{
	  result = 0;
	}
      stack->stackPtr = listDelete(stack->stackPtr);
      (stack->numItems)--;
    }
  else
    {
      result = 0;
    }
  return result;

}

void * stackGetData(stack_t *stack, int *tag, size_t *size)
{

  return listGetData(stack->stackPtr, tag, size);

}

size_t stackCount(stack_t *stack)
{

  return stack->numItems;

}

