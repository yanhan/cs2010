package cs2010.bst;

import java.util.Stack;
import java.util.LinkedList;

public class BST {
	private class Node {
		private int height;
		private int key;
		private Node left;
		private Node right;

		public Node(int k) {
			key = k;
			height = 1;
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

		public int getHeight() {
			return height;
		}

		public void setLeft(Node x) {
			left = x;
		}

		public void setRight(Node x) {
			right = x;
		}

		public void updateHeight() {
			int left_ht = left == null ? 0 : left.getHeight();
			int right_ht = right == null ? 0 : right.getHeight();
			height = Math.max(left_ht, right_ht) + 1;
		}

		private Node rotateRight() {
			assert(left != null);
			Node newParent = left;
			Node orphanRight = left.getRight();
			newParent.setRight(this);
			this.left = orphanRight;
			updateHeight();
			newParent.updateHeight();
			return newParent;
		}

		private Node rotateLeft() {
			assert(right != null);
			Node newParent = right;
			Node orphanLeft = newParent.getLeft();
			newParent.setLeft(this);
			this.right = orphanLeft;
			updateHeight();
			newParent.updateHeight();
			return newParent;
		}

		private Node rebalance() {
			if (left == null && right == null)
				return this;

			int left_ht = left == null ? 0 : left.getHeight();
			int right_ht = right == null ? 0 : right.getHeight();

			if (Math.abs(left_ht - right_ht) <= 1)
				return this;

			if (left_ht > right_ht)
				return rotateRight();
			else
				return rotateLeft();
		}
	}

	public class NodeWrap {
		private Node node;
		int ht;

		public NodeWrap(Node n, int height) {
			node = n;
			ht = height;
		}

		public Node getNode() {
			return node;
		}

		public int getHeight() {
			return ht;
		}
	}

	enum DIRECTION {
		LEFT, RIGHT
	}

	private Node root;

	public BST() {
		root = null;
	}

	public void insert(int key) {
		if (root == null) {
			root = new Node(key);
			return;
		}

		Node parent;
		Node cur;
		DIRECTION dir;
		Stack<Node> st;
		Stack<DIRECTION> direction;

		parent = null;
		cur = root;
		dir = DIRECTION.LEFT;
		st = new Stack<Node>();
		direction = new Stack<DIRECTION>();

		while (cur != null) {
			parent = cur;
			st.push(parent);
			if (key < cur.getKey()) {
				cur = cur.getLeft();
				dir = DIRECTION.LEFT;
				direction.push(DIRECTION.LEFT);
			} else if (key == cur.getKey()) {
				return;
			} else {
				cur = cur.getRight();
				dir = DIRECTION.RIGHT;
				direction.push(DIRECTION.RIGHT);
			}
		}

		cur = new Node(key);
		if (dir == DIRECTION.LEFT)
			parent.setLeft(cur);
		else
			parent.setRight(cur);

		Node prev = cur;
		while (!st.isEmpty()) {
			cur = st.pop();
			dir = direction.pop();

			if (dir == DIRECTION.LEFT)
				cur.setLeft(prev);
			else
				cur.setRight(prev);

			cur.updateHeight();
			prev = cur.rebalance();
		}

		root = prev;
	}

	private void inorderPrint1(Node n) {
		if (n == null)
			return;

		inorderPrint1(n.getLeft());
		System.out.print(n.getKey() + " ");
		inorderPrint1(n.getRight());
	}

	public void inorderPrint() {
		if (root == null) {
			System.out.print("Empty tree");
		} else {
			System.out.println("Inorder traversal:");
			inorderPrint1(root);
		}

		System.out.printf("\n\n");
	}

	public void levelPrint() {
		if (root == null) {
			System.out.println("Empty tree");
			return;
		}

		System.out.println("Level order traversal:");

		LinkedList<NodeWrap> queue = new LinkedList<NodeWrap>();
		NodeWrap cur;
		Node n;
		int cur_ht = 0;

		queue.add(new NodeWrap(root, cur_ht));
		while (!queue.isEmpty()) {
			cur = queue.poll();
			n = cur.getNode();

			if (cur.getHeight() > cur_ht) {
				System.out.println();
				cur_ht++;
			}

			/* Shouldnt happen */
			if (n == null)
				continue;

			if (n.getLeft() != null)
				queue.add(new NodeWrap(n.getLeft(), cur_ht + 1));
			if (n.getRight() != null)
				queue.add(new NodeWrap(n.getRight(), cur_ht + 1));

			System.out.print(n.getKey() + " ");
		}

		System.out.println();
	}

	private void printNode(Node n) {
		if (n == null)
			return;
		System.out.printf("Node %d, left_ht = %d, right_ht = %d, ht = %d\n",
			n.getKey(),
			n.getLeft() != null ? n.getLeft().getHeight() : 0,
			n.getRight() != null ? n.getRight().getHeight() : 0,
			n.getHeight());
		printNode(n.getLeft());
		printNode(n.getRight());
	}

	public void print() {
		printNode(root);
	}

	public static void main(String args[]) {
		BST bst = new BST();
		for (int i = 20; i > 0; i--)
			bst.insert(i);
		bst.inorderPrint();
		bst.levelPrint();
	}
}
