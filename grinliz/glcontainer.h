/*
Copyright (c) 2000-2019 Lee Thomason (www.grinninglizard.com)
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


#ifndef GRINLIZ_CONTAINER_INCLUDED
#define GRINLIZ_CONTAINER_INCLUDED

#include <new>
#include <cstring>
#include <vector>
#include <mutex>
#include <limits>

#include "gldebug.h"
#include "glutil.h"

namespace grinliz
{

void TestContainers();

// I'm completely intriqued by CombSort. QuickSort is
// fast, but an intricate algorithm that is easy to
// implement an "almost working" solution. CombSort
// is very simple, and almost as fast.
inline int CombSortGap( int gap ) 
{
	GLASSERT( (gap & 0xff000000) == 0 );
	// shrink: 1.247
	// 9 or 10 go to 11 (awesome)
	//							   0  1  2  3  4  5  6  7  8  9 10 11  12  13  14  15
	static const int table[16] = { 1, 1, 1, 2, 3, 4, 4, 5, 6, 7, 8, 8, 11, 11, 11, 12 };
	if ( gap < 16 ) {
		gap = table[gap];
	}
	else {
		gap = (gap * 103)>>7;
	}
	GLASSERT( gap  > 0 );
	return gap;
}

// Binary Search: array must be sorted!
template<typename T, typename LessFunc>
int BSearch(const T& t, const T* mem, const T* end, LessFunc func) {
    int low = 0;
    int high = end - mem;
    const int size = end - mem;

    while (low < high) {
        int mid = low + (high - low) / 2;
        if(func(mem[mid], t))
            low = mid + 1;
        else
            high = mid;
    }
    if ((low < size) && EqualFromLessThan(mem[low], t, func)) {
        // if multiple matches, return the first.
        while (low && EqualFromLessThan(mem[low-1], t, func)) {
            --low;
        }
        return low;
    }
    return -1;
}

template<typename T>
int BSearch(const T& t, const T* mem, const T* end) {
    return grinliz::BSearch(t, mem, end, [](const T& a, const T& b) { return a < b; });
}

/*
	::Sort(T* mem, int size);						uses operator <
	::Sort(T* mem, int size, LessFunc func );		uses lambda function
*/

template <class T, typename LessFunc >
inline void Sort( T* mem, int size, LessFunc lessfunc  )
{
	int gap = size;
	for (;;) {
		gap = CombSortGap(gap);
		bool swapped = false;
		const int end = size - gap;
		for (int i = 0; i < end; i++) {
			int j = i + gap;
			if ( lessfunc(mem[j],mem[i]) ) {
				Swap(mem[i], mem[j]);
				swapped = true;
			}
		}
		if (gap == 1 && !swapped) {
			break;
		}
	}
}


template <typename T>
inline void Sort(T* mem, int size) {
	Sort(mem, size, [](const T&a, const T&b) {
		return a < b;
	});
}

template <typename T, typename Func >
inline int ArrayFind(const T* mem, int size, Func func)
{
	for (int i = 0; i < size; ++i) {
		if (func(mem[i])) {
			return i;
		}
	}
	return -1;
}

template <typename T, typename Func >
inline int ArrayFindMax(const T* mem, int size, Func func)
{
	int r = 0;
	auto best = func(mem[0]);

	for( int i=1; i<size; ++i ) {
		auto score = func(mem[i]);
		if (score > best) {
			best = score;
			r = i;
		}
	}
	return r;
}

/*	A dynamic array for blittable data. (No constructor / destructor / virtual.)
*/
template <class T>
class CDynArray
{
    enum { CACHE = 4 };
public:
    typedef T ElementType;

    CDynArray() : size(0), capacity(CACHE) {
        mem = reinterpret_cast<T*>(cache);
        GLASSERT(CACHE_SIZE * sizeof(int) >= CACHE * sizeof(T));
    }

	CDynArray(const CDynArray<T>& rhs) {
		mem = reinterpret_cast<T*>(cache);
		for (int i = 0; i < rhs.Size(); ++i)
			Push(rhs[i]);
	}

