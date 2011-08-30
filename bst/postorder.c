#include <stdio.h>
#include <ctype.h>
#include <string.h>

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
};

struct bst {
	struct node *root;
};

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

	/* Only a number */
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

		#ifdef someshit
		case '-':
			/* More tricky, may involve negative numbers */
			if (p[1] && isspace(p[1])) {
			} else if (p[1] >= '0' && p[1] <= '9') {
			} else {
				/* Inval */
				return 0;
			}
			break;
		#endif

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

int main(int argc, char *argv[])
{
	char buf[BUF_SZ];
	char *str;
	int ret;

	if (argc == 2 && !strcmp(argv[1], "--test")) {
		test_parser(buf);
		return 0;
	}

	while (fgets(buf, BUF_SZ, stdin)) {
		str = strtrim(buf);
		ret = is_valid_input(str);
		if (ret)
			printf("input is valid\n");
		else
			printf("input is invalid\n");
	}

	return 0;
}
