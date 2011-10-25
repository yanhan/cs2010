#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BUFSZ 4096
#define MAXROWS 99

char land[MAXROWS + 4][MAXROWS + 4];
int rows;
int cols;

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
		m = 0;
		memset(land, 0, sizeof(land));
		while (1) {
			gets(land[m]);
			if (land[m][0] != 'L' && land[m][0] != 'W')
				break;
			m++;
		}

		/* first of the k lines was stored in the last land buffer */
		sscanf(land[m], "%d %d", &rows, &cols);
		while (1) {
			if (!gets(buf))
				goto done;
			else if (!strcmp(buf, ""))
				break;

			/* Flood fill time */
			wetlands = floodfill();
		}
	}

done:
	return 0;
}