    ~CDynArray() {
        Clear();
		FreeMemPtr();
    }

	void operator=(const CDynArray<T>& rhs) {
		Clear();
		for (int i = 0; i < rhs.Size(); ++i)
			Push(rhs[i]);
	}

    T* begin() { return mem; }
    const T* begin() const { return mem; }
    T* end() { return mem + size; }
    const T* end() const { return mem + size; }

    T& operator[](int i) { GLASSERT(i >= 0 && i < (int)size); return mem[i]; }
    const T& operator[](int i) const { GLASSERT(i >= 0 && i < (int)size); return mem[i]; }
    const T& at(int i) const { GLASSERT(i >= 0 && i < (int)size); return mem[i]; }

    const T& First() const { GLASSERT(size); return mem[0]; }
    const T& Last() const { GLASSERT(size); return mem[size - 1]; }

    void Push() {
        PushArr(1);
    }

    void Push(const T& t) {
        EnsureCap(size + 1);
		mem[size] = t;
        ++size;
    }

    void PushFront(const T& t) {
        EnsureCap(size + 1);
        for (int i = size; i > 0; --i) {
            mem[i] = mem[i - 1];
        }
        mem[0] = t;
        ++size;
    }

    T* PushArr(int count) {
        EnsureCap(size + count);
		int oldSize = size;
        size += count;
		return mem + oldSize;
    }

    T Pop() {
        GLASSERT(size > 0);
        --size;
        T temp = mem[size];
        return temp;
    }

    T PopFront() {
        GLASSERT(size > 0);
        T temp = mem[0];
        Remove(0);
        return temp;
    }

    void Remove(int i) {
        GLASSERT(i < (int)size);
        // Copy down.
        for (int j = i; j < size - 1; ++j) {
            mem[j] = mem[j + 1];
        }
        // Get rid of the end:
        Pop();
    }

    void SwapRemove(int i) {
        if (i < 0) {
            GLASSERT(i == -1);
            return;
        }
        GLASSERT(i < (int)size);
        GLASSERT(size > 0);

        mem[i] = mem[size - 1];
        Pop();
    }

    void Reverse() {
        for (int i = 0; i < size / 2; ++i) {
            Swap(&mem[i], &mem[size - 1 - i]);
        }
    }

    int Find(const T& t) const {
        for (int i = 0; i < size; ++i) {
            if (mem[i] == t)
                return i;
        }
        return -1;
    }

    template<typename Context, typename Func>
    int Find(Context context, Func func) {
        return ArrayFind(mem, size, context, func);
    }

    template<typename Context, typename Func>
    int FindMax(Context context, Func func) {
        return ArrayFindMax(mem, size, context, func);
    }

    int Size() const { return size; }

	void SetSize(int n, const T& def) {
		Clear();
		for (int i = 0; i < n; ++i)
			Push(def);
	}

    void Clear() {
		size = 0;
    }
	void FreeMem() {
		Clear();
		FreeMemPtr();
		mem = reinterpret_cast<T*>(cache);
		size = 0;
		capacity = 0;
	}

    bool Empty() const { return size == 0; }
    const T* Mem() const { return mem; }
    T* Mem() { return mem; }
    const T* End() const { return mem + size; }	// mem never 0, because of cache
    const T& Front() const { GLASSERT(size);  return mem[0]; }
    const T& Back() const { GLASSERT(size);  return mem[size - 1]; }

    void Reserve(int n) { EnsureCap(n); }

    void EnsureCap(int count) {
        if (count > capacity) {
            capacity = Max(CeilPowerOf2(count), (uint32_t)CACHE_SIZE * 4);
			size_t s = capacity * sizeof(T);
			
			if (mem == reinterpret_cast<T*>(cache)) {
				mem = (T*) malloc(s);
                memcpy(mem, cache, size * sizeof(T));
            }
            else {
				mem = (T*)realloc(mem, s);
            }
        }
    }

    void Sort() {
        grinliz::Sort(mem, size);
    }

