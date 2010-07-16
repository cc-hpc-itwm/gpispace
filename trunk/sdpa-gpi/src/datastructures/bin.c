#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "bin.h"

struct  bin_tree *bin_create(void)
{
	struct bin_tree *tree = (struct bin_tree *)malloc(sizeof *tree);
	if(tree == NULL)
		return NULL;
	tree->root = NULL;
	tree->count = 0;
	return tree;
}


int bin_search(const struct bin_tree *tree,  void *object)
{
	const struct bin_node *node;

	assert(tree!=NULL);
	
	node = tree->root;
	for(;;){
		if(node == NULL)
			return 0;
		else if(handle == node->handle)
			return 1;
		else if(handle > node->handle)
			node = node->right;
		else
			node = node->left;
	}
}

/* MR: without forever:

node = tree->root

while (node != NULL && handle != node->handle)
{
  node = (handle > node->handle) ? tree->right : tree->left;
}

return (node == NULL) ? 0 : 1;
*/


int bin_insert(struct bin_tree *tree, fvmAllocHandle_t handle,fvmMemPointer_t pointer)
{

	struct bin_node *node;
	struct bin_node **newnode;

	assert(tree != NULL);
	
	newnode = &tree->root;
	node = tree->root;

	for(;;){
		if(node == NULL) {
			node = *newnode = (struct bin_node *)malloc(sizeof *node);

			if(node !=NULL){
				node->handle = handle;
				node->left = node->right = NULL;
				node->pointer = pointer;
				tree->count++;
				return 1;
			}
			else
				return 0;
		}
		else if(handle == node->handle)
			return 2;
		else if(handle > node->handle){
			newnode = &node->right;
			node = node->right;
		}
		else {
			newnode = &node->left;
			node = node->left;
		}

	}

}


int bin_delete(struct bin_tree *tree, fvmAllocHandle_t handle)
{

	struct bin_node **q, *z;

	assert(tree != NULL);
	
	q=&tree->root;
	z=tree->root;

	for(;;){
		if(z == NULL)
			return 0;
		else if(handle == z->handle)
			break;
		else if(handle > z->handle) {
			q = &z->right;
			z=z->right;
		}
		else {
			q = &z->left;
			z = z->left;
		}
	}

	if(z->right == NULL)
		*q = z->left;
	else {
		struct bin_node *y = z->right;
		if(y->left == NULL) {
			y->left = z->left;
			*q = y;
		}
		else {
			struct bin_node *x = y->left;
			while(x->left != NULL) {
				y = x;
				x = y->left;
			}
			y->left = x->right;
			x->left = z->left;
			x->right = z->right;
			*q = x;
		}
	}
	tree->count--;
	free(z);
	return 1;

}

static void walk(const struct bin_node *node)
{
	if (node == NULL)
		return;
	walk(node->left);
	printf("%ld ",node->handle);
	walk(node->right);
}



void bin_walk(const struct bin_tree *tree)
{
	assert(tree!=NULL);
	walk(tree->root);
}

static void walk_do(const struct bin_node *node,  void (*func)(void *) )
{
	if (node == NULL)
		return;

	func((void*)node);
 
	walk_do(node->left,func);
	walk_do(node->right,func);

}

void bin_walk_do(const struct bin_tree *tree, void (*func)(void *) )
{
	walk_do(tree->root,func); 

}

int bin_count(const struct bin_tree *tree)
{
	assert(tree!=NULL);
	return tree->count;
}

static void destroy(struct bin_node *node)
{
	if(node == NULL)
		return;
	destroy(node->left);
	destroy(node->right);
	free(node);
}

void bin_destroy(const struct bin_tree *tree)
{

	assert(tree !=NULL);
	destroy(tree->root);
	free((void *)tree);
}
