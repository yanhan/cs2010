#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFSZ 4096
#define S_BUFSZ 256
#define MAXCHARS 264
#define SP_EOF 258

#define COMPRESSED_OUT "compressed"
#define HUFFMAN_HEADER 0xdddddddd

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

char *xstrdup(const char *s)
{
	char *p = strdup(s);
	if (!p)
		die("xstrdup failed\n");
	return p;
}

struct node {
	int ch;
	int freq;
	struct node *left;
	struct node *right;
};

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

void header_print(struct node **heap, int nr)
{
	/* Print header info */
	int i;
	for (i = 0; i < nr; i++) {
		printf("char = %d (", heap[i]->ch);
		if (heap[i]->ch > 127)
			printf("not viewable");
		else
			printf("%c", heap[i]->ch);
		printf("), freq = %d\n", heap[i]->freq);
	}
}

void prefix_print(char **prefix, int nr)
{
	int i;
	for (i = 0; i < nr; i++) {
		if (!prefix[i])
			continue;
		printf("char %d (", i);
		if (i > 127)
			printf("not viewable");
		else
			printf("%c", i);
		printf("), prefix = \"%s\"\n", prefix[i]);
	}
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
	int ch;

	/* First, write out magic number */
	ch = HUFFMAN_HEADER;
	if (write_to_header(hdr, hdr_nr, hdr_alloc, &ch, 4)) {
		fprintf(stderr, "writing of opening header failed\n");
		return -1;
	}

	/*
	 * Then, write out the heap information as below:
	 *
	 * letterfrequency
	 *
	 * where "letter" and "frequency" are both 4 byte integers
	 *
	 * Given the same heap building algorithm, we are guaranteed to have the same
	 * prefix tree built.
	 */

	for (i = 0; i < heap_nr; i++) {
		if (write_to_header(hdr, hdr_nr, hdr_alloc, &(heap[i]->ch), 4)) {
			fprintf(stderr, "writing of heap failed (char)\n");
			return -1;
		}

		if (write_to_header(hdr, hdr_nr, hdr_alloc, &(heap[i]->freq), 4)) {
			fprintf(stderr, "writing of heap failed (int)\n");
			return -1;
		}
	}

	/* Then write ending header (magic number) */
	ch = HUFFMAN_HEADER;
	if (write_to_header(hdr, hdr_nr, hdr_alloc, &ch, 4)) {
		fprintf(stderr, "writing of closing header failed\n");
		return -1;
	}

	return 0;
}

void build_prefix_tree(struct node **heap, int *heap_nr, int heap_sz)
{
	assert(heap_nr != NULL);
	int nr = *heap_nr;
	struct node *first, *second;

	while (nr > 1) {
		struct node *p;
		heap_remove(heap, &nr, &first);
		heap_remove(heap, &nr, &second);

		/* Internal nodes have character of -1 */
		p = node_new(-1, first->freq + second->freq);
		p->left = first;
		p->right = second;
		heap_insert(heap, p, &nr, heap_sz);
	}

	*heap_nr = 1;
}

/* dir: left is 0, right is 1 */
void build_prefixes(char **prefix, struct node *n, char *buf, int buf_idx,
		int dir)
{
	if (!n) {
		fprintf(stderr, "build_prefixes: null tree node\n");
		return;
	}

	buf[buf_idx++] = dir == 0 ? '0' : '1';
	if (n->ch != -1) {
		buf[buf_idx] = '\0';
		/* 1st char is a dummy */
		prefix[n->ch] = xstrdup(buf + 1);
	} else {
		build_prefixes(prefix, n->left, buf, buf_idx, 0);
		build_prefixes(prefix, n->right, buf, buf_idx, 1);
	}
}

void get_prefix_strings(char **prefix, struct node *prefix_tree)
{
	char buf[BUFSZ];
	build_prefixes(prefix, prefix_tree, buf, 0, 0);
}

void write_content(unsigned char *content, int content_alloc,
		struct node *prefix_tree, FILE *infp, FILE *outfp)
{
	/* Time to replace by prefix string... */
	int i, k, n;
	int byte_idx;
	int bit_pos;
	int cur_prefix_len;
	unsigned char ch;
	char *cur_prefix;
	char *prefix[MAXCHARS];

	memset(prefix, 0, sizeof(prefix));
	get_prefix_strings(prefix, prefix_tree);

	/* DEBUG: Print prefix strings */
	prefix_print(prefix, MAXCHARS);

	byte_idx = 0;
	bit_pos = 7;
	cur_prefix = NULL;
	memset(content, 0, sizeof(content));

	while (fread(&ch, 1, 1, infp) == 1) {
		cur_prefix = prefix[ch];
		cur_prefix_len = strlen(cur_prefix);

		for (i = 0; i < cur_prefix_len; i++) {
			if (byte_idx >= content_alloc) {
				if (fwrite(content, 1, content_alloc, outfp) != content_alloc) {
					fprintf(stderr, "writing of content failed\n");
					goto cleanup;
				}

				byte_idx = 0;
				bit_pos = 7;
				memset(content, 0, sizeof(content));
			}

			n = cur_prefix[i] == '0' ? 0 : 1;
			content[byte_idx] |= (n << bit_pos);
			bit_pos--;

			if (bit_pos < 0) {
				bit_pos = 7;
				byte_idx++;
			}
		}
	}