    template<typename LessFunc>
    void Sort(LessFunc func) { 
        grinliz::Sort<T, LessFunc>(mem, size, func); 
    }

    // Binary Search: array must be sorted!
    int BSearch(const T& t) const {
        return grinliz::BSearch<T>(t, mem, mem + Size());
    }

    template<typename LessFunc>
    int BSearch(const T& t, LessFunc func) const {
        return grinliz::BSearch<T, LessFunc>(t, mem, mem + Size(), func);
    }

    template<typename Context, typename Func>
    void Filter(Context context, Func func) {
        for (int i = 0; i < size; ++i) {
            if (!func(context, mem[i])) {
                SwapRemove(i);
                --i;
            }
        }
    }

protected:
	void FreeMemPtr() {
		if (mem != reinterpret_cast<T*>(cache)) {
			free(mem);
		}
		mem = 0;
	}

    T* mem;
    int size;
    int capacity;
    enum {
        CACHE_SIZE = (CACHE * sizeof(T) + sizeof(int) - 1) / sizeof(int)
    };
    int cache[CACHE_SIZE];
};

template<typename T, typename Func>
inline int Filter(int n, T* arr, Func keep) 
{
	int i = n - 1;
	while (i >= 0 && n) {
		if (keep(arr[i])) {
			--i;
		}
		else {
			Swap(arr[i], arr[n-1]);
			--n;
			if (i == n) i = n - 1;
		}
	}
	return n;
}

/*
	Sorted Array
	Does support repeated keys. (Unlike hash table.)
*/
template <class T>
class SortedDynArray : public CDynArray<T>
{
public:
    void Add(const T& t) {
        this->EnsureCap(this->size + 1);

        int i = this->size;
        while ((i > 0) && (t < this->mem[i - 1])) {
            this->mem[i] = this->mem[i - 1];
            --i;
        }
        this->mem[i] = t;
        ++(this->size);
    }
};


/* A fixed array class for any type.
   Supports copy construction, proper destruction, etc.
   Does keep the objects around, until entire CArray is destroyed,

 */
template < class T, int CAPACITY>
class CArray
{
public:
	typedef T ElementType;

	// construction
	CArray() : size(0)	{}
	~CArray()				{}

	// operations
	T& operator[]( int i )				{ GLASSERT( i>=0 && i<(int)size ); return mem[i]; }
	const T& operator[]( int i ) const	{ GLASSERT( i>=0 && i<(int)size ); return mem[i]; }

	T* begin() { return mem; }
	const T* begin() const { return mem; }
	T* end() { return mem + size; }
	const T* end() const { return mem + size; }

	const T& Front() const { return (*this)[0]; }
	const T& Back() const { return (*this)[this->Size() - 1]; }

	// Push on
	void Push( const T& t ) {
		GLASSERT( size < CAPACITY );
		if (size < CAPACITY) 
			mem[size++] = t;
	}

	bool PushIfCap(const T& t) {
		if (size < CAPACITY) {
			mem[size++] = t;
			return true;
		}
		return false;
	}

	void Insert(int index, const T& t) {
		GLASSERT(size <= CAPACITY);
		if (size == CAPACITY) --size;
		for (int i = size; i > index; --i) {
			mem[i] = mem[i - 1];
		}
		mem[index] = t;
		++size;
	}

	// Returns space to uninitialized objects.
	T* PushArr( int n ) {
		GLASSERT( size+n <= CAPACITY );
		T* rst = &mem[size];
		size += n;
		return rst;
	}

	T Pop() {
		GLASSERT( size > 0 );
		return mem[--size];
	}

	T PopFront() {
		GLASSERT( size > 0 );
		T temp = mem[0];
		for( int i=0; i<size-1; ++i ) {
			mem[i] = mem[i+1];
		}
		--size;
		return temp;
	}

	void Crop(int maxSize) {
		GLASSERT(maxSize >= 0);
		if (size > maxSize) {
			size = maxSize;
		}
	}

	int Find( const T& key ) const { 
		for( int i=0; i<size; ++i ) {
			if ( mem[i] == key )
				return i;
		}
		return -1;
	}

