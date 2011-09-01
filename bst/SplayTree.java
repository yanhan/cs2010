package cs2010.bst;

import java.util.Stack;
import java.util.LinkedList;

public class SplayTree {
	private class Node {
		private int key;
		private Node left;
		private Node right;

		public Node(int k) {
			key = k;
			left = right = null;
		}

		public int getKey() {
			return key;
		}

		public Node getLeft() {
			return left;
		}

		public Node getRight() {
			return right;
		}

		public void setKey(int k) {
			key = k;
		}

		public void setLeft(Node l) {
			left = l;
		}

		public void setRight(Node r) {
			right = r;
		}

		public Node rotateLeft() {
			assert(this.right != null);
			Node newRoot = this.right;
			Node orphanLeft = newRoot.getLeft();

			newRoot.setLeft(this);
			this.setRight(orphanLeft);
			return newRoot;
		}

		public Node rotateRight() {
			assert(this.left != null);
			Node newRoot = this.left;
			Node orphanRight = newRoot.getRight();

			newRoot.setRight(this);
			this.setLeft(orphanRight);
			return newRoot;
		}
	}

	enum DIRECTION {
		LEFT, RIGHT
	}

	private Node root;

	public SplayTree() {
		root = null;
	}

	/* Zig rotations:
	 *
	 *       5                  Splay                 -20
	 *     /   \                 -20                    \
	 *   -20    200             ----->                   5
	 *                                                    \
	 *                                                    200
	 *
	 * Zig-Zig rotations:
	 *
	 *            5             Splay         -100
	 *         /     \          -100          /    \
	 *       -20      357       ----->      -123   -20
	 *       / \                                    / \
	 *    -100 -5                                 -50  5
	 *    /  \                                        / \
	 *  -123 -50                                     -5  357
	 *
	 * Zig-Zag rotations:
	 *
	 *           5              Splay                   -5
	 *        /     \            -5                   /    \
	 *      -20     357         ----->              -20     5
	 *      /  \                                    /        \
	 *    -100  -5                                -100       357
	 *    /  \                                    /  \
	 * -123  -50                                -123  50
	 *
	 */

	public void insert(int k) {
		Node cur;
		Node parent;
		Node grandparent;

		Stack<Node> st = new Stack<Node>();
		Stack<DIRECTION> dirs = new Stack<DIRECTION>();
		int tmpKey;
		DIRECTION curDir = DIRECTION.LEFT;
		DIRECTION parentDir, grandparentDir;

		if (root == null) {
			cur = new Node(k);
			root = cur;
			return;
		}

		cur = root;
		parent = null;
		while (cur != null) {
			parent = cur;
			tmpKey = cur.getKey();
			st.push(parent);

			if (k < tmpKey) {
				cur = cur.getLeft();
				curDir = DIRECTION.LEFT;
				dirs.push(DIRECTION.LEFT);
			} else if (k == tmpKey) {
				return;
			} else {
				cur = cur.getRight();
				curDir = DIRECTION.RIGHT;
				dirs.push(DIRECTION.RIGHT);
			}
		}

		cur = new Node(k);
		if (curDir == DIRECTION.LEFT)
			parent.setLeft(cur);
		else
			parent.setRight(cur);

		root = splayRotate(cur, st, dirs);
	}

	private Node splayRotate(Node newRoot, Stack<Node> st,
							 Stack<DIRECTION> dirs) {
		assert(newRoot != null);
		Node cur = newRoot;;
		Node parent;
		DIRECTION curDir;
		DIRECTION parentDir;

		st.push(newRoot);

		/* Hacks to splay the node up to root... */
		while (!st.isEmpty()) {
			cur = st.pop();
			if (st.isEmpty())
				break;

			curDir = dirs.pop();

			if (st.size() >= 2) {
				cur = zigzag(curDir, st, dirs);
			} else if (st.size() == 1) {
				/* Zig rotation */
				parent = st.pop();
				if (curDir == DIRECTION.LEFT)
					cur = parent.rotateRight();
				else
					cur = parent.rotateLeft();
			}
		}

		return cur;
	}

