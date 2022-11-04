#pragma once
#include "glcontainer.h"
#include "glstringpool.h"

#include <stdint.h>
#include <stdio.h>
#include <string>

namespace grinliz
{
	class Writer {
	public:
		virtual void Write(const void* mem, int size) = 0;

		template<typename T>
		void Write(const T& t) { Write(&t, sizeof(T)); }

		void WriteU32(uint32_t t) { Write(t); }
		void WriteI32(int32_t t) { Write(t); }
		void WriteStr(const std::string& str);
		void WriteIStr(const grinliz::IString& str);
		void WriteCStr(const char*);
	};

	class Reader {
	public:
		virtual void Read(void* mem, int size) = 0;

		template<typename T>
		void Read(T* t) { Read(t, sizeof(T)); }

		template<typename T>
		T Read() { T t; Read(&t); return t; }

		uint32_t ReadU32() { return Read<uint32_t>(); }
		int32_t ReadI32() { return Read<int32_t>(); }
		std::string ReadStr();
		grinliz::IString ReadIStr();
		const char* ReadCStr();

		std::vector<char> cStrBuf;
	};

	class MemWriterReader : public Writer, public Reader
	{
	public:
		DynMemBuf memBuf;

		virtual void Write(const void* mem, int size) override {
			memBuf.Add(mem, size);
		}

		virtual void Read(void* mem, int size) override {
			GLASSERT(pos + size <= memBuf.Size());
			memcpy(mem, (uint8_t*)memBuf.Mem() + pos, size);
			pos += size;
		}
		int pos = 0;
	};

	class FileWriter : public Writer
	{
	public:
		FileWriter(FILE* _fp) : fp(_fp) {}

		virtual void Write(const void* mem, int size) override {
			fwrite(mem, size, 1, fp);
		}

	private:
		FILE* fp;
	};

	class FileReader : public Reader
	{
	public:
		FileReader(FILE* _fp) : fp(_fp) {}

		virtual void Read(void* mem, int size) override {
			fread(mem, size, 1, fp);
		}

	private:
		FILE* fp;
	};
}
