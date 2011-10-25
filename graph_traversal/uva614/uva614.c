#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAXROWS 12
#define DIRS    4
#define BUFSZ   4096

#define TERM_STR "0 0 0 0 0 0"

/*#define PRINT_DEBUG*/

#define WEST  0
#define NORTH 1
#define EAST  2
#define SOUTH 3

#define VISITED 0xDEAD

/* Directions */
int d_row[DIRS] = {0, -1, 0, 1};
int d_col[DIRS] = {-1, 0, 1, 0};

struct pt {
	int row;
	int col;
};

struct pt parent[MAXROWS + 4][MAXROWS + 4];
struct pt stack[MAXROWS * MAXROWS + 1];
int maze[MAXROWS + 4][MAXROWS + 4][DIRS];
int visited[MAXROWS + 4][MAXROWS + 4];
int progress[MAXROWS + 4][MAXROWS + 4];

int rows, cols;
int startRow, startCol;
int endRow, endCol;

int mazeNum = 0;

void print_hori_border(void)
{
	int i;
	putchar('+');
	for (i = 0; i < cols; i++)
		printf("---+");
	putchar('\n');
}

void traverse_graph(void)
{
	int i, k, stIdx;
	int prevRow, prevCol;
	int curRow, curCol;
	int nRow, nCol;
	int tmpRow, tmpCol;

	prevRow = prevCol = -1;
	curRow = startRow;
	curCol = startCol;

	/* Visited - 0 indicates not visited. 0xDEAD indicates visited */
	memset(parent, 0, sizeof(parent));
	memset(stack, 0, sizeof(stack));
	memset(visited, 0, sizeof(visited));
	memset(progress, 0, sizeof(progress));

	while (1) {
		visited[curRow][curCol] = VISITED;
		#ifdef PRINT_DEBUG
		printf("now at (%d, %d)\n", curRow, curCol);
		#endif
		if (curRow == endRow && curCol == endCol) {
			break;
		}

		prevRow = curRow;
		prevCol = curCol;

		/* Try West, North, East, South */
		for (i = progress[curRow][curCol]; i < DIRS; i++) {
			/* Wall */
			if (maze[curRow][curCol][i] == -1)
				continue;
			nRow = curRow + d_row[i];
			nCol = curCol + d_col[i];

			if (nRow < 0 || nRow >= rows || nCol < 0 || nCol >= cols ||
				visited[nRow][nCol] == VISITED)
				continue;

			progress[curRow][curCol] = i+1;

			parent[nRow][nCol].row = curRow;
			parent[nRow][nCol].col = curCol;
			prevRow = curRow;
			prevCol = curCol;
			curRow  = nRow;
			curCol  = nCol;
			break;
		}

		/* Backtrack */
		if (curRow == prevRow && curCol == prevCol) {
			tmpRow = parent[curRow][curCol].row;
			tmpCol = parent[curRow][curCol].col;
			curRow = tmpRow;
			curCol = tmpCol;
		}
	}

	/* Backtrack from (endRow, endCol) to (startRow, startCol) */
	curRow = endRow;
	curCol = endCol;
	stIdx = 0;
	while (curRow != startRow || curCol != startCol) {
		stack[stIdx].row = curRow;
		stack[stIdx].col = curCol;
		stIdx++;
		tmpRow = parent[curRow][curCol].row;
		tmpCol = parent[curRow][curCol].col;
		curRow = tmpRow;
		curCol = tmpCol;
	}

	stack[stIdx].row = startRow;
	stack[stIdx].col = startCol;

	/* Reverse the stack to get order */
	i = 1;
	for (; stIdx >= 0; stIdx--) {
		tmpRow = stack[stIdx].row;
		tmpCol = stack[stIdx].col;

		visited[tmpRow][tmpCol] = i;
		i++;
	}

	printf("Maze %d\n\n", ++mazeNum);
	print_hori_border();
	for (i = 0; i < rows; i++) {
		putchar('|');
		for (k = 0; k < cols; k++) {
			if (visited[i][k] == 0) {
				printf("   ");
			} else if (visited[i][k] == VISITED) {
				printf("???");
			} else {
				printf("%3d", visited[i][k]);
			}
			if (maze[i][k][EAST] == -1)
				putchar('|');
			else if (k < cols -1)
				putchar(' ');
		}
		printf("|\n");

		/* Print southern walls if not last line */
		if (i < rows - 1) {
			int m;
			putchar('+');
			for (m = 0; m < cols; m++) {
				if (maze[i][m][SOUTH] == -1)
					printf("---+");
				else
					printf("   +");
			}
			putchar('\n');
		}
	}
	print_hori_border();
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	char buf[BUFSZ];
	rows = cols = startRow = startCol = endRow = endCol = 0;

	while (1) {
		if (!gets(buf))
			break;

		if (!strcmp(buf, TERM_STR))
			break;

		sscanf(buf, "%d %d %d %d %d %d", &rows, &cols, &startRow,
			   &startCol, &endRow, &endCol);

		#ifdef PRINT_DEBUG
		printf("ht = %d, width = %d, startRow = %d, startCol = %d, endRow = %d, endCol = %d\n",
			   rows, cols, startRow, startCol, endRow, endCol);
		#endif

		int row = 0;
		int col;

		memset(maze, 0, sizeof(maze));
		while (1) {
			if (!gets(buf) || strlen(buf) == 0)
				break;

			int idx = 0;
			col = 0;
			while (1) {
				if (!buf[idx])
					break;

				switch(buf[idx]) {
				case '0':
					/* No walls */
					break;
				case '1':
					/* Wall on east */
					maze[row][col][EAST] = -1;
					maze[row][col+1][WEST] = -1;
					break;
				case '2':
					/* Wall on south */
					maze[row][col][SOUTH] = -1;
					maze[row+1][col][NORTH] = -1;
					break;
				case '3':
					/* Wall on east + south */
					maze[row][col][EAST] = -1;
					maze[row][col+1][WEST] = -1;
					maze[row][col][SOUTH] = -1;
					maze[row+1][col][NORTH] = -1;
					break;
				default:
					fprintf(stderr, "invalid char in maze, %c\n", buf[idx]);
					exit(1);
				}

				idx++;
				if (isspace(buf[idx]))
					idx++;

				col++;
			}
			row++;
		}

		/* Minus 1 off the coordinates (0 indexing) */
		startRow--;
		startCol--;
		endRow--;
		endCol--;
		traverse_graph();
	}
	return 0;
}
