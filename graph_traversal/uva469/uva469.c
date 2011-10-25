#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSZ    4096
#define MAXROWS  99
#define IMPOS_SZ 2000000
#define DIRS     8
#define WATER    'W'
#define VISITED  'D'

/*#define PRINT_DEBUG*/

struct pt {
	int row;
	int col;
};

struct pt queue[IMPOS_SZ];
int qhead, qtail;
int rows, cols;
char land[MAXROWS + 4][MAXROWS + 4];

/* N, NE, E, SE, S, SW, W, NW */
int d_row[DIRS] = {-1, -1, 0, 1, 1,  1,  0, -1};
int d_col[DIRS] = { 0,  1, 1, 1, 0, -1, -1, -1};

void restore_lands()
{
	int i, k;
	for (i = 0; i < rows; i++) {
		for (k = 0; k < cols; k++) {
			if (land[i][k] == VISITED)
				land[i][k] = WATER;
		}
	}
}

int floodfill(int r, int c)
{
	int i, k, m;
	int curRow, curCol;
	int nRow, nCol;
	int wetlands = 0;

	memset(queue, 0, sizeof(queue));
	qhead = qtail = 0;

	queue[qtail].row = r;
	queue[qtail].col = c;
	qtail++;

	while (qhead < qtail) {
		curRow = queue[qhead].row;
		curCol = queue[qhead].col;
		qhead++;

		if (land[curRow][curCol] != WATER)
			continue;
		land[curRow][curCol] = VISITED;
		++wetlands;

		for (i = 0; i < DIRS; i++) {
			nRow = curRow + d_row[i];
			nCol = curCol + d_col[i];

			if (nRow < 0 || nRow >= rows || nCol < 0 || nCol >= cols)
				continue;
			if (land[nRow][nCol] != WATER)
				continue;
			queue[qtail].row = nRow;
			queue[qtail].col = nCol;
			qtail++;
		}
	}

	/* Change back */
	restore_lands();
	return wetlands;
}

int main(int argc, char *argv[])
{
	int i, k, m;
	int wetlands;
	int numCases;
	char buf[BUFSZ];

	rows = cols = numCases = 0;

	gets(buf);
	sscanf(buf, "%d", &numCases);

	/* Blank line */
	gets(buf);
	if (strcmp(buf, "")) {
		fprintf(stderr, "Not blank line!\n");
		exit(1);
	}

	for (i = 0; i < numCases; i++) {
		rows = 0;
		memset(land, 0, sizeof(land));
		while (1) {
			gets(land[rows]);
			if (land[rows][0] != 'L' && land[rows][0] != 'W')
				break;
			rows++;
		}

		cols = strlen(land[0]);
		#ifdef PRINT_DEBUG
		for (k = 0; k < rows; k++) {
			for (m = 0; m < cols; m++) {
				putchar(land[k][m]);
			}
			putchar('\n');
		}
		#endif

		k = m = 0;

		/* first of the k lines was stored in the last land buffer */
		sscanf(land[rows], "%d %d", &k, &m);
		#ifdef PRINT_DEBUG
		printf("i = %d, j = %d\n", k, m);
		#endif

		if (i > 0)
			putchar('\n');

		/* Change to 0 based from 1 based */
		wetlands = floodfill(k - 1, m - 1);
		printf("%d\n", wetlands);

		while (1) {
			if (!gets(buf))
				goto done;
			else if (!strcmp(buf, ""))
				break;

			sscanf(buf, "%d %d", &k, &m);
			#ifdef PRINT_DEBUG
			printf("i = %d, j = %d\n", k, m);
			#endif

			/* 
			 * Flood fill time 
			 * Change to 0 based from 1 based
			 */
			wetlands = floodfill(k - 1, m - 1);
			printf("%d\n", wetlands);
		}
	}

done:
	return 0;
}
