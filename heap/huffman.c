#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "heap.h"
#include "util.h"

#define BUFSZ    4096
#define S_BUFSZ  256
#define MAXCHARS 264
#define SP_EOF   258

#define COMPRESSED_OUT   "compressed"
#define DECOMPRESSED_OUT "decompressed"
#define HUFFMAN_HEADER   0xdddddddd

enum OP_HUFFMAN { OP_UNKNOWN = 0, OP_ENCODE = 1, OP_DECODE };
enum { OPT_DEBUG = 1000 };

/* Globals */
struct opt {
	int debug;
	int op;
	const char *src;
	const char *dest;
} opt;

void usage(void)
{
	fprintf(stderr, "huffman [--encode] [--decode] <file>\n");
	exit(1);
}

int error(const char *msg)
{
	fprintf(stderr, msg);
	return -1;
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
	 * Given the same heap building algorithm, we are guaranteed to have
	 * the same prefix tree built.
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
	if (fseek(infp, 0L, SEEK_SET)) {
		fprintf(stderr, "cannot rewind input file\n");
		return;
	}
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
	if (opt.debug)
		prefix_print(prefix, MAXCHARS);

	ch = 0;
	byte_idx = 0;
	bit_pos = 7;
	memset(content, 0, content_alloc * sizeof(unsigned char));

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
				memset(content, 0, content_alloc * sizeof(unsigned char));
			}

			if (cur_prefix[i] == '1')
				content[byte_idx] |= (1 << bit_pos);
			bit_pos--;

			if (bit_pos < 0) {
				bit_pos = 7;
				byte_idx++;
				if (opt.debug)
					printf("just parsed %d\n", content[byte_idx-1]);
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
		memset(content, 0, content_alloc * sizeof(unsigned char));
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
			memset(content, 0, content_alloc * sizeof(unsigned char));
		}

		if (cur_prefix[i] == '1')
			content[byte_idx] |= (1 << bit_pos);
		bit_pos--;

		if (bit_pos < 0) {
			bit_pos = 7;
			byte_idx++;
			if (opt.debug)
				printf("just parsed %d\n", content[byte_idx-1]);
		}
	}

	/* Write out */
	if (byte_idx != 0 || bit_pos != 7) {
		int write_out = byte_idx + 1;
		if (bit_pos != 7) {
			if (opt.debug)
				printf("just parsed %d\n", content[write_out-1]);
		}
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
		int heap_sz, FILE *infp, FILE *outfp)
{
	assert(range <= heap_sz && infp != NULL);
	int i;
	int ret;
	int heap_nr;
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
	if (opt.debug)
		header_print(heap, heap_nr);

	/* Allocate header for write out later */
	hdr_alloc = BUFSZ;
	hdr_nr = 0;
	hdr = xcalloc(hdr_alloc, sizeof(char));

	ret = prepare_header(hdr, &hdr_nr, &hdr_alloc, heap, heap_nr);
	if (ret)
		goto cleanup;

	/* Actually write out the header */
	if (fwrite(hdr, hdr_nr, 1, outfp) != 1)
		fprintf(stderr, "error writing out header\n");

	/* Time for the fun: Build prefix tree and compress */
	build_prefix_tree(heap, &heap_nr, heap_sz);
	write_content(content, BUFSZ, heap[0], infp, outfp);

cleanup:
	free(hdr);
	heap_free(heap, heap_nr);
}

void encode_file(const char *src, const char *dest)
{
	char buf[BUFSZ];
	int freq[MAXCHARS];
	FILE *infp;
	FILE *outfp;
	ssize_t i;
	ssize_t bread, bwritten;
	struct node *heap[MAXCHARS];

	memset(buf, 0, sizeof(buf));
	memset(freq, 0, sizeof(freq));

	infp = fopen(src, "r");
	if (!infp)
		die("opening of file for reading failed");

	outfp = fopen(dest, "w");
	if (!outfp) {
		fclose(infp);
		die("opening of file for writing failed");
	}

	while (bread = fread(buf, sizeof(char), BUFSZ, infp)) {
		for (i = 0; i < bread; i++)
			freq[buf[i]]++;
	}

	memset(heap, 0, sizeof(struct node *) * MAXCHARS);
	huffman_encode(freq, MAXCHARS, heap, MAXCHARS, infp, outfp);
	fclose(infp);
	fclose(outfp);
}

