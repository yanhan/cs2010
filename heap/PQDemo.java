package cs2010.heap;

import java.util.PriorityQueue;

public class PQDemo {
	private void run() {
		PriorityQueue<Woman> pq = new PriorityQueue<Woman>();
		Woman jane     = new Woman("Jane", 5);
		Woman ann      = new Woman("Ann", 5);
		Woman michelle = new Woman("Michelle", 4);
		Woman jodi     = new Woman("Jodi", 3);
		Woman eve      = new Woman("Eve", 2);
		Woman caroline = new Woman("Caroline", 3);
		Woman joan     = new Woman("Joan", 1);
		Woman nicole   = new Woman("Nicole", 2);
		Woman jennifer = new Woman("Jennifer", 1);

		pq.offer(jane);
		pq.offer(ann);
		pq.offer(michelle);
		pq.offer(jodi);
		pq.offer(eve);
		pq.offer(caroline);
		pq.offer(joan);
		pq.offer(nicole);
		pq.offer(jennifer);

		while (!pq.isEmpty()) {
			Woman w = pq.poll();
			System.out.print(w);
		}
	}

	public static void main(String args[]) {
		PQDemo p = new PQDemo();
		p.run();
	}
}
