package cs2010.heap;

public class Woman implements Comparable<Woman> {
	private String name;
	private int desirability;

	private static int maxlen = 0;

	public Woman(String n, int d) {
		name = n;
		desirability = d;

		if (n.length() > maxlen)
			maxlen = n.length();
	}

	public int compareTo(Woman w) {
		if (desirability != w.desirability)
			return desirability - w.desirability;
		else
			return name.compareTo(w.name);
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(name);
		sb.append(", ");

		int diffLen = maxlen - name.length();
		for (int i = 0; i < diffLen; i++)
			sb.append(' ');

		sb.append("desirability = ");
		sb.append(desirability);
		sb.append('\n');
		return sb.toString();
	}
}