	template<typename Context, typename Func>
	int Find(Context context, Func func) {
		return ArrayFind(mem, size, context, func);
	}

	template<typename Context, typename Func>
	int FindMax(Context context, Func func) {
		return ArrayFindMax(mem, size, context, func);
	}

	int Size() const		{ return size; }
	int Capacity() const	{ return CAPACITY; }
	bool HasCap() const		{ return size < CAPACITY; }
	
	void Clear()	{ 
		size = 0; 
	}
	bool Empty() const		{ return size==0; }
	T*		 Mem() 			{ return mem; }
	const T* Mem() const	{ return mem; }

	const T* End() 			{ return mem + size; }
	const T* End() const	{ return mem + size; }

	void SwapRemove( int i ) {
		GLASSERT( i >= 0 && i < (int)size );
		mem[i] = mem[size-1];
		Pop();
	}

	void Reverse() {
		for (int i = 0; i < size / 2; ++i) {
			Swap(&mem[i], &mem[size - 1 - i]);
		}
	}

	template<typename Context, typename Func>
	void Filter(Context context, Func func) {
		for (int i = 0; i < size; ++i) {
			if (!func(context, mem[i])) {
				SwapRemove(i);
				--i;
			}
		}
	}

	void Sort() { grinliz::Sort<T>(mem, size); }

	// LessFunc returns a < b
	template<typename LessFunc>
	void Sort(LessFunc func) { grinliz::Sort<T, LessFunc>(mem, size, func); }

private:
	T mem[CAPACITY];
	int size;
};


class CompCharPtr {
public:
	template <class T>
	static uint32_t Hash( T& _p) {
		const unsigned char* p = (const unsigned char*)_p;
        uint32_t hash = 2166136261UL;
		for( ; *p; ++p ) {
			hash ^= *p;
			hash *= 16777619;
		}
		return hash;
	}
	template <class T>
	static bool Equal( T& v0, T& v1 ) { return strcmp( v0, v1 ) == 0; }
};

// Simple HashTable for blittable data. (No constructor / destructor / virtual.)
template <class K, class V>
class IntHashTable
{
public:
	IntHashTable() {}
	~IntHashTable() { Clear(); }

	// Adds a key/value pair. What about duplicates? Duplicate
	// keys aren't allowed. An old value will be deleted and replaced.
	V& Add(K key, const V& value)
	{
		GLASSERT(key != UNUSED && key != DELETED);
		if (!reallocating) {
			EnsureCap();
		}

		// Existing value?
		int index = FindIndex(key);
		if (index >= 0) {
			// Replace!
			buckets[index].value = value;
			return buckets[index].value;
		}
		else {
			uint32_t hash = Hash(key);
			while (true) {
				hash = hash & (nBuckets - 1);
				K state = buckets[hash].key;
				if (state == UNUSED || state == DELETED) {
					if (state == DELETED)
						--nDeleted;
					++nItems;

					buckets[hash].key = key;
					buckets[hash].value = value;
					return buckets[hash].value;
				}
				++hash;
			}
		}
	}

	V Remove(K key) {
		int index = FindIndex(key);
		GLASSERT(index >= 0);
		// It's always okay to be DELETED; but if the next bucket is
		// trivially UNUSED, this can be UNUSED too, since we'll
		// never need to move to the next bucket.
		if (index + 1 < nBuckets && buckets[index + 1].key == UNUSED) {
			buckets[index].key = UNUSED;
		}
		else {
			buckets[index].key = DELETED;
			++nDeleted;
		}
		--nItems;
		return buckets[index].value;
	}

	void Clear() {
		free(buckets);
		buckets = 0;
		nBuckets = 0;
		nItems = 0;
		nDeleted = 0;
		nBits = 0;
	}


	bool TryGet(K key, V* value) const {
		int index = FindIndex(key);
		if (index >= 0) {
			if (value) *value = buckets[index].value;
			return true;
		}
		return false;
	}

