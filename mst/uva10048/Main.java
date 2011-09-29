import java.util.PriorityQueue;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.Vector;

class Main {
	class Edge implements Comparable<Edge> {
		int v;
		int w;
		int wt;
		public Edge(int vv, int ww, int wtt) {
			v = vv;
			w = ww;
			wt = wtt;
		}

		public int compareTo(Edge o) {
			if (wt != o.wt)
				return wt - o.wt;
			else if (v != o.v)
				return v - o.v;
			else
				return w - o.w;
		}
	}

	class IntPair {
		int first;
		int sec;

		public IntPair(int a, int b) {
			first = a;
			sec = b;
		}
	}

	private static int MAXVERTICES = 100;

	private Vector<Vector<Edge> > adj;
	private Vector<IntPair> queries;
	private boolean[] intree = new boolean[MAXVERTICES + 10];
	private int[] distance = new int[MAXVERTICES + 10];

	public Main() {}

	public void runMst(int ncase) {
		int i;
		int start, end;
		int stop = queries.size();
		int largestEdge;

		if (ncase > 1)
			System.out.println();
		System.out.println("Case #" + ncase);
		for (i = 0; i < stop; i++) {
			resetIntree();

			IntPair query = queries.get(i);
			start = query.first;
			end = query.sec;

			intree[start] = true;
			distance[start] = 0;
			PriorityQueue<Edge> pq = new PriorityQueue<Edge>();

			Vector<Edge> sadj = adj.get(start);
			for (Edge edge: sadj)
				pq.offer(edge);

			largestEdge = Integer.MIN_VALUE;
			while (!pq.isEmpty()) {
				if (intree[start] && intree[end])
					break;
				Edge edge = pq.poll();
				if (intree[edge.w])
					continue;

				intree[edge.w] = true;
				if (edge.wt > largestEdge)
					largestEdge = edge.wt;
				distance[edge.w] = distance[edge.v] + edge.wt;
				Vector<Edge> wadj = adj.get(edge.w);

				for (Edge next: wadj) {
					if (intree[next.w])
						continue;
					pq.offer(next);
				}
			}

			if (distance[end] == Integer.MAX_VALUE)
				System.out.println("no path");
			else
				System.out.println(largestEdge);
		}
	}

	private void resetIntree() {
		for (int i = 0; i < intree.length; i++) {
			intree[i] = false;
			distance[i] = Integer.MAX_VALUE;
		}
	}

	public void run() {
		int C, S, Q;
		int i;
		int c1, c2, d;
		int q1, q2;
		int ncase = 1;
		Scanner sc = new Scanner(System.in);

		while (true) {
			C = sc.nextInt();
			S = sc.nextInt();
			Q = sc.nextInt();

			if (C == 0 && S == 0 && Q == 0)
				break;

			adj = new Vector<Vector<Edge> >();
			queries = new Vector<IntPair>();

			for (i = 0; i <= C; i++) {
				Vector<Edge> vec = new Vector<Edge>();
				adj.add(vec);
			}

			for (i = 0; i < S; i++) {
				c1 = sc.nextInt();
				c2 = sc.nextInt();
				d  = sc.nextInt();

				Edge edge = new Edge(c1, c2, d);
				Edge edge2 = new Edge(c2, c1, d);
				adj.get(c1).add(edge);
				adj.get(c2).add(edge2);
			}

			for (i = 0; i < Q; i++) {
				q1 = sc.nextInt();
				q2 = sc.nextInt();
				queries.add(new IntPair(q1, q2));
			}

			runMst(ncase++);
		}
	}

	public static void main(String[] args) {
		Main m = new Main();
		m.run();
	}
}
