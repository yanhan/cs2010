import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Scanner;
import java.util.Vector;


/**
 * @author Shubham Goyal
 *
 */
public class Main {
	public static void main(String args[])throws IOException {
		Scanner sc = new Scanner(System.in);
		String s;
		String t;
		int c = 0;
		while((!(s = sc.next()).equals("0")) && (!(t = sc.next()).equals("0"))) {
			c ++;
			int count = 0;
			HashMap<String, Integer> hash = new HashMap<String, Integer>();
			Vector<Vector<Integer>> adjList = new Vector<Vector<Integer>>();
			int P = Integer.parseInt(s);
			int R = Integer.parseInt(t);
			String relationships[][] = new String[R][2];
			for(int i = 0; i < R; i ++) {
				relationships[i][0] = sc.next();
				if(!hash.containsKey(relationships[i][0])) {
					hash.put(relationships[i][0], count);
					count ++;
					adjList.add(new Vector<Integer>());
				}
				relationships[i][1] = sc.next();
				if(!hash.containsKey(relationships[i][1])) {
					hash.put(relationships[i][1], count);
					count ++;
					adjList.add(new Vector<Integer>());
				}
				adjList.get(hash.get(relationships[i][0])).add(hash.get(relationships[i][1]));
				adjList.get(hash.get(relationships[i][1])).add(hash.get(relationships[i][0]));
			}
			int V = count;
			int distance[][] = new int[V][V];
			int breadth = 0;
			boolean disconnected = false;
			if (V < P)
				disconnected = true;
			for(int i = 0; i < V; i ++) {
				LinkedList<Integer> list = new LinkedList<Integer>();
				boolean present[] = new boolean[V];
				for(int j = 0; j < V; j ++) {
					present[j] = false;
				}
				list.add(i);
				present[i] = true;
				distance[i][i] = 0;
				while(!list.isEmpty()) {
					int vertex = list.poll();
					for(int j = 0; j < adjList.get(vertex).size(); j ++) {
						if(!present[adjList.get(vertex).get(j)]) {
							list.add(adjList.get(vertex).get(j));
							present[adjList.get(vertex).get(j)] = true;
							distance[i][adjList.get(vertex).get(j)] = distance[i][vertex] + 1;
							if(distance[i][adjList.get(vertex).get(j)] > breadth)
								breadth = distance[i][adjList.get(vertex).get(j)];
						}
					}
				}
				for(int j = 0; j < V; j ++) {
					if(!present[j]) {
						disconnected = true;
						break;
					}
				}
				if(disconnected)
					break;
			}
			
			if(disconnected)
				System.out.println("Network " + c + ": DISCONNECTED\n");
			else
				System.out.println("Network " + c + ": " + breadth + "\n");
		}
	}
}