	V Get(K key) const {
		int index = FindIndex(key);
		if (index >= 0) {
			return buckets[index].value;
		}
		GLASSERT(false);
		return V();
	}

	V& operator[] (K key) {
		int index = FindIndex(key);
		if (index >= 0)
			return buckets[index].value;
		V v;
		return Add(key, v);
	}

	const V& operator[] (K key) const {
		int index = FindIndex(key);
		GLASSERT(index >= 0);
		return buckets[index].value;
	}

	bool Empty() const { return nItems == 0; }
	int Size() const { return nItems; }
	
	int NumBuckets() const { return nBuckets; } // testing
	int NumDeleted() const { return nDeleted; }	// testing
	void Analyze(int* nDeleted, int* nUnused, int* nUsed) const 
	{
		*nDeleted = 0;
		*nUnused = 0;
		*nUsed = 0;
		for (int i = 0; i < nBuckets; ++i) {
			if (buckets[i].key == UNUSED) *nUnused += 1;
			else if (buckets[i].key == DELETED) *nDeleted += 1;
			else *nUsed += 1;
		}
	}

	struct Iterator {
		friend class IntHashTable<K, V>;
	public:
		K Key() const { return table->buckets[index].key; }
		V Value() const { return table->buckets[index].value; }

		void Next() {
			++index;
			while (index < table->nBuckets && UnusedOrDeleted(table->buckets[index].key))
				index++;
		}
		bool Done() const {
			return !table || index == table->nBuckets;
		}

	private:
		const IntHashTable<K, V>* table = 0;
		int index = 0;
	};

	Iterator GetIterator() const {
		Iterator it;
		if (!Empty()) {
			it.table = this;
			it.index = -1;
			it.Next();
		}
		else {
			it.table = 0;
			it.index = 0;
		}
		return it;
	}

private:
	IntHashTable(IntHashTable&);
	void operator=(const IntHashTable&);

	// Since all the keys are integers, this may
	// not really be a hash so much as spreading
	// out the values.
	uint32_t Hash(K k) const {
		GLASSERT(nBits);
		GLASSERT(nBits < 32);	// WAT. Not made to be that big. Need to test for 32bit+ sizes.

		const uint32_t mask = (1 << nBits) - 1;
		uint32_t hash = 0;
		int b = sizeof(K) * 8;
		
		while (b > 0) {
			hash ^= k & mask;
			k >>= nBits;
			b -= nBits;
		}
		GLASSERT(hash < (uint32_t)nBuckets);
		return hash;
	}

	void EnsureCap() {
		static const int MIN_BUCKETS = 64;
		if (!nBuckets) {
			GLASSERT(buckets == 0);
			nItems = nDeleted = 0;
			nBuckets = MIN_BUCKETS;
			nBits = LogBase2(nBuckets);
			buckets = (Bucket*)malloc(sizeof(Bucket) * nBuckets);
			memset(buckets, 0, sizeof(Bucket) * nBuckets);
			for (int i = 0; i < nBuckets; ++i)
				buckets[i].key = UNUSED;
		}
		else if ((nItems + nDeleted) >= nBuckets * 2 / 3) {
			GLASSERT(!reallocating);
			reallocating = true;

			Bucket* oldBuckets = buckets;
			int oldNBuckets = nBuckets;

			int n = nBuckets;
			// If mostly 'deleted', don't expand, just re-distribute.
			if (nDeleted < nItems) {
				n = CeilPowerOf2(Max((nItems + nDeleted) * 2, MIN_BUCKETS));
				GLASSERT(n > nBuckets);
			}
			nBuckets = n;
			nBits = LogBase2(nBuckets);
			nItems = 0;
			nDeleted = 0;
			buckets = (Bucket*)malloc(sizeof(Bucket) * nBuckets);
			memset(buckets, 0, sizeof(Bucket) * nBuckets);
			for (int i = 0; i < nBuckets; ++i)
				buckets[i].key = UNUSED;

			for (int i = 0; i < oldNBuckets; ++i) {
				if (!UnusedOrDeleted(oldBuckets[i].key))
					Add(oldBuckets[i].key, oldBuckets[i].value);
			}
			free(oldBuckets);
			reallocating = false;
		}
	}

