#include "glstringpool.h"
#include "glstringutil.h"
#include "SpookyV2.h"
#include "glserialize.h"

using namespace grinliz;
constexpr uint64_t HASH_SEED = 135789;

CDynArray<StringPool::Block> StringPool::blocks;
std::mutex StringPool::poolMutex;
IntHashTable<uint64_t, char*> StringPool::stringHash;
IString StringPool::emptyStr;

IString::IString()
{
	str = StringPool::EmptyStr().c_str();
}

IString::IString(const char* c)
{
	str = StringPool::Intern(c).str;
}

void StringPool::Create() 
{
	emptyStr = Intern("");
}

void StringPool::Destroy()
{
	printf("StringPool: blocks=%d totalMem=%dk nStrings=%d\n", blocks.Size(), TotalMem() / 1024, NumStrings());
	for (int i = 0; i < blocks.Size(); ++i) {
		free(blocks[i].mem);
	}
	blocks.FreeMem();
	stringHash.Free();
}

int StringPool::TotalMem()
{
	std::scoped_lock<std::mutex> guard(poolMutex);
	int mem = 0;
	for (int i = 0; i < blocks.Size(); ++i) {
		mem += blocks[i].memUse;
	}
	return mem;
}


int StringPool::NumStrings()
{
	std::scoped_lock<std::mutex> guard(poolMutex);
	int n = 0;
	for (int i = 0; i < blocks.Size(); ++i) {
		n += blocks[i].nStrings;
	}
	return n;
}


IString StringPool::Intern(const char* str)
{
	std::scoped_lock<std::mutex> guard(poolMutex);
	const char* p = InternInner(str, 0);
	IString istr;
	istr.str = p;
	return istr;
}

const char* StringPool::InternInner(const char* str, uint64_t* hashOut)
{
	if (!str) return nullptr;

	size_t len = strlen(str);
	GLASSERT(len <= STRINGPOOL_MAX_SIZE - STRINGPOOL_OVERHEAD);
	uint64_t h = SpookyHash::Hash64(str, len, HASH_SEED);

	char* value = 0;
	if (stringHash.TryGet(h, &value)) {
		GLASSERT(StrEqual(value, str));	// if this fires there is a string hash collision!!
		if (hashOut) *hashOut = h;
		return value;
	}

	int blockIndex = -1;
	for (int i = 0; i < blocks.Size(); ++i) {
		if (STRINGPOOL_MAX_SIZE - STRINGPOOL_OVERHEAD - blocks[i].memUse >= (int64_t)len) {
			blockIndex = i;
			break;
		}
	}
	if (blockIndex < 0) {
		Block b;
		b.mem = (uint8_t*)malloc(STRINGPOOL_MAX_SIZE);
		blockIndex = blocks.Size();
		blocks.Push(b);
	}

	Block b = blocks[blockIndex];
	b.mem[b.memUse + HIGH_SIZE] = uint8_t(len >> 8);
	b.mem[b.memUse + LOW_SIZE] = uint8_t(len & 0xff);
	memcpy(b.mem + b.memUse + START, str, len + 1);

	char* result = ((char*)(b.mem + b.memUse + START));

	b.memUse += int(START + len + 1);
	b.nStrings++;
	GLASSERT(b.memUse <= STRINGPOOL_MAX_SIZE);
	GLASSERT(b.mem[b.memUse - 1] == 0);
	blocks[blockIndex] = b;

	stringHash[h] = result;

	if (hashOut) *hashOut = h;
	return result;
}


IString StringPool::Get(uint64_t h)
{
	std::scoped_lock<std::mutex> guard(poolMutex);
	char* value = 0;
	if (stringHash.TryGet(h, &value)) {
		return IString(value);
	}
	GLASSERT(false);	// string not found; system unstable; will crash.
	return IString(0);
}


bool StringPool::Has(const char* str)
{
	size_t len = strlen(str);
	uint64_t h = SpookyHash::Hash64(str, len, HASH_SEED);

	std::scoped_lock<std::mutex> guard(poolMutex);
	return stringHash.TryGet(h, 0);
}

bool StringPool::Test()
{
	{
		StringPool::Create();

		IString foo = StringPool::Intern("Foo");
		IString bar = StringPool::Intern("Bar");

		IString a = StringPool::Intern("Hello");
		IString b = StringPool::Intern("World");
		IString c = StringPool::Intern("Foo");
		IString d = StringPool::Intern("Bar");

		GLASSERT(a == "Hello");
		GLASSERT(b == "World");
		GLASSERT(c == foo);
		GLASSERT(c == "Foo");
		GLASSERT(d == bar);
		StringPool::Destroy();
	}
	return true;
}
