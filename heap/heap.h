#ifndef HUFFMAN_HEAP_H
#define HUFFMAN_HEAP_H

struct node {
	int ch;
	int freq;
	struct node *left;
	struct node *right;
};

struct node *node_new(int ch, int freq);
void node_free(struct node *n);
void node_print(struct node *n);

void heap_insert(struct node **heap, struct node *node,
		int *nr, int alloc);
void heap_remove(struct node **heap, int *nr, struct node **elem);
void heap_free(struct node **heap, int nr);

#endif
