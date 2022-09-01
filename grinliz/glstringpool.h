#pragma once

#include <stdio.h>
#include <mutex>

#include "glcontainer.h"

namespace grinliz
{
class Writer;
class Reader;
class StringPool;
class CompString;

static constexpr int64_t STRINGPOOL_MAX_SIZE = (1 << 16) - 256;	// 64k - someRoomForOverhead
static constexpr int64_t STRINGPOOL_OVERHEAD = 4;	// header + null terminator

// Wrapper around a `const char*` that is guaranteed
// to be immutable in memory. Can not be in a component,
// although it is blittable.
class IString
{
	friend class StringPool;

public:
	IString();

	const char* c_str() const { return str; }
	intptr_t IntPtr() const { return (intptr_t)str; }

	bool Equals(const IString& istr) const { return str == istr.str; }
	bool Equals(const char* p) const;

	bool operator==(const IString& rhs) const { return str == rhs.str; }
	bool operator==(const char* p) const { return this->Equals(p); }

	int Length() const {
		return *(str - 2) * 256 + *(str - 1);
	}

	bool Empty() const { return Length() == 0; }

private:
	IString(const char* p) : str(p) {}

	const char* str;
};

// Packed into components.
class CompString
{
	friend class StringPool;
	friend class IString;

public:
	CompString() : hash(0) {}

	bool IsSet() const { return hash != 0; }
	bool operator==(const CompString& rhs) const { return hash == rhs.hash; }
	bool operator==(const char* p) const;

private:
	CompString(uint64_t h) : hash(h) {}

	uint64_t hash;
};


class StringPool
{
public:
	static void Create();
	static void Destroy();

	static IString EmptyStr() {
		return emptyStr;
	}

	// Multi-thread safe, but not fast.
	static IString Intern(const char* str);
	static IString Intern(const CompString& str);
	static CompString InternC(const char* str);
	static CompString InternC(const IString& str);
	static const char* c_str(const CompString& str);
	static IString Get(uint64_t hash);
	static bool Has(const char* str);
	// --- end multi-thread safe ---

	// Saving & Loading
	// On save, everything a given registry (or client of the string pool) uses
	// needs to be marked. This guarantees all those strings will be saved.
	// Load just adds strings to the pool. Multiple registrys (or anything) can
	// share the pool as long as they are careful to mark & save the strings they own.

	// Multi-thread write only (can't be used in combination with MT methods above)
	static void Mark(uint64_t h);
	static void ClearMarks();
	static void SaveMarked(grinliz::Writer* writer);

	// Really append; adds strings to the pool.
	static void Load(grinliz::Reader* writer);

	static int TotalMem();
	static int NumStrings();

	static bool Test();

private:
	StringPool();
	~StringPool();

	static const char* InternInner(const char* str, uint64_t* hash);

	enum {
		MARK,
		HIGH_SIZE,
		LOW_SIZE,
		START
	};

	struct Block
	{
		int memUse = 0;
		int nStrings = 0;
		uint8_t* mem = 0;
	};
	static IString emptyStr;
	static CDynArray<Block> blocks;
	static std::mutex poolMutex;
	static IntHashTable<uint64_t, char*> stringHash;
};

}
