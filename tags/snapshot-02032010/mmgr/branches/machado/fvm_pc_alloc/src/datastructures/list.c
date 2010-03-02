#include <stdlib.h>
#include <string.h>

#include "list.h"

int listAdd(list_t **item,
	    int tag,
	    void *object,
	    size_t size)
{

  list_t *newItem;

  int result = 1;

  if(size > 0)
    {
      newItem = (list_t *)malloc(sizeof(*newItem));

      if(newItem != NULL)
	{
	  newItem->tag = tag;
	  newItem->size = size;
	  newItem->object = object;

	  if(newItem->object != NULL)
	    {
	      memcpy(newItem->object, object, size);
	      
	      if(*item == NULL)
		{
		  newItem->next = NULL;
		  *item = newItem;
		}
	      else //insert after current item
		{
		  newItem->next = (*item)->next;
		  (*item) = newItem;
		}
	    }
	  else
	    {
	      free(newItem);
	      result = 0;
	    }
	}
      else
	{
	  result = 0;
	}
    }
  else
    {
      result = 0;
    }
  return result;
}

int listAddFront(list_t **item,
	     int tag,
	     void *object,
	     size_t size)
{
  int result = 1;

  list_t *p = NULL;

  result = listAdd(&p, tag, object, size);

  if(result)
    {
      p->next = *item;
      *item = p;
    }

  return result;

}

//retrieve data on a list item in a more cleaner way
void *listGetData(list_t *item, 
		int *tag,
		size_t *size)
{

  void *p = NULL;
  
  if(item != NULL)
    {
      if(tag != NULL)
	{
	  *tag = item->tag;
	}
      if(size != NULL)
	{
	  *size = item->size;
	}
      p = item->object;
    }
  return p;
}

//remember, this a single linked list
//make sure to set up links after deleting an item
list_t *listDelete(list_t *item)
{

  list_t *next = NULL;

  if(item != NULL)
    {
      next = item->next;
      
      if(item->object != NULL)
	{
	  free(item->object);
	}
      free(item);
    }
  return next;
}

void listDeleteNext(list_t *item)
{
  if(item != NULL && item->next != NULL)
    {
      item->next = listDelete(item->next);
    }
}


void listDestroy(list_t **list)
{
  list_t *next;

  if(*list != NULL)
    {
      next = *list;

      do 
	{
	  next = listDelete(next);
	} while (next != NULL);
      *list = NULL;
    }
}



