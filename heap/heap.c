#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
#include "util.h"

struct node *node_new(int ch, int freq)
{
	struct node *node = xcalloc(1, sizeof(struct node));
	node->ch = ch;
	node->freq = freq;
	return node;
}

void node_free(struct node *n)
{
	if (!n)
		return;

	node_free(n->left);
	node_free(n->right);
	free(n);
}

void node_print(struct node *n)
{
	if (!n)
		return;

	if (n->left && n->right) {
		printf("internal node, freq: %d\n", n->freq);
		node_print(n->left);
		node_print(n->right);
	} else {
		printf("char %c, freq %d\n", n->ch, n->freq);
	}
}

#define PARENT(nn) (((nn) % 2 == 0) ? ((nn) - 1) / 2 : (nn) / 2)
#define LEFTCHILD(nn) (2 * (nn) + 1)
#define RIGHTCHILD(nn) (2 * (nn) + 2)

void heap_swim(struct node **heap, int idx)
{
	struct node *orig = heap[idx];
	struct node *parent;
	int parent_idx;

	while (idx > 0) {
		parent_idx = PARENT(idx);
		parent = heap[parent_idx];
		if (parent->freq < orig->freq)
			break;

		heap[idx] = parent;
		idx = parent_idx;
	}

	heap[idx] = orig;
}

void heap_sink(struct node **heap, int idx, int nr)
{
	struct node *orig;
	struct node *best;
	struct node *leftc;
	struct node *rightc;
	int best_idx;
	int leftc_idx;
	int rightc_idx;

	int stop = nr / 2;
	orig = heap[idx];
	while (idx <= stop) {
		best = orig;
		best_idx = idx;

		leftc_idx = LEFTCHILD(idx);
		rightc_idx = RIGHTCHILD(idx);

		if (leftc_idx >= nr)
			goto compare_done;

		leftc = heap[leftc_idx];
		if (leftc->freq < best->freq) {
			best = leftc;
			best_idx = leftc_idx;
		}

		if (rightc_idx >= nr)
			goto compare_done;

		rightc = heap[rightc_idx];
		if (rightc->freq < best->freq) {
			best = rightc;
			best_idx = rightc_idx;
		}

compare_done:
		if (best_idx == idx)
			break;

		heap[idx] = best;
		idx = best_idx;
	}

	heap[idx] = orig;
}

void heap_insert(struct node **heap, struct node *node,
		int *nr, int alloc)
{
	if (!nr)
		return;
	if (*nr + 1 >= alloc)
		return;

	heap[*nr] = node;
	*nr = *nr + 1;
	heap_swim(heap, *nr - 1);
}

void heap_remove(struct node **heap, int *nr, struct node **elem)
{
	if (!nr || *nr == 0)
		return;

	*elem = heap[0];
	*nr = *nr - 1;
	if (*nr > 0) {
		heap[0] = heap[*nr];
		if (*nr > 1)
			heap_sink(heap, 0, *nr);
	}
}

void heap_free(struct node **heap, int nr)
{
	if (nr <= 0)
		return;

	int i;
	for (i = 0; i < nr; i++)
		node_free(heap[i]);
}