void decode_file(const char *src, const char *dest)
{
	FILE *fp;
	FILE *outfp;
	unsigned char buf[BUFSZ];
	size_t bread;

	struct node *heap[MAXCHARS];
	int i;
	int nr = 0;
	int ch = 0;
	int freq = 0;

	char *prefix[MAXCHARS];

	memset(heap, 0, sizeof(heap));
	fp = fopen(src, "r");
	if (!fp)
		die("cannot open file for decoding\n");

	outfp = fopen(dest, "w");
	if (!outfp) {
		fprintf(stderr, "cannot open decompressed file for writing\n");
		goto cleanup;
	}

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

	if (opt.debug)
		header_print(heap, nr);
	build_prefix_tree(heap, &nr, MAXCHARS);

	memset(prefix, 0, sizeof(prefix));
	get_prefix_strings(prefix, heap[0]);

	/* DEBUG: Print the prefix strings */
	if (opt.debug)
		prefix_print(prefix, MAXCHARS);

	/* Time for the real job... */
	unsigned char rch = 0;
	int bit_pos;
	int cur_bit;
	struct node *prefix_tree = heap[0];

	bread = 0;
	while (fread(&rch, 1, 1, fp) == 1) {
		if (opt.debug)
			printf("just parsed %d\n", rch);

		bit_pos = 7;
		while (bit_pos >= 0) {
			cur_bit = (rch >> bit_pos) & 1;

			if (opt.debug)
				printf("%c", cur_bit == 0 ? '0' : '1');

			if (cur_bit == 0)
				prefix_tree = prefix_tree->left;
			else
				prefix_tree = prefix_tree->right;

			assert(prefix_tree != NULL);

			if (prefix_tree->ch != -1) {
				if (opt.debug)
					printf("\n");

				if (prefix_tree->ch == SP_EOF) {
					goto cleanup;
				}

				unsigned char outch = prefix_tree->ch;
				fwrite(&outch, 1, 1, outfp);
				prefix_tree = heap[0];
			}

			bit_pos--;
		}
	}

cleanup:
	for (i = 0; i < MAXCHARS; i++) {
		if (prefix[i])
			free(prefix[i]);
	}
	if (outfp)
		fclose(outfp);
	fclose(fp);
	heap_free(heap, nr);
}

int parse_op(int op, int *n, int argc, char *argv[])
{
	int i;
	assert(op == OP_ENCODE || op == OP_DECODE);
	if (opt.op != OP_UNKNOWN)
		return error("only 1 of --encode and --decode is accepted.\n");

	opt.op = op;
	i = *n;
	if (i >= argc)
		return error("no source file\n");
	else
		opt.src = argv[i];

	if (i + 1 < argc && argv[i + 1][0] != '-') {
		i++;
		opt.dest = argv[i];
	} else {
		if (op == OP_ENCODE)
			opt.dest = COMPRESSED_OUT;
		else
			opt.dest = DECOMPRESSED_OUT;
	}

	*n = i;
	return 0;
}

int parse_args(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--debug")) {
			opt.debug = 1;
		} else if (!strcmp(argv[i], "--encode")) {
			i++;
			if (parse_op(OP_ENCODE, &i, argc, argv))
				return -1;
		} else if (!strcmp(argv[i], "--decode")) {
			i++;
			if (parse_op(OP_DECODE, &i, argc, argv))
				return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	opt.op = OP_UNKNOWN;
	if (parse_args(argc, argv))
		usage();

	if (opt.op == OP_ENCODE)
		encode_file(opt.src, opt.dest);
	else
		decode_file(opt.src, opt.dest);
	return 0;
}
