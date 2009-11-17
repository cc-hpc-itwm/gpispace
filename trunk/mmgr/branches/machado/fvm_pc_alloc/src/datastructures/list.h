#include <stddef.h>

typedef struct list
{
  int tag;
  struct list *next;
  void *object;
  size_t size;
} list_t;


int listAdd(list_t **item,
	    int tag,
	    void *object,
	    size_t size);

int listAddFront(list_t **item,
	     int tag,
	     void *object,
		 size_t size);

void *listGetData(list_t *item, 
		  int *tag,
		  size_t *size);

list_t *listDelete(list_t *item);

void listDeleteNext(list_t *item);

void listDestroy(list_t **list);
