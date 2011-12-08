import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;

/**
 * @author Shubham Goyal
 *
 */

public class Main {
	static int N;
	static ArrayList<ArrayList<Integer>> memo;
	public static void main(String args[]) throws IOException {
		Scanner sc = new Scanner(System.in);
		int num = sc.nextInt();
		for(int i = 0; i < num; i ++) {
			int xMax = sc.nextInt();
			int yMax = sc.nextInt();
			int xStart = sc.nextInt();
			int yStart = sc.nextInt();
			N = sc.nextInt();
			int pos[][] = new int[N+1][2];
			pos[0][0] = xStart;
			pos[0][1] = yStart;
			for(int j = 1; j <= N; j ++) {
				pos[j][0] = sc.nextInt();
				pos[j][1] = sc.nextInt();
			}
			
			memo = new ArrayList<ArrayList<Integer>>();
			for(int j = 0; j < (N + 1); j ++) {
				memo.add(new ArrayList<Integer>());
				for(int k = 0; k < (1 << (N + 1)); k ++) {
					memo.get(j).add(-1);
				}
			}
			int AdjMatrix[][] = new int[N+1][N+1];
			for(int j = 0; j < (N + 1); j ++) {
				for(int k = 0; k < (N + 1); k ++) {
					int xDiff, yDiff;
					xDiff = pos[j][0] - pos[k][0];
					if(xDiff < 0)
						xDiff = - xDiff;
					yDiff = pos[j][1] - pos[k][1];
					if(yDiff < 0)
						yDiff = - yDiff;
					AdjMatrix[j][k] = xDiff + yDiff;
				}
			}
			System.out.println("The shortest path has length " + DP_TSP(0, 1 << 0, AdjMatrix));
		}
	}
	
	private static int DP_TSP(int u, int vis, int AdjMatrix[][]) {
		if(vis == ((1 << (N + 1)) - 1))
			return AdjMatrix[u][0];
		if(memo.get(u).get(vis) != -1)
			return memo.get(u).get(vis);
		int bestAns = Integer.MAX_VALUE/2;
		for(int i = 0; i < (N + 1); i ++) {
			if(((AdjMatrix[u][i] > 0) || ((AdjMatrix[u][i] == 0) && (u != i))) && ((vis & (1 << i)) == 0))
				bestAns = Math.min(bestAns, AdjMatrix[u][i] + DP_TSP(i, vis | (1 << i), AdjMatrix));
		}
		memo.get(u).set(vis, bestAns);
		return bestAns;
	}
}