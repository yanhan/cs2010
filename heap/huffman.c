#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFSZ 4096
#define S_BUFSZ 256
#define MAXCHARS 256

void die(const char *msg)
{
	fprintf(stderr, msg);
	exit(1);
}

void usage(void)
{
	fprintf(stderr, "huffman [--encode] [--decode] <file>\n");
	exit(1);
}

void *xcalloc(size_t nmemb, size_t sz)
{
	void *ptr = calloc(nmemb, sz);
	if (!ptr)
		die("xcalloc failed\n");
	return ptr;
}

void *xrealloc(void *ptr, size_t sz)
{
	ptr = realloc(ptr, sz);
	if (!ptr)
		die("xrealloc failed\n");
	return ptr;
}

struct node {
	char ch;
	int freq;
	struct node *left;
	struct node *right;
};

struct node *node_new(char ch, int freq)
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

int write_to_header(char *hdr, int *hdr_nr, int *hdr_alloc,
		void *buf, int sz)
{
	int i;
	int alloc, nr;

	nr = *hdr_nr;
	alloc = *hdr_alloc;

	if (nr + sz >= alloc) {
		int new_alloc = (alloc + 16) * 2;
		if (new_alloc <= alloc) {
			fprintf(stderr, "write_to_header: int overflow\n");
			return -1;
		}

		hdr = xrealloc(hdr, new_alloc);
		alloc = new_alloc;
		*hdr_alloc = new_alloc;
	}

	memcpy(hdr + nr, buf, sz);
	*hdr_nr = nr + sz;
	return 0;
}

int prepare_header(char *hdr, int *hdr_nr, int *hdr_alloc, struct node **heap,
		int heap_nr)
{
	assert(hdr_alloc && hdr_nr);

	char buf[S_BUFSZ];
	int alloc = *hdr_alloc;
	int nr = *hdr_nr;
	int i;

	/* First, write out HHHH */
	buf[0] = buf[1] = buf[2] = buf[3] = 'H';
	if (write_to_header(hdr, hdr_nr, hdr_alloc, buf, 4)) {
		fprintf(stderr, "writing of opening header failed\n");
		return -1;
	}

	/*
	 * Then, write out the heap information as below:
	 *
	 * letter\0\0\0frequency
	 *
	 * where "letter" is a 1 byte character, the 3 '\0' bytes are for padding,
	 * and "frequency" is a 4 byte integer
	 *
	 * Given the same heap building algorithm, we are guaranteed to have the same
	 * prefix tree built.
	 */

	for (i = 0; i < heap_nr; i++) {
		if (write_to_header(hdr, hdr_nr, hdr_alloc, &(heap[i]->ch), 1)) {
			fprintf(stderr, "writing of heap failed (char)\n");
			return -1;
		}

		int k;
		buf[0] = buf[1] = buf[2] = '\0';
		if (write_to_header(hdr, hdr_nr, hdr_alloc, buf, 3)) {
			fprintf(stderr, "writing of heap failed (padding)\n");
			return -1;
		}

		if (write_to_header(hdr, hdr_nr, hdr_alloc, &(heap[i]->freq), 4)) {
			fprintf(stderr, "writing of heap failed (int)\n");
			return -1;
		}
	}

	/* Then write HHHH */
	buf[0] = buf[1] = buf[2] = buf[3] = 'H';
	if (write_to_header(hdr, hdr_nr, hdr_alloc, buf, 4)) {
		fprintf(stderr, "writing of closing header failed\n");
		return -1;
	}

	return 0;
}

void build_prefix_tree(struct node **heap, int *heap_nr)
{
	assert(heap_nr != NULL);
	int nr = *heap_nr;

}

