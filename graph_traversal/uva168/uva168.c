#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define MAXCHARS 512
#define MAXVERTICES 26
#define FIRST_VERTEX 'A'

char line[MAXCHARS];
int adj[MAXVERTICES + 2][MAXVERTICES + 2];
int adjIdx[MAXVERTICES + 2];
int lighted[MAXVERTICES + 2];

void hunt_minotaur(int minotaur, int theseus, int k)
{
	memset(lighted, 0, sizeof(lighted));

	int mino_cur;
	int mino_oldcur;
	int mino_prev;
	int next;

	int i, m;
	int vertex_cnt = 0;

	for (mino_cur = minotaur, mino_prev = theseus; ; ) {
		mino_oldcur = mino_cur;
		for (i = 0; i < adjIdx[mino_cur]; i++) {
			next = adj[mino_cur][i];
			if (lighted[next] || next == mino_prev)
				continue;

			mino_prev = mino_cur;
			mino_cur = next;
			vertex_cnt++;
			break;
		}

		if (mino_cur == mino_oldcur) {
			putchar('/');
			putchar(mino_cur + FIRST_VERTEX);
			putchar('\n');
			break;
		}

		if (vertex_cnt % k == 0) {
			lighted[mino_prev] = 1;
			putchar(mino_prev + FIRST_VERTEX);
			putchar(' ');
		}
	}
}

int main(int argc, char *argv[])
{
	int idx;
	int vertex;
	int neighbor;
	int maze_ended;
	int minotaur_vertex;
	int theseus_vertex;
	int k;

	while (gets(line) != NULL) {
		if (!strcmp(line, "#"))
			break;

		idx = 0;
		maze_ended = 0;
		memset(adj, 0, sizeof(adj));
		memset(adjIdx, 0, sizeof(adjIdx));

		while (1) {
			if (!line[idx])
				break;

			vertex = line[idx++] - FIRST_VERTEX;
			if (line[idx] == ';') {
				idx++;
				continue;
			}

			assert(line[idx] == ':');
			idx++;
			if (line[idx] == ';') {
				idx++;
				continue;
			}

			while (1) {
				if (line[idx] == ';') {
					idx++;
					break;
				} else if (line[idx] == '.') {
					idx++;
					maze_ended = 1;
					break;
				}

				neighbor = line[idx++] - FIRST_VERTEX;
				adj[vertex][adjIdx[vertex]++] = neighbor;
			}

			if (maze_ended) {
				while (isspace(line[idx]))
					idx++;

				minotaur_vertex = line[idx++] - FIRST_VERTEX;
				while (isspace(line[idx]))
					idx++;
				theseus_vertex = line[idx++] - FIRST_VERTEX;

				k = 0;
				sscanf(line + idx, "%d", &k);

				#ifdef PRINT_DEBUG
				int i, m;
				for (i = 0; i < MAXVERTICES; i++) {
					printf("%c:", i + FIRST_VERTEX);
					for (m = 0; m < adjIdx[i]; m++) {
						printf(" %c", adj[i][m] + FIRST_VERTEX);
					}
					printf("\n");
				}
				printf("Minotaur starts from %c\n", minotaur_vertex + FIRST_VERTEX);
				printf("Theseus starts from %c\n", theseus_vertex + FIRST_VERTEX);
				printf("k = %d\n", k);
				#endif
				break;
			}
		}

		/* Do the hunting */
		hunt_minotaur(minotaur_vertex, theseus_vertex, k);
	}
	return 0;
}
