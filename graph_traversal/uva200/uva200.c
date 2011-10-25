#include <stdio.h>
#include <string.h>

/*#define PRINT_DEBUG*/

#define REAL_MAXCHARS 20
#define MAXCHARS 32
#define ALPHABET   26
#define FIRST_CHAR 'A'

int adj[ALPHABET + 4][ALPHABET + 4];
int adjIdx[ALPHABET + 4];
int inside[ALPHABET + 4];

int stack[128];
int stIdx;

int visited[ALPHABET + 4];
int dfsOrder[ALPHABET + 4];
int dfsProgress[ALPHABET + 4];
int dfsOrderIdx;
int dfsRoot;

void toposort(void)
{
	int i;
	int v, n;
	memset(dfsProgress, 0, sizeof(dfsProgress));

	dfsOrderIdx = 0;
	stIdx = 0;
	stack[stIdx++] = dfsRoot;
	visited[dfsRoot] = 1;

	while (stIdx > 0) {
		v = stack[stIdx - 1];
		if (dfsProgress[v] >= adjIdx[v]) {
			stIdx--;
			dfsOrder[dfsOrderIdx++] = v;
			continue;
		}

		for (; dfsProgress[v] < adjIdx[v]; dfsProgress[v]++) {
			n = adj[v][dfsProgress[v]];
			if (visited[n])
				continue;
			visited[n] = 1;
			stack[stIdx++] = n;
			break;
		}
	}

	while (--dfsOrderIdx >= 0)
		printf("%c", dfsOrder[dfsOrderIdx] + FIRST_CHAR);
	for (i = 0; i < ALPHABET; i++)
		if (inside[i] && !visited[i])
			putchar(i + FIRST_CHAR);
	printf("\n");
}

int main(int argc, char *argv[])
{
	int i, k;
	char buf1[MAXCHARS];
	char buf2[MAXCHARS];

	char *line1, *line2, *tmpLine;
	line1 = buf1;
	line2 = buf2;

	memset(adj, 0, sizeof(adj));
	memset(adjIdx, 0, sizeof(adjIdx));
	memset(inside, 0, sizeof(inside));
	memset(visited, 0, sizeof(visited));

	if (!gets(line1))
		return 0;

	/* Root of dfs */
	dfsRoot = line1[0] - FIRST_CHAR;
	for (i = 0; line1[i]; i++)
		inside[line1[i] - FIRST_CHAR] = 1;

	while (1) {
		if (!gets(line2) || !strcmp(line2, "#"))
			break;

		for (i = 0; i < REAL_MAXCHARS; i++) {
			if (!line1[i] || !line2[i]) {
				break;
			}
				
			if (line1[i] == line2[i]) {
				continue;
			} else {
				int vertex = line1[i] - FIRST_CHAR;
				/* Differing char. Insert edge */
				adj[vertex][adjIdx[vertex]++] = line2[i] - FIRST_CHAR;
				inside[line2[i] - FIRST_CHAR] = 1;
				break;
			}
		}

		for (; line2[i]; i++)
			inside[line2[i] - FIRST_CHAR] = 1;

		tmpLine = line1;
		line1 = line2;
		line2 = tmpLine;
	}

	/* Print edges */
	#ifdef PRINT_DEBUG
	for (i = 0; i < ALPHABET; i++) {
		if (!inside[i])
			continue;

		printf("%c:", i + FIRST_CHAR);
		for (k = 0; k < adjIdx[i]; k++) {
			printf(" %c", adj[i][k] + FIRST_CHAR);
		}
		putchar('\n');
	}
	#endif

	/* Topological sort */
	toposort();
	return 0;
}
