import java.util.Collections;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.Arrays;

class Main {
	class Edge implements Comparable<Edge> {
		int from;
		int to;
		int wt;

		public Edge(int f, int t, int w) {
			from = f;
			to = t;
			wt = w;
		}

		public int compareTo(Edge e) {
			if (wt != e.wt)
				return wt - e.wt;
			else if (from != e.from)
				return from - e.from;
			else
				return to - e.to;
		}
	}

	class UF {
		private int[] weight;
		private int[] parent;
		private int sz;

		public UF(int s) {
			sz = s;
			weight = new int[sz];
			parent = new int[sz];

			reset();
		}

		public void reset() {
			for (int i = 0; i < sz; i++) {
				weight[i] = 1;
				parent[i] = i;
			}
		}

		public int find(int v) {
			int orgV = v;
			int orgParent = parent[v];
			int orgWt = weight[v];

			while (v != parent[v]) {
				v = parent[v];
				weight[v] -= orgWt;
			}

			if (v != orgParent)
				weight[v] += orgWt;
			parent[orgV] = v;
			return v;
		}

		public void Union(int v, int w) {
			v = find(v);
			w = find(w);
			if (v == w) {
				return;
			} else if (weight[v] > weight[w]) {
				weight[v] += weight[w];
				parent[w] = v;
			} else {
				weight[w] += weight[v];
				parent[v] = w;
			}
		}
	}

	private static int MAXVERTICES = 1000;
	private static int MAXEDGES = 25000;

	private Edge[] edgeList = new Edge[MAXEDGES];
	private int nr_edges = 0;
	private UF uf = new UF(MAXVERTICES + 10);
	private ArrayList<Edge> heavyEdges;

	public Main() {}

	public void run() {
		int n, m;
		int u, v, w;
		int i;
		Scanner sc = new Scanner(System.in);

		while (true) {
			n = sc.nextInt();
			m = sc.nextInt();

			if (n == 0 && m == 0)
				break;

			nr_edges = 0;
			for (i = 0; i < m; i++) {
				u = sc.nextInt();
				v = sc.nextInt();
				w = sc.nextInt();

				Edge e = new Edge(u, v, w);
				edgeList[nr_edges++] = e;
			}

			runMst(n, m);
		}
	}

	private void runMst(int vertices, int numEdges) {
		Arrays.sort(edgeList, 0, nr_edges);
		uf.reset();
		//heavyEdges = new ArrayList<Edge>();

		int i;
		int v, w, wt;
		int numCC = vertices;
		int numHeavy = 0;
		for (i = 0; i < numEdges && numCC > 1; i++) {
			Edge edge = edgeList[i];
			v = uf.find(edge.from);
			w = uf.find(edge.to);
			wt = edge.wt;

			if (v == w) {
				if (numHeavy++ > 0)
					System.out.print(" ");
				System.out.print(wt);
				continue;
			}

			uf.Union(v, w);
			numCC--;
		}

		for (; i < numEdges; i++) {
			Edge edge = edgeList[i];
			wt = edge.wt;
			if (numHeavy++ > 0)
				System.out.print(" ");
			System.out.print(wt);
		}

		if (numHeavy == 0)
			System.out.println("forest");
		else
			System.out.println();
	}

	public static void main(String[] args) {
		Main m = new Main();
		m.run();
	}
}