void huffman_encode(int *freq, int range, struct node **heap,
		int heap_sz)
{
	assert(range <= heap_sz);
	int i;
	int heap_nr;
	FILE *fp;
	char ch;

	char *hdr;
	int hdr_alloc, hdr_nr;

	int ret;

	heap_nr = 0;
	for (i = 0; i < range; i++) {
		if (!freq[i])
			continue;

		struct node *node = node_new(i, freq[i]);
		heap_insert(heap, node, &heap_nr, heap_sz);
	}

	/* Allocate header for write out later */
	hdr_alloc = BUFSZ;
	hdr_nr = 0;
	hdr = xcalloc(hdr_alloc, sizeof(char));

	ret = prepare_header(hdr, &hdr_nr, &hdr_alloc, heap, heap_nr);
	if (ret)
		goto cleanup;

	/* Time for the fun: Build prefix tree */
	build_prefix_tree(heap, &heap_nr);

	/* Actually write out the header */
	fp = fopen("header", "w");
	if (!fp)
		goto cleanup;

	if (fwrite(hdr, hdr_nr, 1, fp) != 1)
		fprintf(stderr, "error writing out header\n");

cleanup:
	if (fp)
		fclose(fp);
	free(hdr);
	heap_free(heap, heap_nr);
}

void encode_file(const char *file)
{
	char buf[BUFSZ];
	int freq[MAXCHARS];
	FILE *fp;
	ssize_t i;
	ssize_t bread, bwritten;
	struct node *heap[MAXCHARS];

	memset(buf, 0, sizeof(buf));
	memset(freq, 0, sizeof(freq));

	fp = fopen(file, "r");
	if (!fp)
		die("reading of file failed");

	while (bread = fread(buf, sizeof(char), BUFSZ, fp)) {
		for (i = 0; i < bread; i++)
			freq[buf[i]]++;
	}

	fclose(fp);

	memset(heap, 0, sizeof(struct node *) * MAXCHARS);
	huffman_encode(freq, MAXCHARS, heap, MAXCHARS);
}

void decode_file(const char *file)
{
	FILE *fp;
	char buf[BUFSZ];
	size_t bread;

	struct node *heap[MAXCHARS];
	int nr = 0;

	memset(heap, 0, sizeof(heap));
	fp = fopen(file, "r");
	if (!fp)
		die("cannot open file for decoding\n");

	bread = fread(buf, 1, 4, fp);
	if (bread != 4) {
		fprintf(stderr, "invalid header\n");
		goto cleanup;
	}

	if (buf[0] != 'H' || buf[1] != 'H' || buf[2] != 'H' || buf[3] != 'H') {
		fprintf(stderr, "invalid opening header \"%c%c%c%c\"\n",
				buf[0], buf[1], buf[2], buf[3]);
		goto cleanup;
	}

	while (1) {
		char ch;
		int freq;
		struct node *node;

		bread = fread(buf, 1, 4, fp);
		if (bread != 4) {
			fprintf(stderr, "invalid header\n");
			goto cleanup;
		}

		/* Closing header */
		if (buf[0] == 'H' && buf[1] == 'H' && buf[2] == 'H' && buf[3] == 'H')
			break;

		if (buf[1] != '\0' || buf[2] != '\0' || buf[3] != '\0') {
			fprintf(stderr, "invalid padding between character and frequency\n");
			goto cleanup;
		}

		ch = buf[0];
		bread = fread(&freq, 4, 1, fp);
		if (bread != 1) {
			fprintf(stderr, "invalid header\n");
			goto cleanup;
		}

		node = node_new(ch, freq);
		heap_insert(heap, node, &nr, MAXCHARS);
	}

	/* Print header info */
	int i;
	for (i = 0; i < nr; i++)
		printf("char = %c, freq = %d\n", heap[i]->ch, heap[i]->freq);

cleanup:
	fclose(fp);
	heap_free(heap, nr);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		usage();

	if (!strcmp(argv[1], "--encode"))
		encode_file(argv[2]);
	else if (!strcmp(argv[1], "--decode"))
		decode_file(argv[2]);
	else
		usage();
	return 0;
}
