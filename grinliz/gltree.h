#pragma once

#include <glm/glm.hpp>

#include "glrectangle.h"
#include "glcontainer.h"	// sort: move to own header?

namespace grinliz {
	template<typename T>
	class Tree
	{
	public:
		struct Data {
			glm::vec3 p;
			T t;
		};

		struct Node {
			int start = 0;
			int count = 0;
			int splitAxis = -1;
			float splitValue = 0;
			Rect3F bounds;
		};

		void Add(const glm::vec3& p, const T& t) {
			m_nodes[0].bounds.DoUnion(p);
			m_nodes[0].count++;
			m_data.push_back({ p, t });
		}

		void Sort() {
			SplitNode(m_nodes);
		}
		void Query(const glm::vec3& rectMin, const glm::vec3& rectMax, std::vector<Tree::Node>& nodes);
		void Clear();

		int numNodes = 0;

	private:
		static constexpr int N_NODES = 200;		// TUNE
		static constexpr int SMALL_NODE = 16;	// probably too small...TUNE

		//    depth      left right
		// 0      0		 1	  2			+1 +2	Left delta: index+1, Right: index + 2
		// 1      1 L	 3    4			+2 +3
		// 2      1 R    5    6         +3 +4
		// 3      2 L    7    8			+4 +5
		// 4      2 R    8    9			+5 +6
		// 5      2 L   10   11
		// 6      2 R   12   13
		// 7      3 L   14   15
		// ...

		enum {
			LEFT, RIGHT
		};

		Node* Child(Node* current, int dir) {
			int myIndex = current - m_nodes;
			int delta = myIndex + (dir + 1);
			int index = myIndex + delta;
			if (index < N_NODES)
				return &m_nodes[index];
			return nullptr;
		}

		void SplitNode(Node* node) {
			numNodes += 1;
			node->splitAxis = -1;	// no split by default.

			if (node->count < SMALL_NODE)
				return;
			Node* left = Child(node, LEFT);
			Node* right = Child(node, RIGHT);
			if (!left || !right)
				return;

			int splitAxis = -1;
			float size = 0.0;
			for (int i = 0; i < 3; ++i) {
				if (node->bounds.size[i] > size) { // fixme: check numeric "can split"
					splitAxis = i;
					size = node->bounds.size[i];
				}
			}
			if (splitAxis == -1) {
				// Leaf
				node->splitAxis = -1;
				return;
			}
			node->splitAxis = splitAxis;
			Data* start = &m_data[node->start];
			Data* end = start + node->count;
			// Sort the data on the splitting axis.
			grinliz::Sort(start, node->count, [ax = node->splitAxis](const Data& a, const Data& b) {
				return a.p[ax] < b.p[ax];
				});
			node->splitValue = start[node->count / 2].p[splitAxis];
			float splitVal = node->splitValue;

			*left = Node();
			*right = Node();
			Data* p = start;

			left->start = node->start;
			for (; p < end && p->p[splitAxis] < splitVal; ++p) {
				left->bounds.DoUnion(p->p);
				left->count++;
			}
			right->start = p - &m_data[0];
			for (; p < end; ++p) {
				right->bounds.DoUnion(p->p);
				right->count++;
			}
			
			SplitNode(left);
			SplitNode(right);
		}

		Node m_nodes[N_NODES];
		std::vector<Data> m_data;
	};

	bool TreeTest();
}