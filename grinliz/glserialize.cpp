#include "glserialize.h"

using namespace grinliz;

void Writer::WriteStr(const std::string& str)
{
	GLASSERT(!str.empty());
	WriteI32((int32_t)str.size());
	Write(str.c_str(), (int)str.size());
}

void Writer::WriteCStr(const char* str)
{
	GLASSERT(str);
	int32_t len = (int32_t)strlen(str);
	WriteI32(len);
	Write(str, len);
}

void Writer::WriteIStr(const grinliz::IString& str)
{
	GLASSERT(!str.Empty());	// need to handle??
	WriteI32(str.Length());
	Write(str.c_str(), str.Length());
}

void Writer::WriteCompStr(const grinliz::CompString& str)
{
	WriteIStr(StringPool::Intern(str));
}

grinliz::CompString Reader::ReadCompStr()
{
	return StringPool::InternC(ReadIStr());
}


grinliz::IString Reader::ReadIStr()
{
	grinliz::DynMemBuf buf;
	int32_t len = ReadI32();
	buf.SetSize(len + 1);
	Read(buf.Mem(), len);
	*((char*)buf.Mem() + len) = 0;
	return StringPool::Intern((const char*)buf.Mem());
}

std::string Reader::ReadStr()
{
	int32_t size = ReadI32();
	std::string str;
	for (int32_t i = 0; i < size; ++i) {
		char c;
		Read(&c, 1);
		str += c;
	}
	return str;
}

char* Reader::ReadCStr()
{
	int32_t size = ReadI32();
	char* p = (char*) malloc(size + 1);
	Read(p, size);
	p[size] = 0;
	return p;
}
