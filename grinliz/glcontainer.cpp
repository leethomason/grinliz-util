/*
Copyright (c) 2000-2012 Lee Thomason (www.grinninglizard.com)
Grinning Lizard Utilities.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/

#include "glcontainer.h"
#include "glrandom.h"

using namespace grinliz;

void DynMemBuf::Move(DynMemBuf* target)
{
	Swap(target->mem, mem);
	Swap(target->cap, cap);
	Swap(target->size, size);
	size = 0;
}

void PacketQueue::Move(PacketQueue* queue)
{
	memBuf.Move(&queue->memBuf);
}


void PacketQueue::Push(int id, int size, const void* data)
{
	GLASSERT(id >= 0 && id < 10000);    // sanity check

	Header header = { id, size };
	memBuf.Add(header);
	if (size)
		memBuf.Add(data, size);
}

int PacketQueue::Pop(void* target, int targetSize)
{
	GLASSERT(memBuf.Size() >= sizeof(Header));
	Header header;
	memcpy(&header, memBuf.Mem(), sizeof(Header));
	GLASSERT(header.dataSize >= 0);

	if (header.dataSize) {
		GLASSERT(memBuf.Size() >= sizeof(Header) + header.dataSize);
		GLASSERT(targetSize == header.dataSize);
		memcpy(target, (uint8_t*)memBuf.Mem() + sizeof(Header), targetSize);
	}
	memBuf.DeleteFront(header.dataSize + sizeof(Header));
	return header.id;
}

int PacketQueue::Peek() const
{
	GLASSERT(memBuf.Size() >= sizeof(Header));
	Header header;
	memcpy(&header, memBuf.Mem(), sizeof(Header));
	return header.id;
}


int PacketQueue::Pop(DynMemBuf* target)
{
	GLASSERT(memBuf.Size() >= sizeof(Header));
	Header header;
	memcpy(&header, memBuf.Mem(), sizeof(Header));
	GLASSERT(header.dataSize >= 0);

	if (header.dataSize) {
		GLASSERT(memBuf.Size() >= sizeof(Header) + header.dataSize);
		target->Add((uint8_t*)memBuf.Mem() + sizeof(Header), header.dataSize);
	}
	memBuf.DeleteFront(header.dataSize + sizeof(Header));
	return header.id;
}


template<typename T>
bool InOrder(const T* mem, int N)
{
	for (int i = 0; i < N - 1; ++i) {
		bool inOrder = mem[i] < mem[i + 1];
		GLASSERT(inOrder);
		if (!inOrder) return false;
	}
	return true;
}

struct TestA {
	int a;
};

struct TestAB {
	int a;
	double b;
};

struct Vec2 {
	int x, y;
};

struct Vec2Hash {
	static uint32_t Hash(const Vec2& v) {
		return v.x + (v.y << 8);
	}
	static bool Equal(const Vec2& a, const Vec2& b)  {
		return a.x == b.x && a.y == b.y;
	}
};

void grinliz::TestContainers()
{
	Random random;
	// Global functions.
	{
		static const int N = 10;
		int arr[N] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		random.ShuffleArray(arr, N);

		grinliz::Sort(arr, N);
		GLASSERT(InOrder(arr, N));
		random.ShuffleArray(arr, N);

		grinliz::Sort(arr, N, [](const int& a, const int& b) {
			return a < b;
		});
		GLASSERT(InOrder(arr, N));
		random.ShuffleArray(arr, N);

		struct Context {
			int origin = -1;
			bool Less(const int&a, const int& b) const {
				return (origin + a) < (origin + b);
			}
		};
	}
	// CArray
	{
		CArray<int, 10> arr;
		for (int i = 0; i < 10; ++i) arr.Push(i);
		random.ShuffleArray(arr.Mem(), arr.Size());

		arr.Sort();
		GLASSERT(InOrder(arr.Mem(), arr.Size()));

		random.ShuffleArray(arr.Mem(), arr.Size());
		arr.Sort([](const int& a, const int& b) {
			return a < b;
		});
		GLASSERT(InOrder(arr.Mem(), arr.Size()));
	}

	// PacketQueue
	{
		PacketQueue pq;
		TestA testA = { 17 };
		TestAB testB = { 19, 42.0 };

		pq.Push(0, testA);
		pq.Push(1, testB);
		pq.Push(0, testA);

		pq.Pop(&testA);
		GLASSERT(testA.a == 17);
		pq.Push(0, testA);
		pq.Pop(&testB);
		GLASSERT(testB.a == 19 && testB.b == 42.0);
		pq.Pop(&testA);
		GLASSERT(testA.a == 17);
		pq.Pop(&testA);
		GLASSERT(testA.a == 17);
		GLASSERT(pq.Empty());
	}

	// PacketQueue stress
	{
		PacketQueue pq;
		TestA testA = { 17 };
		TestAB testB = { 19, 42.0 };

		static const int N = 1200;
		for (int i = 0; i < N; ++i) {
			pq.Push(0, testA);
			pq.Push(1, testB);
		}
		for (int i = 0; i < N/2; ++i) {
			int idA = pq.Pop(&testA);
			int idB = pq.Pop(&testB);
			GLASSERT(idA == 0);
			GLASSERT(idB == 1);
			GLASSERT(testA.a == 17);
			GLASSERT(testB.a == 19 && testB.b == 42.0);
		}
		for (int i = 0; i < N/2; ++i) {
			pq.Push(0, testA);
			pq.Push(1, testB);
		}
		for (int i = 0; i < N; ++i) {
			int idA = pq.Pop(&testA);
			int idB = pq.Pop(&testB);
			GLASSERT(idA == 0);
			GLASSERT(idB == 1);
			GLASSERT(testA.a == 17);
			GLASSERT(testB.a == 19 && testB.b == 42.0);
		}
		GLASSERT(pq.Empty());
	}

	// BitArray
	{
		BitArray<16> bit16;
		GLASSERT(!bit16.IsSet(0, 0));
		bit16.Set(5, 9);
		GLASSERT(!bit16.IsSet(9, 5));
		GLASSERT(!bit16.IsSet(4, 9));
		GLASSERT(!bit16.IsSet(9, 10));
		GLASSERT(bit16.IsSet(5, 9));
	}

	// PQueue, as a queue
	{
		PQueue<int> pqueue;
		pqueue.Push(3);
		pqueue.Push(1);
		pqueue.Push(5);
		pqueue.Push(2);
		pqueue.Push(4);
		pqueue.Push(6);
		pqueue.Push(7);
		GLASSERT(pqueue.IsHeap());

		GLASSERT(pqueue.Pop() == 7);
		GLASSERT(pqueue.Pop() == 6);
		GLASSERT(pqueue.Pop() == 5);
		GLASSERT(pqueue.Pop() == 4);
		GLASSERT(pqueue.Pop() == 3);
		GLASSERT(pqueue.Pop() == 2);
		GLASSERT(pqueue.Pop() == 1);
		GLASSERT(pqueue.Empty());
	}

	// PQueue, stress
	{
		PQueue<int> p;
		static const int N = 1000;
		int* input = new int[N];
		for (int i = 0; i < N; ++i) {
			input[i] = i;
		}
		Random r(123);
		r.ShuffleArray(input, N);
		for (int i = 0; i < N; ++i) {
			p.Push(input[i]);
		}
		delete[] input;
		for (int i = N - 1; i >= 0; i--)
			GLASSERT(p.Pop() == i);
	}

	// PQueue, as a set
	{
		struct KV {
			KV(int _id, int _priority) : id(_id), priority(_priority) {}
			int id;
			int priority;

			bool operator<(const KV& rhs) const { return priority < rhs.priority; }
			bool operator>(const KV& rhs) const { return priority > rhs.priority; }
			bool KeyEqual(const KV& rhs) const { return id == rhs.id; }
		};

		PQueue<KV> p;
		p.Set(KV(3, 6));
		p.Set(KV(4, 3));
		p.Set(KV(5, 4));
		p.Set(KV(1, 5));
		p.Set(KV(7, 2));
		p.Set(KV(2, 1));
		p.Set(KV(6, 7));
		GLASSERT(p.IsHeap());
		GLASSERT(p.Pop().id == 6);
		p.Set(KV(6, 7));
		for (int i = 0; i < 7; ++i) {
			p.Set(KV(i + 1, i + 1));
			GLASSERT(p.IsHeap());
		}
		for (int i = 7; i > 0; --i) {
			KV kv = p.Pop();
			GLASSERT(kv.id == i);
			GLASSERT(kv.priority == i);
		}
		GLASSERT(p.Empty());
	}
	{
		IntHashTable<int, int> hash;

		// Put in something to keep it busy.
		int count = 0;
		for (int x = -111; x < 100; x += 2) {
			hash[x] = ++count;
		}
		GLASSERT(!hash.Empty());

		count = 0;
		int nB = hash.NumBuckets();
		int n = hash.Size();

		count = 0;
		for (int x = -111; x < 100; x += 2) {
			int v = -1;
			(void)v;
			GLASSERT(hash.TryGet(x, &v) == true);
			GLASSERT(v == ++count);
			hash[x] = 0;
			GLASSERT(hash.TryGet(x, &v) == true);
			GLASSERT(v == 0);
			hash[x] = 1;
			GLASSERT(hash[x] == 1);

			hash.Remove(x);
			GLASSERT(hash.TryGet(x, &v) == false);
		}
		GLASSERT(hash.NumBuckets() == nB);
		GLASSERT(hash.NumDeleted() <= n);
		GLASSERT(hash.Empty());
	}
	{
		IntHashTable<uint64_t, uint32_t> hash;

		// Put in something to keep it busy.
		int count = 0;
		for (int x = 9; x < 500; x += 3) {
			hash[x] = ++count;
		}
		GLASSERT(!hash.Empty());

		count = 0;
		int nB = hash.NumBuckets();
		int n = hash.Size();

		count = 0;
		for (int x = 9; x < 500; x += 3) {
			uint32_t v = -1;
			(void)v;
			GLASSERT(hash.TryGet(x, &v) == true);
			GLASSERT(v == ++count);
			hash[x] = 0;
			GLASSERT(hash.TryGet(x, &v) == true);
			GLASSERT(v == 0);
			hash[x] = 1;
			GLASSERT(hash[x] == 1);

			hash.Remove(x);
			GLASSERT(hash.TryGet(x, &v) == false);
		}
		GLASSERT(hash.NumBuckets() == nB);
		GLASSERT(hash.NumDeleted() <= n);
		GLASSERT(hash.Empty());
	}
#if true
	{
		static const int N = 5000;
		static const int R = 4000;
		static const int E = 3000;

		// Simulate the entities.
		IntHashTable<int, int> hash;
		Random r(1);

		for (int i = 1; i < N; i++) {
			hash.Add(i, i);			
		}
		for (int i = 0; i < R; ++i) {
			int index = r.Rand(N);
			if (hash.TryGet(index, 0)) {
				hash.Remove(index);
			}
		}
		int nUsed, nDeleted, nUnused;
		hash.Analyze(&nDeleted, &nUnused, &nUsed);
		printf("Hash stress test(0): nItems=%d nBuckets=%d nDeleted=%d nUnused=%d nUsed=%d\n",
			hash.Size(), hash.NumBuckets(), nDeleted, nUnused, nUsed);
		GLASSERT(nDeleted == 2767);
		GLASSERT(nUnused == 3195);
		GLASSERT(nUsed == 2230);

		for (int i = 0; i < E; ++i) {
			hash.Add(N + i, N + i);
		}

		hash.Analyze(&nDeleted, &nUnused, &nUsed);
		printf("Hash stress test(1): nItems=%d nBuckets=%d nDeleted=%d nUnused=%d nUsed=%d\n",
			hash.Size(), hash.NumBuckets(), nDeleted, nUnused, nUsed);
		GLASSERT(nDeleted == 0);
		GLASSERT(nUnused == 2962);
		GLASSERT(nUsed == 5230);
	}
#endif
}