	int FindIndex(K key) const
	{
		if (Empty()) return -1;

		uint32_t hash = Hash(key);
		while (true) {
			hash = hash & (nBuckets - 1);
			if (!UnusedOrDeleted(buckets[hash].key)) {
				if (buckets[hash].key == key) {
					return hash;
				}
			}
			else if (buckets[hash].key == UNUSED) {
				return -1;
			}
			++hash;
		}
		return -1;
	}

	int nBuckets = 0;
	int nItems = 0;
	int nDeleted = 0;
	int nBits = 0;
	bool reallocating = false;

	static constexpr K UNUSED = (std::numeric_limits<K>::max)() - 1;
	static constexpr K DELETED = (std::numeric_limits<K>::max)() - 0;
	static bool UnusedOrDeleted(K k) { return k == UNUSED || k == DELETED; }

	struct Bucket
	{
		Bucket() : key(UNUSED) {}
		K	key;
		V	value;
	};
	Bucket* buckets = 0;
};


// A simple class that accumulates memory to store stuff.
// Usefu for the packet classes and such. Memory is 
// contiguous and only the base pointer is aligned.
class DynMemBuf
{
public:
	DynMemBuf() {}
	~DynMemBuf() { free(mem); }

	void Add(const void* src, size_t nBytes) {
		EnsureCap(Size() + nBytes);
		memcpy(end, src, nBytes);
		end += nBytes;

		GLASSERT(front <= cap);
		GLASSERT(front <= end);
		GLASSERT(front >= mem);
	}

	void DeleteFront(size_t nBytes) {
		GLASSERT(nBytes <= size_t(end - front));
		front += nBytes;
		if (front == end) {
			front = mem;
			end = mem;
		}
		GLASSERT(front <= cap);
		GLASSERT(front <= end);
		GLASSERT(front >= mem);
	}

	// This is just a copy with convenient syntax
	template<typename T>
	void Add(const T& t) {
		Add(&t, sizeof(t));
	}

	template<typename T>
	void Get(T* t) {
		GLASSERT((end - front) >= sizeof(T));
		memcpy(t, front, sizeof(T));
		DeleteFront(sizeof(T));
	}

	// Move all the data from `this` and append to `target`
	// 'this' will be empty after move.
	void Move(DynMemBuf& target);

	const void* Mem() const { return front; }
	void* Mem() { return front; }

	size_t Size() const { return end - front; }
	bool Empty() const { return front == end; }
	void Clear() { front = mem; end = mem; }

private:
	void EnsureCap(size_t s);
	uint8_t* mem = 0;		// origin of memory, returned by malloc/realloc
	uint8_t* front = 0;		// where the beginning of data is. DeleteFront may move this past 'mem'
	uint8_t* end = 0;		// end of memory
	uint8_t* cap = 0;		// capacity of memory
};


// FIFO Queue
// Works on raw memory, not types.
// NOT thread safe. See PacketQueueMT for that.
class PacketQueue
{
public:
	PacketQueue() {}
	~PacketQueue() {}

	template<class T>
	void Push(int id, const T& data) {
		Push(id, sizeof(T), &data);
	}
	void Push(int id, int dataSize, const void* data);

	template<class T>
	int Pop(T* t) {
		return Pop(t, sizeof(T));
	}
	int Pop(DynMemBuf* data);

	int Peek() const;
	bool Empty() const { return memBuf.Empty(); }

	// Move 'this' to 'queue'.
	// 'this' will be empty after move.
	void Move(PacketQueue& queue);

	size_t Size() const {
		return memBuf.Size();
	}

private:
	int Pop(void* target, int size);

	struct Header {
		int id;
		int dataSize;
	};
	DynMemBuf memBuf;
};

template<int SIZE>
class BitArray
{
public:
	BitArray() {
		Clear();
	}

	void Clear() {
		memset(bits, 0, sizeof(uint32_t) * ALLOC);
	}

