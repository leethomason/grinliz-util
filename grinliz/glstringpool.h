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
	IString(const char*);
	IString(const IString& rhs) { str = rhs.str; }
	IString(IString&& rhs) noexcept { str = rhs.str; }
	IString& operator=(const IString& rhs) { str = rhs.str; return *this; }
	IString& operator=(IString&& rhs) noexcept { str = rhs.str; return *this; }

	~IString() = default;

	const char* c_str() const { return str; }
	intptr_t IntPtr() const { return (intptr_t)str; }

	bool operator==(const IString& rhs) const { return str == rhs.str; }
	bool operator==(const char* p) const { return strcmp(str, p) == 0; }
	bool operator!=(const IString& rhs) const { return !(rhs == *this); }
	bool operator!=(const char* p) const { return !(*this == p); }

	int Length() const {
		return *(str - 2) * 256 + *(str - 1);
	}

	bool Empty() const { return Length() == 0; }

private:
	const char* str;
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
	static IString Get(uint64_t hash);
	static bool Has(const char* str);

	static int TotalMem();
	static int NumStrings();

	static bool Test();

private:
	StringPool();
	~StringPool();

	static const char* InternInner(const char* str, uint64_t* hash);

	enum {
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
