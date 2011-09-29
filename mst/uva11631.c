#include <stdio.h>
#include <string.h>

#define MAXVERTICES 200000

struct UF {
	int *weight;
	int *parent;
	int sz;
};

struct UF global_uf;
int weight[MAXVERTICES + 10];
int parent[MAXVERTICES + 10];

struct UF *UF_new(int sz)
{
	struct UF *uf = &global_uf;
	uf->weight = weight;
	uf->parent = parent;
	uf->sz = sz;

	int i;
	for (i = 0; i < sz; i++)
		uf->parent[i] = i;
	memset(weight, 0, sizeof(weight));
	return uf;
}

int UF_find(struct UF *uf, int vertex)
{
	int orgVertex = vertex;
	int orgWt = uf->weight[vertex];
	int orgParent = uf->parent[vertex];
	int tmpParent;

	while (vertex != uf->parent[vertex]) {
		tmpParent = uf->parent[vertex];
		uf->weight[tmpParent] -= orgWt;
		vertex = tmpParent;
	}

	if (vertex != orgParent) {
		uf->weight[vertex] += orgWt;
		uf->parent[orgVertex] = vertex;
	}

	return vertex;
}

void UF_union(struct UF *uf, int v, int w)
{
	int v_root = UF_find(uf, v);
	int w_root = UF_find(uf, w);

	if (v_root == w_root)
		return;

	if (uf->weight[v_root] >= uf->weight[w_root]) {
		uf->weight[v_root] += uf->weight[w_root];
		uf->parent[w_root] = v_root;
	} else {
		uf->weight[w_root] += uf->weight[v_root];
		uf->parent[v_root] = w_root;
	}
}

struct edge {
	int v;
	int w;
	int wt;
};

struct edge edgeArray[MAXVERTICES + 10];

int edge_cmp(const void *a, const void *b)
{
	const struct edge *x = a;
	const struct edge *y = b;
	if (x->wt != y->wt) {
		return x->wt - y->wt;
	} else if (x->v != y->v) {
		return x->v - y->v;
	} else {
		return x->w - y->w;
	}
}

long long kruskal(struct UF *uf, int edges)
{
	qsort(edgeArray, edges, sizeof(struct edge), edge_cmp);
	int components = uf->sz;
	int i;
	int v, w;
	struct edge *edge;
	long long cost = 0;

	for (i = 0; i < edges && components > 0; i++) {
		edge = edgeArray + i;
		v = UF_find(uf, edge->v);
		w = UF_find(uf, edge->w);

		if (v != w) {
			UF_union(uf, v, w);
			components--;
			cost += (long long) edge->wt;
		}
	}

	return cost;
}

int main(int argc, char *argv[])
{
	long long totalCost;
	long long minSpending;
	int m, n;
	int v, w, wt;
	int i;

	v = w = wt = m = n = 0;
	while (1) {
		scanf("%d %d", &m, &n);
		if (m == 0 && n == 0)
			break;

		totalCost = 0;

		for (i = 0; i < n; i++) {
			scanf("%d %d %d", &v, &w, &wt);
			edgeArray[i].v = v;
			edgeArray[i].w = w;
			edgeArray[i].wt = wt;
			totalCost += (long long) wt;
		}

		struct UF *uf = UF_new(m);
		minSpending = kruskal(uf, n);
		printf("%lld\n", totalCost - minSpending);
	}
	return 0;
}
