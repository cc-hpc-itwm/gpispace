
struct bin_node {
	struct bin_node *left;
	struct bin_node *right;
	void *object;
};

// MR: General hint: Structure is exposed!
struct bin_tree {
	struct bin_node *root;
	int count;
};

struct  bin_tree *bin_create(void);
int bin_search(const struct bin_tree *tree,  void *object);
int bin_insert(struct bin_tree *tree, void *object);
int bin_delete(struct bin_tree *tree, void *object);
void bin_walk(const struct bin_tree *tree);
void bin_destroy(const struct bin_tree *tree);
void bin_walk_do(const struct bin_tree *tree, void (*func) (void *) );
