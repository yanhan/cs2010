#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
 * A very basic parser for an RPN-like prefix notation that makes use of binary
 * trees and to store computations and post order traversals for computation.
 *
 * The grammer is as follows:
 *
 * Computation
 * -----------
 *  Number
 *  +  Computation Computation
 *  -  Computation Computation
 *  *  Computation Computation
 *  /  Computation Computation
 *
 *
 * ----------------
 * Example
 * ----------------
 *
 *  4                                        [Evaluates to 4]
 *  + 4 2                                    [Evaluates to 6]
 *  + * 4 3 2                                [Evaluates to 14]
 *  + + 4 + 3 + 6 4 * 7 + -12 + 6 - 7 5      [Evaluates to -11]
 *
 *
 * Explanation
 *
 * 4
 * => As described in grammar, this is a valid computation. Evaluate to 4
 *
 *
 * + 4 2
 *
 * Tree formed:
 *
 *    +
 *   / \
 *  4   2
 *
 * Result: 4 + 2 = 6
 *
 *
 * + * 4 3 2
 *
 * Tree formed:
 *
 *     +
 *    / \
 *   *   2
 *  / \
 * 4   3
 *
 * Result: 4 * 3 + 2 = 12
 *
 *
 * + + 4 + 3 + 6 4 * 7 + -12 + 6 - 7 5
 *
 * Tree formed:
 *
 *                      +
 *              /             \
 *             +               *
 *            / \             / \
 *           4   +           7   +
 *              / \             / \
 *             3   +          -12  +
 *                / \             / \
 *               6   4           6   -
 *                                  / \
 *                                 7   5
 *
 * Result: (4 + (3 + (6 + 4))) + (7 * (-12 + (6 + (7 - 5)))) = 17 + -28 = -11
 */

#define BUF_SZ 4096

enum op {
	OP_NUM, /* Number, not computation. This is our base case */
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV
};

struct node {
	enum op op;
	int result;
	struct node *left;
	struct node *right;
};

struct bst {
	struct node *root;
};

struct opt {
	int quiet;
	int test;
};

void die(const char *msg)
{
	fprintf(stderr, msg);
	exit(1);
}

void *xcalloc(size_t nmemb, size_t sz)
{
	void *ptr = calloc(nmemb, sz);
	if (!ptr)
		die("xcalloc failed\n");
	return ptr;
}

char *strtrim(char *str)
{
	char *p;
	char *q;
	size_t len;

	if (!str || !*str)
		return NULL;

	len = strlen(str);
	p = str;
	q = str + len;
	while (isspace(*p))
		p++;

	if (!*p)
		return NULL;

	while (q > p && isspace(*--q))
		;

	q[1] = '\0';
	return p;
}

int is_valid_input(char *str)
{
	if (!str || !*str)
		return 0;

	int stack[2 * BUF_SZ];
	int cur_comp;
	int grok_started;
	char *p;

	p = str;
	while (*p && isspace(*p))
		p++;

	if (!*p)
		return 0;

	/* Chomp negative sign for negative number */
	if (*p == '-' && p[1] && (p[1] >= '0' && p[1] <= '9'))
		p++;

	/* Number only */
	if (*p >= '0' && *p <= '9') {
		while (*p && *p >= '0' && *p <= '9')
			p++;
		if (*p)
			return 0;
		else
			return 1;
	}

	/* Starts with symbol, stack[0] is dummy */
	cur_comp = 0;
	grok_started = 0;
	while (1) {

		/* Allow nested computations */
		while (cur_comp > 0 && stack[cur_comp] == 0)
			cur_comp--;

		while (*p && isspace(*p))
			p++;

		if (!*p)
			break;

		if (grok_started && cur_comp == 0)
			break;
		else if (!grok_started)
			grok_started = 1;

		switch (*p) {
		case '+': case '*': case '/':
			/* Space between operand and 1st operand */
			if (!p[1] || !isspace(p[1]))
				return 0;

			p++;
			cur_comp++;
			stack[cur_comp] = 2;
			stack[cur_comp-1]--;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':

parse_num: /* Label for negative number parsing */
			while (*p && *p >= '0' && *p <= '9')
				p++;

			stack[cur_comp]--;
			if (stack[cur_comp] == 0) {
				/* Done parsing this level */
				--cur_comp;
			} else if (stack[cur_comp] < 0) {
				/* Shouldnt happen */
				return 0;
			}

			if (!*p)
				goto parse_done;
			else if (!isspace(*p))
				return 0;

			break;

		case '-':
			/* More tricky, may involve negative numbers */
			if (p[1] && isspace(p[1])) {
				/* Minus operator */
				p++;
				cur_comp++;
				stack[cur_comp] = 2;
				stack[cur_comp-1]--;
			} else if (p[1] >= '0' && p[1] <= '9') {
				p++;
				goto parse_num;
			} else {
				/* Inval */
				return 0;
			}

			break;

		default:
			return 0;
		}
	}

parse_done:
	/* Nested computations which are done */
	while (cur_comp > 0 && stack[cur_comp] == 0)
		cur_comp--;

	if (cur_comp == 0) {
		/* Last check, any more tokens will make this invalid */
		while (*p) {
			if (!isspace(*p))
				return 0;
			p++;
		}

		return 1;
	} else {
		return 0;
	}
}

