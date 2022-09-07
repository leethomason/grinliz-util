#include "gltree.h"
#include "glrandom.h"

using namespace grinliz;

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
	return true;
}
