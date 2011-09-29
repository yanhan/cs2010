#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*#define PRINT_DEBUG*/

#define INF INT_MAX
#define MAXVERTICES 210
#define REALMAXVERTICES 200
#define MAXEDGES 20000
#define MAXCITYNAME 40

struct vidx {
	unsigned int hash;
	char *name;
	int idx;
	int taken;
};

struct edge {
	int v;
	int w;
	int wt;
};

struct vidx htab[MAXVERTICES];
struct edge edgeList[MAXEDGES];
int nr_edges;

char cityNames[MAXVERTICES][MAXCITYNAME];
char *cityPtr[MAXVERTICES];
int city_nr;

unsigned int sdbm(const char *s)
{
	unsigned int h = 0;
	int c;
	while (c = *s++)
		h = c + (h << 6) + (h << 16) - h;
	return h;
}

int hash_city(const char *city)
{
	unsigned int hash = sdbm(city);
	unsigned int pos = hash % REALMAXVERTICES;

	for (; htab[pos].taken; pos = (pos + 1) % REALMAXVERTICES) {
		if (htab[pos].hash == hash &&
			!strcmp(htab[pos].name, city))
			return htab[pos].idx;
	}

	/* Not taken */
	htab[pos].hash = hash;
	int i;
	for (i = 0; city[i]; i++)
		cityNames[city_nr][i] = city[i];
	cityNames[city_nr][i] = '\0';
	cityPtr[city_nr] = &cityNames[city_nr][0];
	htab[pos].name = cityPtr[city_nr];
	htab[pos].idx = city_nr;
	htab[pos].taken = 1;
	city_nr++;
	return htab[pos].idx;
}

void insert_edge(const char *c1, const char *c2, int wt)
{
	int c1pos = hash_city(c1);
	int c2pos = hash_city(c2);

	edgeList[nr_edges].v = c1pos;
	edgeList[nr_edges].w = c2pos;
	edgeList[nr_edges].wt = wt;
	nr_edges++;
}

void graph_dump(void)
{
	int i;
	printf("Cities available\n");
	for (i = 0; i < city_nr; i++) {
		printf("City \"%s\"\n", cityPtr[i]);
	}

	printf("Edges\n");
	for (i = 0; i < nr_edges; i++) {
		printf("%s - %s (%d)\n", cityPtr[edgeList[i].v],
			cityPtr[edgeList[i].w], edgeList[i].wt);
	}
	printf("\n");
}

struct UF {
	int *weight;
	int *parent;
	int sz;
};

int gWEIGHT[MAXVERTICES];
int gPARENT[MAXVERTICES];
struct UF globalUF;

struct UF *UF_new(int sz)
{
	struct UF *uf = &globalUF;
	uf->weight = gWEIGHT;
	uf->parent = gPARENT;
	memset(gWEIGHT, 0, sizeof(gWEIGHT));
	memset(gPARENT, 0, sizeof(gPARENT));
	uf->sz = sz;

	int i;
	for (i = 0; i < sz; i++) {
		uf->weight[i] = 1;
		uf->parent[i] = i;
	}
	return uf;
}

int UF_find(struct UF *uf, int v)
{
	int orgWt = uf->weight[v];
	int orgParent = uf->parent[v];
	int newParent;

	while (v != uf->parent[v]) {
		uf->weight[uf->parent[v]] -= orgWt;
		v = uf->parent[v];
	}

	if (orgParent != v)
		uf->weight[v] += orgWt;
	return v;
}

void UF_union(struct UF *uf, int v, int w)
{
	v = UF_find(uf, v);
	w = UF_find(uf, w);

	if (v == w) {
		return;
	} else if (uf->weight[v] > uf->weight[w]) {
		uf->weight[v] += uf->weight[w];
		uf->parent[w] = v;
	} else {
		uf->weight[w] += uf->weight[v];
		uf->parent[v] = w;
	}
}

int edgeCmp(const void *a, const void *b)
{
	const struct edge *x = a;
	const struct edge *y = b;
	if (x->wt != y->wt)
		return x->wt - y->wt;
	else if (x->v != y->v)
		return x->v - y->v;
	else
		return x->w - y->w;
}

int run_mst(const char *start, const char *end)
{
	struct UF *uf = UF_new(city_nr);
	int min = INF;
	int i; 
	int v, w, wt;

	int startPos = hash_city(start);
	int endPos = hash_city(end);

	qsort(edgeList, nr_edges, sizeof(struct edge), edgeCmp);
	for (i = 0; i < nr_edges; i++) {
		if (UF_find(uf, startPos) == UF_find(uf, endPos))
			break;

		v = UF_find(uf, edgeList[i].v);
		w = UF_find(uf, edgeList[i].w);

		if (v != w) {
			UF_union(uf, v, w);
			/* Reverse back */
			wt = -edgeList[i].wt;
			if (wt < min)
				min = wt;
		}
	}

	return min;
}

int main(int argc, char *argv[])
{
	int numCities, numRoads;
	int numTest = 0;
	int i, idx;
	int c1idx, c2idx;
	int wt;
	char buf[1024];
	char cityOne[40];
	char cityTwo[40];

	wt = numCities = numRoads = 0;
	while (1) {
		gets(buf);
		sscanf(buf, "%d %d", &numCities, &numRoads);
		if (numCities == 0 && numRoads == 0)
			break;

		memset(htab, 0, sizeof(htab));
		memset(edgeList, 0, sizeof(edgeList));
		memset(cityNames, 0, sizeof(cityNames));
		memset(cityPtr, 0, sizeof(cityPtr));
		nr_edges = city_nr = 0;
		for (i = 0; i < numRoads; i++) {
			gets(buf);

			c1idx = c2idx = 0;
			for (idx = 0; buf[idx] && buf[idx] != ' '; idx++)
				cityOne[c1idx++] = buf[idx];
			cityOne[c1idx] = '\0';

			/* Check it's not null */
			idx++;
			for (; buf[idx] && buf[idx] != ' '; idx++)
				cityTwo[c2idx++] = buf[idx];
			cityTwo[c2idx] = '\0';

			buf[idx] = '\0';
			idx++;
			wt = atoi(buf + idx);

			/* NOTE: Insert negative of weight */
			insert_edge(cityOne, cityTwo, -wt);

			#ifdef PRINT_DEBUG
			printf("city one = \"%s\", city two = \"%s\", wt = %d\n", cityOne,
				cityTwo, wt);
			#endif
		}

		/* start and end */
		gets(buf);
		c1idx = c2idx = idx = 0;
		for (; buf[idx] && buf[idx] != ' '; idx++)
			cityOne[c1idx++] = buf[idx];
		cityOne[c1idx] = '\0';

		for (++idx; buf[idx] && buf[idx] != ' '; idx++)
			cityTwo[c2idx++] = buf[idx];
		cityTwo[c2idx] = '\0';

		#ifdef PRINT_DEBUG
		printf("start = \"%s\", dest = \"%s\"\n", cityOne, cityTwo);
		graph_dump();
		#endif

		hash_city(cityOne);
		hash_city(cityTwo);

		printf("Scenario #%d\n", ++numTest);
		printf("%d tons\n", run_mst(cityOne, cityTwo));
		putchar('\n');
	}
	return 0;
}
