package cs2010.dp2.knapsack;

public class Knapsack {
	public static final int TOTALITEMS = 5;
	public static final int MAXWEIGHT = 120;

	private int[] value;
	private int[] weight;
	private int[][] DP;

	public Knapsack() {
		value  = new int[] { 50,  80, 20, 60, 40 };
		weight = new int[] { 45, 100, 10, 65, 60 };
		DP = new int[TOTALITEMS + 1][MAXWEIGHT + 1];
	}

	public void run() {
		int i, k;
		for (i = 0; i <= TOTALITEMS; i++) {
			for (k = 0; k <= MAXWEIGHT; k++)
				DP[i][k] = -1;
		}

		/* Initial state: No items and no weight */
		DP[0][0] = 0;
		for (i = 0; i < TOTALITEMS; i++) {
			for (k = 0; k <= MAXWEIGHT; k++) {
				if (DP[i][k] == -1)
					continue;

				/*
				 * Try taking current item if it does not exceed total weight
				 * we can carry
				 */

				int newWeight = k + weight[i];
				int newVal = DP[i][k] + value[i];
				if (newWeight <= MAXWEIGHT)
					DP[i+1][newWeight] = Math.max(DP[i+1][newWeight], newVal);

				/* Skip current item */
				DP[i+1][k] = Math.max(DP[i+1][k], DP[i][k]);
			}
		}

		int maxVal = 0;
		int weight = 0;
		for (i = 0; i <= MAXWEIGHT; i++) {
			if (DP[TOTALITEMS][i] > maxVal) {
				maxVal = DP[TOTALITEMS][i];
				weight = i;
			}
		}

		System.out.println("Maximum value is " + maxVal + " at weight " + weight);
	}

	public static void main(String[] args) {
		Knapsack k = new Knapsack();
		k.run();
	}
}
