#include "gltree.h"
#include "glrandom.h"
#include "glperformance.h"

using namespace grinliz;

std::vector<glm::vec3> input;
std::vector<glm::vec2> input2;

void PrintNode(int depth, const Tree<Rect3F, int>::Node* node, const Tree<Rect3F, int>& tree)
{
	for (int i = 0; i < depth; ++i) printf(" ");

	const Tree<Rect3F, int>::Node* left = tree.Child(node, 0);
	const Tree<Rect3F, int>::Node* right = tree.Child(node, 1);

	printf("%3d: (%.2f,%.2f,%.2f) -  (%.2f,%.2f,%.2f) axis=%d @ %f %s start=%d count=%d\n",
		depth,
		node->bounds.Lower().x,
		node->bounds.Lower().y,
		node->bounds.Lower().z,
		node->bounds.Upper().x,
		node->bounds.Upper().y,
		node->bounds.Upper().z,
		node->splitAxis,
		node->splitValue,
		(left && right) ? "" : "LEAF: ",
		node->start,
		node->count);

	/*
	* This doesn't work, because the rightmost point is not
	* contained in the Rect3F, because of the Rect3F containment rules.
	for (int i = 0; i < node->count; ++i) {
		glm::vec3 p = input[i + node->start];
		GLASSERT(node->bounds.Contains(p));
	}
	*/

	if (left && right) {
		GLASSERT(node->bounds.Contains(left->bounds));
		GLASSERT(node->bounds.Contains(right->bounds));

		PrintNode(depth + 1, left, tree);
		PrintNode(depth + 1, right, tree);
	}
}


void PrintNode(int depth, const Tree<Rect2I, int>::Node* node, const Tree<Rect2I, int>& tree)
{
	for (int i = 0; i < depth; ++i) printf(" ");

	const Tree<Rect2I, int>::Node* left = tree.Child(node, 0);
	const Tree<Rect2I, int>::Node* right = tree.Child(node, 1);

	printf("%3d: (%d,%d) -  (%d,%d) axis=%d @ %d %s start=%d count=%d\n",
		depth,
		node->bounds.Lower().x,
		node->bounds.Lower().y,
		node->bounds.Upper().x,
		node->bounds.Upper().y,
		node->splitAxis,
		node->splitValue,
		(left && right) ? "" : "LEAF: ",
		node->start,
		node->count);

	/*
	* This doesn't work, because the rightmost point is not
	* contained in the Rect3F, because of the Rect3F containment rules.
	for (int i = 0; i < node->count; ++i) {
		glm::vec3 p = input[i + node->start];
		GLASSERT(node->bounds.Contains(p));
	}
	*/

	if (left && right) {
		GLASSERT(node->bounds.Contains(left->bounds));
		GLASSERT(node->bounds.Contains(right->bounds));

		PrintNode(depth + 1, left, tree);
		PrintNode(depth + 1, right, tree);
	}
}


struct Grid {
	int x, y;
};

bool grinliz::TreeTest()
{
	{
		// Print out a small tree for inspection & test it.
		constexpr int N = 200;
		Random rand;

		input.reserve(N);
		for (int i = 0; i < N; ++i) {
			glm::vec3 v = { rand.Uniform(), rand.Uniform(), rand.Uniform() };
			input.push_back(v);
		}

		Tree<Rect3F, int> tree;
		for (int i = 0; i < N; ++i) {
			tree.Add(input[i], i);
		}
		tree.Sort();
		printf("m_nodes=%d\n", tree.numNodes);

		PrintNode(0, tree.Root(), tree);

		std::vector<Tree<Rect3F, int>::Data> out;
		Rect3F rect({ 0, 0, 0 }, { 1, 1, 1 });
		tree.Query(rect, out);
		GLASSERT(out.size() == N);
	}
	{
		input.clear();
		Tree<Rect3F, Grid> tree;

		// A bigger tree on a 10x10x10 grid
		// Intentionally pathelogical.
		for (int y = 0; y < 10; ++y) {
			for (int x = 0; x < 10; ++x) {
				for (int i = 0; i < 10; ++i) {
					glm::vec3 v = { x, y, 0 };
					tree.Add(v, { x, y });
				}
			}
		}

		tree.Sort();

		{
			std::vector<Tree<Rect3F, Grid>::Data> out;
			Rect3F r({ -0.5, -0.5, -0.5 }, { 1, 1, 1 });
			tree.Query(r, out);
			GLASSERT(out.size() == 10);
			for (int i = 0; i < 10; ++i) {
				GLASSERT(out[i].value.x == 0);
				GLASSERT(out[i].value.y == 0);
			}
		}
		{
			std::vector<Tree<Rect3F, Grid>::Data> out;
			Rect3F r({ 8.5, 8.5, -0.5 }, { 1, 1, 1 });
			tree.Query(r, out);
			GLASSERT(out.size() == 10);
			for (int i = 0; i < 10; ++i) {
				GLASSERT(out[i].value.x == 9);
				GLASSERT(out[i].value.y == 9);
			}
		}
	}
	{
		for (int count = 0; count < 2; ++count) {
			// Target size performance. Well, 1000 is probably fine.
			// But lets do well at 10,000
			constexpr int N = 10'000;
			Random rand;

			input.clear();
			input.reserve(N);
			for (int i = 0; i < N; ++i) {
				glm::vec3 v = { rand.Uniform(), rand.Uniform(), rand.Uniform() };
				input.push_back(v);
			}

			int n = count == 0 ? 1000 : 10'000;

			printf("Rect3F run n=%d -------------- \n", n);

			Tree<Rect3F, int> tree;
			for (int i = 0; i < n; ++i) {
				tree.Add(input[i], i);
			}

			{
				QuickProfile profile(" sort");
				tree.Sort();
			}

			std::vector<Tree<Rect3F, int>::Data> out;

			int64_t checksum = 0;
			{
				QuickProfile profile("query");
				for (int i = 0; i < n; ++i) {
					Rect3F bounds({ rand.Uniform(), rand.Uniform(), rand.Uniform() }, { 0.05f, 0.05f, 0.05f });
					tree.Query(bounds, out);
					checksum += out.size();
				}
			}

			printf("Perf run n=%d. nNodes=%d check=%lld sizeof=%dk\n", n, tree.numNodes, checksum, int(sizeof(Tree<Rect3F, int>) / 1024));
		}
	}
	{
		Tree<Rect2I, int> tree;
		static const int SY = 37;
		static const int SX = 10;
		for (int y = 0; y < SY; ++y) {
			for (int x = 0; x < SX; ++x) {
				tree.Add({x,y}, y * SX + x);
			}
		}
		tree.Sort();
		PrintNode(0, tree.Root(), tree);
		std::vector< Tree<Rect2I, int>::Data > out;
		for (int y = 0; y < SY; ++y) {
			for (int x = 0; x < SX; ++x) {
				Rect2I r = { {x, y}, {1,1} };
				tree.Query(r, out);
				GLASSERT(out.size() == 1);
				GLASSERT(out[0].value == y * SX + x);
			}
		}
	}
	return true;
}