	int IsSet(int x, int y) const {
		int dx = x >> 5;
		int dbit = x & 31;
		GLASSERT(y * STRIDE + dx < ALLOC);
		return (bits[y * STRIDE + dx] & (1 << dbit)) ? 1 : 0;
	}

	void Set(int x, int y) {
		int dx = x >> 5;
		int dbit = x & 31;
		GLASSERT(y * STRIDE + dx < ALLOC);
		bits[y * STRIDE + dx] |= (1 << dbit);
	}

	void Clear(int x, int y) {
		int dx = x >> 5;
		int dbit = x & 31;
		GLASSERT(y * STRIDE + dx < ALLOC);
		bits[y * STRIDE + dx] &= (~(1 << dbit));
	}

	void Set(int x, int y, int v) {
		if (v) Set(x, y);
		else Clear(x, y);
	}

private:
	static const int STRIDE = (SIZE + 31) / 32;
	static const int ALLOC = STRIDE * SIZE;
	uint32_t bits[ALLOC];
};

template<typename T>
inline void Reverse(T* mem, int n) {
	for (int i = 0; i < n / 2; ++i) {
		Swap(mem[i], mem[n - 1 - i]);
	}
}

// A simple Priority Queue, based on Sedgewick's Algorithms in C++
// (Which is a very elegant, simple heap.)
// Requires a type with operator < and operator >
// If Set() is used, requires a KeyEquals()
// Operates on blittable data. (No constructor / destructor / virtual.)
template<typename T>
class PQueue
{
public:
	// Pops the highest priority item.
	T Pop() {
		T v = arr[0];
		if (arr.Size() > 1) {
			arr[0] = arr.Pop();
			DownHeap(0);
		}
		else {
			arr.Pop();
		}
		return v;
	}

	// Pushes a new item. (Duplicates aren't checked for.)
	void Push(const T& t) {
		arr.Push(t);
		UpHeap(arr.Size() - 1);
	}

	// Pushes or Sets an item; will be unique based on KeyEquals.
	void Set(const T& t) {
		// Need to remove an existing item. Always awkward.
		// Puts a linear walk in this, which is slow.
		// But fixing it is more memory.
		for (int i = 0; i < arr.Size(); ++i) {
			if (arr[i].KeyEqual(t)) {
				if (t < arr[i]) {
					arr[i] = t;
					DownHeap(i);
				}
				else if (t > arr[i]) {
					arr[i] = t;
					UpHeap(i);
				}
				return;
			}
		}
		Push(t);
	}

	bool Empty() const { return arr.Empty(); }

	bool IsHeap() const {
		for (int i = 0; i < arr.Size(); ++i) {
			if (Left(i) < arr.Size()) {
				if (arr[Left(i)] > arr[i])
					return false;
			}
			if (Right(i) < arr.Size()) {
				if (arr[Right(i)] > arr[i])
					return false;
			}
		}
		return true;
	}

	void Clear() { arr.Clear(); }

private:
	void UpHeap(int k) {
		T v = arr[k];
		while (k > 0 && arr[Parent(k)] < v) {
			arr[k] = arr[Parent(k)];
			k = Parent(k);
		}
		arr[k] = v;
	}

	void DownHeap(int k) {
		T v = arr[k];
		while (Left(k) < arr.Size()) {
			int j = Left(k);
			if (Right(k) < arr.Size() && arr[j] < arr[j + 1])
				j++;
			if (v > arr[j])
				break;
			arr[k] = arr[j];
			k = j;
		}
		arr[k] = v;
	}

	int Left(int i) const { return i * 2 + 1; }
	int Right(int i) const { return i * 2 + 2; }
	int Parent(int i) const { return (i-1) / 2; }

	CDynArray<T> arr;
};

struct AStarNode
{
	int index;
	float cost;

	bool operator<(const AStarNode& rhs) const { return cost > rhs.cost; }
	bool operator>(const AStarNode& rhs) const { return cost < rhs.cost; }
	bool KeyEqual(const AStarNode& rhs) const { return index == rhs.index; }
};

}	// namespace grinliz
#endif
