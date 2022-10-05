#pragma once

#include <glm/glm.hpp>

#include "glrectangle.h"
#include "glcontainer.h"	// sort: move to own header?

namespace grinliz {
	bool TreeTest();

	// R: Rect2F or Rect3F
	// T: some small-ish type (int, entity, id, etc.)
	template<typename R, typename T>
	class Tree
	{
		friend bool TreeTest();
		using V = typename R::Vec_t;

	public:
		struct Data {
			V pos;
			T key;
		};

		struct Node {
			int start = 0;
			int count = 0;
			int splitAxis = -1;
			float splitValue = 0;
			R bounds;

			bool Leaf() const { return splitAxis < 0; }
		};

		void Add(const V& p, const T& t) {
			m_nodes[0].bounds.DoUnion(p);
			m_nodes[0].count++;
			m_data.push_back({ p, t });
		}

		void Sort() {
			SplitNode(m_nodes);
		}

		// Be wary that the right side of rect is exclusive
		// Note this is constant and thread safe, so multiple
		// threads (each with their own std::vector) can query
		// at the same time.
		void Query(const R& rect, std::vector<Tree::Data>& r) const {
			r.clear();
			QueryRec(rect, r, m_nodes);
		}

		void Clear() {
			m_data.clear();
			m_nodes[0] = Node();
			numNodes = 0;
		}

		// Debugging / perf
		int numNodes = 0;

		const Node* Root() const { return m_nodes; }
		const Node* Child(const Node* node, int dir) const {
			if (node->Leaf())
				return nullptr;
			intptr_t myIndex = node - m_nodes;
			intptr_t delta = myIndex + (dir + 1);
			intptr_t index = myIndex + delta;
			if (index < N_NODES)
				return &m_nodes[index];
			return nullptr;
		}

	private:
		enum { LEFT, RIGHT };
		static constexpr int N_NODES = 800;
		static constexpr int SMALL_NODE = 32;

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

		Node* Child(const Node* current, int dir) {
			intptr_t myIndex = current - m_nodes;
			intptr_t delta = myIndex + (dir + 1);
			intptr_t index = myIndex + delta;
			if (index < N_NODES)
				return &m_nodes[index];
			return nullptr;
		}

		int FindSplitAxis(const Node* node) const {
			int splitAxis = -1;
			float size = 0.0;
			for (int i = 0; i < V::length(); ++i) {
				if (node->bounds.size[i] > size) {
					splitAxis = i;
					size = node->bounds.size[i];
				}
			}
			return splitAxis;
		}

		void SplitNode(Node* node) {
			numNodes += 1;
			node->splitAxis = -1;	// leaf node by default

			// Don't split small ones - no point,
			// and not enough samples for the algorithms below.
			if (node->count < SMALL_NODE) return;

			Node* left = Child(node, LEFT);
			Node* right = Child(node, RIGHT);
			if (!left || !right) return;	// not enough nodes left.

			node->splitAxis = FindSplitAxis(node);
			if (node->Leaf()) return;

			Data* start = &m_data[node->start];
			Data* end = start + node->count;

			// Sort the data on the splitting axis.
#if false
			// About 6ms @10k
			grinliz::Sort(start, node->count, [ax = node->splitAxis](const Data& a, const Data& b) {
				return a.p[ax] < b.p[ax];
				});
#endif
			// About 4ms @10k
			std::sort(start, end, [ax = node->splitAxis](const Data& a, const Data& b) {
				return a.pos[ax] < b.pos[ax];
				});

			// Choose the median split value. Note it is possible this 
			// creates a pathelogical case, where there is only 1 data
			// in a node. (Or possibly - with numerical issues - 0?). But
			// it won't break, and this is clearly intended to work 
			// with a distribution.
			node->splitValue = start[node->count / 2].pos[node->splitAxis];

			*left = Node();
			*right = Node();
			Data* p = start;

			left->start = node->start;
			for (; p < end && p->pos[node->splitAxis] < node->splitValue; ++p) {
				left->bounds.DoUnion(p->pos);
				left->count++;
			}
			right->start = int(p - &m_data[0]);
			for (; p < end; ++p) {
				right->bounds.DoUnion(p->pos);
				right->count++;
			}
			
			SplitNode(left);
			SplitNode(right);
		}

		void QueryRec(const R& rect, std::vector<Tree::Data>& r, const Node* node) const {
			const Node* left = Child(node, LEFT);
			const Node* right = Child(node, RIGHT);

			if (left && left->bounds.Intersects(rect))
				QueryRec(rect, r, left);
			if (right && right->bounds.Intersects(rect))
				QueryRec(rect, r, right);

			if (node->Leaf()) {
				for (int i = 0; i < node->count; ++i) {
					if (rect.Contains(m_data[i + node->start].pos)) {
						r.push_back(m_data[i + node->start]);
					}
				}
			}
		}

		Node m_nodes[N_NODES];
		std::vector<Data> m_data;
	};
}