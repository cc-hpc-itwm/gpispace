#include <stdlib.h>
#include <stdio.h>
#include "bin.h"

typedef unsigned long fvmAllocHandle_t;



int main ()
{

	int i;
	struct bin_tree *allocList=bin_create();

	//insert to the left
	for(i=20;i>=0;i--)
		bin_insert(allocList,i);

    
	//insert to the right
	for(i=21;i<50;i++)
		bin_insert(allocList,i);



	bin_walk(allocList);

	printf("------\n");
	
	bin_delete(allocList,21);
	bin_delete(allocList,20);
	bin_delete(allocList,0);
	bin_delete(allocList,1);

	bin_walk(allocList);

	for(i=0;i<50;i++)
		bin_delete(allocList,i);
	printf("------\n");
	
	bin_walk(allocList);
    
	bin_destroy(allocList);
	return 0;
}