void test_parser(char *buf)
{
	char *str;
	int ret;

	while (fgets(buf, BUF_SZ, stdin)) {
		str = strtrim(buf);
		ret = is_valid_input(str);
		if (ret)
			printf("input is valid\n");
		else
			printf("input is invalid\n");
	}
}

struct node *build_tree_real(char **str)
{
	if (!str || !*str)
		return NULL;

	char *p;
	char *saved_ptr;
	struct node *node;

	p = *str;
	saved_ptr = NULL;

	while (*p && isspace(*p))
		p++;

	if (!*p)
		return NULL;

	node = xcalloc(1, sizeof(struct node));

	switch (*p) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		node->op = OP_NUM;
		node->result = strtol(p, &saved_ptr, 10);
		node->left = NULL;
		node->right = NULL;
		p = saved_ptr;
		break;
	case '+':
		node->op = OP_ADD;
		p++;
		node->left = build_tree_real(&p);
		node->right = build_tree_real(&p);
		break;
	case '*':
		node->op = OP_MUL;
		p++;
		node->left = build_tree_real(&p);
		node->right = build_tree_real(&p);
		break;
	case '/':
		node->op = OP_DIV;
		p++;
		node->left = build_tree_real(&p);
		node->right = build_tree_real(&p);
		break;
	case '-':
		if (p[1] && (p[1] >= '0' && p[1] <= '9')) {
			node->op = OP_NUM;
			node->result = strtol(p, &saved_ptr, 10);
			node->left = NULL;
			node->right = NULL;
			p = saved_ptr;
		} else {
			node->op = OP_SUB;
			p++;
			node->left = build_tree_real(&p);
			node->right = build_tree_real(&p);
		}
		break;
	}

	/* Update the pointer... */
	*str = p;
	return node;
}

struct bst *build_tree(const char *str)
{
	struct bst *bst = xcalloc(1, sizeof(struct bst));
	char **ptr = (char **) &str;
	bst->root = build_tree_real(ptr);
	return bst;
}

void node_free(struct node *n)
{
	if (!n)
		return;
	node_free(n->left);
	node_free(n->right);
	free(n);
}

void bst_free(struct bst *bst)
{
	node_free(bst->root);
	free(bst);
}

void node_print(struct node *n)
{
	if (!n)
		return;

	switch (n->op) {
	case OP_NUM:
		printf("%d\n", n->result);
		return;
	case OP_ADD:
		printf("+\n");
		break;
	case OP_SUB:
		printf("-\n");
		break;
	case OP_MUL:
		printf("*\n");
		break;
	case OP_DIV:
		printf("/\n");
		break;
	}

	node_print(n->left);
	node_print(n->right);
}

void bst_print(struct bst *bst)
{
	node_print(bst->root);
}

int bst_postorder_compute(struct node *n)
{
	/* Not supposed to happen */
	if (!n)
		return -1;

	int ret = 0;

	if (n->op == OP_NUM)
		return 0;

	ret = bst_postorder_compute(n->left);
	if (ret)
		return -1;

	ret = bst_postorder_compute(n->right);
	if (ret)
		return -1;

	switch (n->op) {
	case OP_ADD:
		n->result = n->left->result + n->right->result;
		break;
	case OP_SUB:
		n->result = n->left->result - n->right->result;
		break;
	case OP_MUL:
		n->result = n->left->result * n->right->result;
		break;
	case OP_DIV:
		if (n->right->result == 0)
			return -1;
		n->result = n->left->result / n->right->result;
		break;
	}

	return 0;
}

void bst_compute(struct bst *bst, int *result)
{
	if (!result)
		printf("Pointer must be supplied to compute result\n");

	int ret = bst_postorder_compute(bst->root);
	if (ret)
		printf("Error in computation\n");
	else
		*result = bst->root->result;
}

void parseargs(int argc, char *argv[], struct opt *opt)
{
	int i;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--test")) {
			opt->test = 1;
		} else if (!strcmp(argv[i], "--quiet")) {
			opt->quiet = 1;
		}
	}
}

int main(int argc, char *argv[])
{
	char buf[BUF_SZ];
	char *str;
	int ret;
	int result;
	struct opt opt;

	/* Don't really feel like using gnu getopt for this... */
	parseargs(argc, argv, &opt);

	if (opt.test)
		test_parser(buf);

	while (fgets(buf, BUF_SZ, stdin)) {
		str = strtrim(buf);
		ret = is_valid_input(str);
		if (ret) {
			struct bst *bst = build_tree(str);
			bst_compute(bst, &result);

			if (opt.quiet)
				printf("%d\n", result);
			else
				printf("result of \"%s\" is %d\n", str, result);
			bst_free(bst);
		} else
			printf("input is invalid\n");
	}

	return 0;
}