	private Node zigzag(DIRECTION curDir, Stack<Node> st, Stack<DIRECTION> dirs) {
		Node cur;
		Node parent;
		Node grandparent;
		DIRECTION parentDir;

		parent = st.pop();
		grandparent = st.pop();
		parentDir = dirs.pop();

		if (curDir == parentDir) {
			/* Zig-Zig rotation */
			if (parentDir == DIRECTION.LEFT) {
				grandparent.rotateRight();
				cur = parent.rotateRight();
			} else {
				grandparent.rotateLeft();
				cur = parent.rotateLeft();
			}
		} else {
			/* Zig-Zag rotation */
			if (curDir == DIRECTION.LEFT)
				cur = parent.rotateRight();
			else
				cur = parent.rotateLeft();

			if (parentDir == DIRECTION.LEFT) {
				grandparent.setLeft(cur);
				cur = grandparent.rotateRight();
			} else {
				grandparent.setRight(cur);
				cur = grandparent.rotateLeft();
			}
		}

		if (!st.isEmpty()) {
			Node gp = st.peek();
			DIRECTION gpdir = dirs.peek();

			if (gpdir == DIRECTION.LEFT)
				gp.setLeft(cur);
			else
				gp.setRight(cur);
		}

		/*
		 * Handle case where we pop off great grandparent the
		 * next iteration of the loop if st.size() == 1.
		 *
		 * Current node is child of great grandparent, but if
		 * great grandparent is the only node left on the stack,
		 * the current node will not be rotated to the root.
		 * Push current node onto the stack.
		 */
		st.push(cur);
		return cur;
	}

	/* Returns 1 if key is in tree, 0 otherwise */
	public int search(int k) {
		if (root == null)
			return 0;

		Node parent;
		Node cur;
		DIRECTION curDir;
		DIRECTION parentDir;
		int curKey;

		curDir = DIRECTION.LEFT; /* Silence compiler */
		Stack<Node> st = new Stack<Node>();
		Stack<DIRECTION> dirs = new Stack<DIRECTION>();

		cur = root;
		while (cur != null) {
			parent = cur;
			curKey = cur.getKey();

			if (k < curKey) {
				cur = cur.getLeft();
				curDir = DIRECTION.LEFT;
				dirs.push(curDir);
			} else if (k == curKey) {
				/* Need to splay node up to root! */
				break;
			} else {
				cur = cur.getRight();
				curDir = DIRECTION.RIGHT;
				dirs.push(curDir);
			}

			/*
			 * Only push parent here since we can accidentally
			 * push current node when it's a search hit.
			 */
			st.push(parent);
		}

		if (cur == null)
			return 0;

		root = splayRotate(cur, st, dirs);
		return 1;
	}

	public void printInner(Node n) {
		if (n == null)
			return;

		System.out.printf("Key: %d, lc: ", n.getKey());
		if (n.getLeft() != null)
			System.out.print(n.getLeft().getKey());
		else
			System.out.print("null");
		System.out.print(", rc: ");

		if (n.getRight() != null)
			System.out.print(n.getRight().getKey());
		else
			System.out.print("null");
		System.out.println();
		printInner(n.getLeft());
		printInner(n.getRight());
	}

	public void print() {
		if (root == null) {
			System.out.println("Empty tree");
			return;
		}

		printInner(root);
	}

	public static int sptTest() {
		/* Only test
		 *
		 * Insert the following keys:
		 * 13, 7, 3, 56, 9, 58, 162, 33
		 *
		 * This should print:
		 *
		 *              33
		 *            /    \
		 *           13    162
		 *          /       /
		 *         9      56
		 *        /         \
		 *       3           58
		 *        \
		 *         7
		 */
		SplayTree spt = new SplayTree();
		spt.insert(13);
		spt.insert(7);
		spt.insert(3);
		spt.insert(56);
		spt.insert(9);
		spt.insert(58);
		spt.insert(162);
		spt.insert(33);
		System.out.println("\nTree:");
		spt.print();

		/* Searching for 7 should turn the tree into:
		 *
		 *           7
		 *         /   \
		 *        3     13
		 *             /  \
		 *            9   33
		 *                  \
		 *                  162
		 *                  /
		 *                 56
		 *                  \
		 *                  58
		 */
		int error, ret;
		error = 0;

		ret = spt.search(7);
		error += ret == 1 ? 0 : 1;

		System.out.println("\nTree:");
		spt.print();

		ret = spt.search(-3);
		error += ret == 1? 1 : 0;

		return error;
	}

	public static void main(String args[]) {
		int ret = sptTest();
		if (ret > 0)
			System.out.println("Test Search failed");
		else
			System.out.println("Test Search passed");
	}
}
