#include "gltree.h"
#include "glrandom.h"

using namespace grinliz;

void PrintNode(int depth, const Tree<int>::Node* node, const Tree<int>& tree)
{
	for (int i = 0; i < depth; ++i) printf(" ");

	const Tree<int>::Node* left = tree.Child(node, 0);
	const Tree<int>::Node* right = tree.Child(node, 1);

	printf("%3d: (%.2f,%.2f,%.2f) -  (%.2f,%.2f,%.2f) %s start=%d count=%d\n",
		depth,
		node->bounds.Lower().x,
		node->bounds.Lower().y,
		node->bounds.Lower().z,
		node->bounds.Upper().x,
		node->bounds.Upper().y,
		node->bounds.Upper().z,
		(left && right) ? "" : "LEAF: ",
		node->start,
		node->count);

	if (left && right) {
		PrintNode(depth + 1, left, tree);
		PrintNode(depth + 1, right, tree);
	}
}


bool grinliz::TreeTest()
{
	constexpr int N = 1000;
	Random rand;

	std::vector<glm::vec3> input;
	input.reserve(N);
	for (int i = 0; i < N; ++i) {
		glm::vec3 v = { rand.Uniform(), rand.Uniform(), rand.Uniform() };
		input.push_back(v);
	}

	Tree<int> tree;
	for (int i = 0; i < N; ++i) {
		tree.Add(input[i], i);
	}
	tree.Sort();
	printf("m_nodes=%d\n", tree.numNodes);

	PrintNode(0, tree.Root(), tree);
	return true;
}
