#include <stdlib.h>
#include <stdio.h>
#include "bin.h"

typedef unsigned long fvmAllocHandle_t;


static void printBlock(void *tmp)
{
  printf("handle %lu\n", ((binode *)tmp)->handle);
  
}

int main ()
{

	int i;
	binode *allocList=bin_create();

	//insert to the left
	for(i=20;i>=0;i--){
		binode *newElement = (binode *) malloc(sizeof(binode));
		newElement->handle = i;

		insert(&allocList,newElement, NULL);

	}

    
	//insert to the right
	for(i=21;i<50;i++){
		binode *newElement = (binode *) malloc(sizeof(binode));
		newElement->handle = i;

		insert(&allocList,newElement, NULL);

	}

	binode *tmp = allocList;
	walk_do(tmp,&printBlock);

	printf("------\n");
	
	remove(&allocList,21);
	remove(&allocList,20);
	remove(&allocList,0);
	remove(&allocList,1);

	tmp=allocList;
	walk_do(tmp,&printBlock);

	for(i=0;i<50;i++)
		remove(&allocList,i);
	printf("------\n");
	
	tmp=allocList;
	walk_do(tmp,&printBlock);
    
	return 0;
}
