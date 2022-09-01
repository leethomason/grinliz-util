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

bool IString::Equals(const char* p) const 
{
	GLASSERT(str);
	return grinliz::StrEqual(p, str);
}

bool CompString::operator==(const char* p) const
{
	size_t len = strlen(p);
	uint64_t h = SpookyHash::Hash64(p, len, HASH_SEED);
	return hash == h;
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
	stringHash.Clear();
}

int StringPool::TotalMem()
{
	std::lock_guard<std::mutex> guard(poolMutex);
	int mem = 0;
	for (int i = 0; i < blocks.Size(); ++i) {
		mem += blocks[i].memUse;
	}
	return mem;
}


int StringPool::NumStrings()
{
	std::lock_guard<std::mutex> guard(poolMutex);
	int n = 0;
	for (int i = 0; i < blocks.Size(); ++i) {
		n += blocks[i].nStrings;
	}
	return n;
}


IString StringPool::Intern(const char* str)
{
	std::lock_guard<std::mutex> guard(poolMutex);
	const char* p = InternInner(str, 0);
	return IString(p);
}


IString StringPool::Intern(const CompString& str)
{
	std::lock_guard<std::mutex> guard(poolMutex);
	GLASSERT(str.hash);
	char* value = 0;
	if (stringHash.TryGet(str.hash, &value)) {
		return IString(value);
	}
	GLASSERT(false);
	return IString(0);
}


const char* StringPool::c_str(const CompString& str)
{
	std::lock_guard<std::mutex> guard(poolMutex);
	GLASSERT(str.hash);
	char* value = 0;
	if (stringHash.TryGet(str.hash, &value)) {
		return value;
	}
	GLASSERT(false);
	return nullptr;
}


CompString StringPool::InternC(const char* str)
{
	std::lock_guard<std::mutex> guard(poolMutex);
	uint64_t h = 0;
	InternInner(str, &h);
	return CompString(h);
}


CompString StringPool::InternC(const IString& str)
{
	std::lock_guard<std::mutex> guard(poolMutex);
	uint64_t h = SpookyHash::Hash64(str.str, str.Length(), HASH_SEED);
	char* value = 0;

	if (stringHash.TryGet(h, &value)) {
		return CompString(h);
	}
	GLASSERT(false);
	InternInner(str.str, &h);
	return CompString(h);
}


const char* StringPool::InternInner(const char* str, uint64_t* hashOut)
{
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
	b.mem[b.memUse + MARK] = 0;
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
	std::lock_guard<std::mutex> guard(poolMutex);
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

	std::lock_guard<std::mutex> guard(poolMutex);
	return stringHash.TryGet(h, 0);
}


void StringPool::ClearMarks()
{
	std::lock_guard<std::mutex> guard(poolMutex);
	for (int i = 0; i < blocks.Size(); ++i) {
		uint8_t* ptr = blocks[i].mem;
		for (int j = 0; j < blocks[i].nStrings; ++j) {
			*ptr = 0;
			uint32_t len = ptr[HIGH_SIZE] * 256 + ptr[LOW_SIZE];
			ptr += (uint64_t)len + 1 + START;
		}
	}
}


void StringPool::Mark(uint64_t h)
{
	char* ptr = 0;
	if (stringHash.TryGet(h, &ptr)) {
		*(ptr - START) = 1;
	}
}


void StringPool::SaveMarked(grinliz::Writer* writer)
{
	for (int i = 0; i < blocks.Size(); ++i) {
		uint8_t* ptr = blocks[i].mem;
		for (int j = 0; j < blocks[i].nStrings; ++j) {
			uint32_t len = ptr[HIGH_SIZE] * 256 + ptr[LOW_SIZE];
			if (ptr[MARK]) {
				writer->Write(ptr + 1, len + 2);
			}
			ptr += (uint64_t)len + 1 + START;
		}
	}
	writer->Write<uint8_t>(255);
	writer->Write<uint8_t>(255);
}


void StringPool::Load(grinliz::Reader* reader)
{
	DynMemBuf mem;

	while (true) {
		int high = reader->Read<uint8_t>();
		int low  = reader->Read<uint8_t>();
		int len = high * 256 + low;

		if (len == 65535)
			return;

		mem.SetSize(len + 1);
		reader->Read(mem.Mem(), len);
		char* p = (char*)mem.Mem();
		*(p + len) = 0;
		Intern(p);
	}
}



bool StringPool::Test()
{
	struct Comp {
		CompString compString;
	};

	{
		StringPool::Create();
		Comp comps[4];
		comps[0].compString = StringPool::InternC("Hello");
		comps[1].compString = StringPool::InternC("World");

		IString foo = StringPool::Intern("Foo");
		IString bar = StringPool::Intern("Bar");
		comps[2].compString = StringPool::InternC(foo);
		comps[3].compString = StringPool::InternC(bar);

		IString a = StringPool::Intern(comps[0].compString);
		IString b = StringPool::Intern(comps[1].compString);
		IString c = StringPool::Intern(comps[2].compString);
		IString d = StringPool::Intern(comps[3].compString);

		GLASSERT(a == "Hello");
		GLASSERT(b == "World");
		GLASSERT(c == foo);
		GLASSERT(c == "Foo");
		GLASSERT(d == bar);
		StringPool::Destroy();
	}
#ifdef DEBUG
	{
		StringPool::Create();
		constexpr int N = 16000;
		char buf[64];

		for (int i = 0; i < N; ++i) {
			grinliz::SNPrintf(buf, 64, "This is a test. %d", i);
			StringPool::Intern(buf);
		}

		for (int i = 0; i < N; ++i) {
			grinliz::SNPrintf(buf, 64, "This is a test. %d", i);
			GLASSERT(StringPool::Has(buf));
		}

		StringPool::ClearMarks();
		for (int i = 0; i < N; i += 2) {
			grinliz::SNPrintf(buf, 64, "This is a test. %d", i);
			uint64_t h = SpookyHash::Hash64(buf, strlen(buf), HASH_SEED);
			StringPool::Mark(h);
		}
		for (int i = 0; i < 200; ++i) {
			StringPool::Mark(i);
		}
		FILE* fp = 0;
		fopen_s(&fp, "testpool.sav", "wb");
		GLASSERT(fp);

		if (fp) {
			FileWriter writer(fp);
			StringPool::SaveMarked(&writer);
			fclose(fp);
		}
		StringPool::Destroy();
		StringPool::Create();

		fopen_s(&fp, "testpool.sav", "rb");
		GLASSERT(fp);
		if (fp) {
			FileReader reader(fp);
			StringPool::Load(&reader);
			fclose(fp);
		}
		for (int i = 0; i < N; ++i) {
			grinliz::SNPrintf(buf, 64, "This is a test. %d", i);
			if (i & 1) {
				GLASSERT(!StringPool::Has(buf));
			}
			else {
				GLASSERT(StringPool::Has(buf));
			}
		}
		StringPool::Destroy();
	}
#endif
	return true;
}