	/* No more input. Flush if buffer is full */
	if (byte_idx >= content_alloc) {
		if (fwrite(content, 1, content_alloc, outfp) != content_alloc) {
			fprintf(stderr, "writing of content failed\n");
			goto cleanup;
		}
		byte_idx = 0;
		bit_pos = 7;
		memset(content, 0, sizeof(content));
	}

	/* Write remaining stuff + special EOF string */
	cur_prefix = prefix[SP_EOF];
	cur_prefix_len = strlen(cur_prefix);
	for (i = 0; i < cur_prefix_len; i++) {
		if (byte_idx >= content_alloc) {
			if (fwrite(content, 1, content_alloc, outfp) != content_alloc) {
				fprintf(stderr, "writing of content failed\n");
				goto cleanup;
			}
			byte_idx = 0;
			bit_pos = 7;
			memset(content, 0, sizeof(content));
		}

		n = cur_prefix[i] == '0' ? 0 : 1;
		content[byte_idx] |= (n << bit_pos);
		bit_pos--;

		if (bit_pos < 0) {
			bit_pos = 7;
			byte_idx++;
		}
	}

	/* Write out */
	if (byte_idx != 0 || bit_pos != 7) {
		int write_out = byte_idx + 1;
		if (fwrite(content, 1, write_out, outfp) != write_out) {
			fprintf(stderr, "writing of contents failed\n");
			goto cleanup;
		}
	}

cleanup:
	for (i = 0; i < MAXCHARS; i++) {
		if (prefix[i])
			free(prefix[i]);
	}
}

void huffman_encode(int *freq, int range, struct node **heap,
		int heap_sz, FILE *infp)
{
	assert(range <= heap_sz && infp != NULL);
	int i;
	int ret;
	int heap_nr;
	FILE *outfp;
	char ch;

	char *hdr;
	int hdr_alloc, hdr_nr;

	unsigned char content[BUFSZ];
	struct node *node;

	heap_nr = 0;

	/* Insert SP EOF */
	freq[SP_EOF] = 1;
	for (i = 0; i < range; i++) {
		if (!freq[i])
			continue;

		node = node_new(i, freq[i]);
		heap_insert(heap, node, &heap_nr, heap_sz);
	}

	/* Debug: print header */
	header_print(heap, heap_nr);

	/* Allocate header for write out later */
	hdr_alloc = BUFSZ;
	hdr_nr = 0;
	hdr = xcalloc(hdr_alloc, sizeof(char));

	ret = prepare_header(hdr, &hdr_nr, &hdr_alloc, heap, heap_nr);
	if (ret)
		goto cleanup;

	/* Actually write out the header */
	outfp = fopen(COMPRESSED_OUT, "w");
	if (!outfp)
		goto cleanup;

	if (fwrite(hdr, hdr_nr, 1, outfp) != 1)
		fprintf(stderr, "error writing out header\n");

	/* Time for the fun: Build prefix tree and compress */
	build_prefix_tree(heap, &heap_nr, heap_sz);
	write_content(content, BUFSZ, heap[0], infp, outfp);

cleanup:
	if (outfp)
		fclose(outfp);
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

	memset(heap, 0, sizeof(struct node *) * MAXCHARS);
	huffman_encode(freq, MAXCHARS, heap, MAXCHARS, fp);
	fclose(fp);
}

void decode_file(const char *file)
{
	FILE *fp;
	unsigned char buf[BUFSZ];
	size_t bread;

	struct node *heap[MAXCHARS];
	int i;
	int nr = 0;
	int ch = 0;
	int freq = 0;

	char *prefix[MAXCHARS];

	memset(heap, 0, sizeof(heap));
	fp = fopen(file, "r");
	if (!fp)
		die("cannot open file for decoding\n");

	bread = fread(&ch, 4, 1, fp);
	if (bread != 1) {
		fprintf(stderr, "invalid header: start\n");
		goto cleanup;
	}

	if (ch != HUFFMAN_HEADER) {
		fprintf(stderr, "invalid opening header \"%x\"\n", ch);
		goto cleanup;
	}

	while (1) {
		int ch;
		int freq;
		struct node *node;

		bread = fread(&ch, 4, 1, fp);
		if (bread != 1) {
			fprintf(stderr, "invalid header\n");
			goto cleanup;
		}

		/* Closing header */
		if (ch == HUFFMAN_HEADER)
			break;

		/* Letter Frequency pair */
		bread = fread(&freq, 4, 1, fp);
		if (bread != 1) {
			fprintf(stderr, "invalid header\n");
			goto cleanup;
		}

		node = node_new(ch, freq);
		/* Bypass insertion procedure since we wrote out in heap order */
		heap[nr++] = node;
	}

	header_print(heap, nr);
	build_prefix_tree(heap, &nr, MAXCHARS);

	memset(prefix, 0, sizeof(prefix));
	get_prefix_strings(prefix, heap[0]);

	/* Print the prefix strings */
	prefix_print(prefix, MAXCHARS);

cleanup:
	for (i = 0; i < MAXCHARS; i++) {
		if (prefix[i])
			free(prefix[i]);
	}
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
