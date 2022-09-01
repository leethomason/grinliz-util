/*
Copyright (c) 2000-2005 Lee Thomason (www.grinninglizard.com)
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

#include "glstringutil.h"
#include "glrandom.h"
#include "glcontainer.h"

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

using namespace grinliz;

void grinliz::StrCpy(char* dst, const char* src, size_t bufferSize)
{
	if (!bufferSize) return;
	char* end = dst + bufferSize;
	char* nullEnd = end - 1;
	*nullEnd = 0;

	char* q = dst;
	while (*src && q < nullEnd) {
		*q++ = *src++;
	}
	GLASSERT(q <= nullEnd);
	*q = 0;
	GLASSERT(strlen(dst) < bufferSize);
}


void grinliz::StrCpy(char* dst, const char* src, const char* srcEnd, size_t bufferSize)
{
	if (!bufferSize) return;
	char* end = dst + bufferSize;
	char* nullEnd = end - 1;
	*nullEnd = 0;

	char* q = dst;
	while (*src && src < srcEnd && q < nullEnd) {
		*q++ = *src++;
	}
	GLASSERT(q <= nullEnd);
	*q = 0;
	GLASSERT(strlen(dst) < bufferSize);
}


int grinliz::SNPrintf(char *str, size_t size, const char *format, ...)
{
    va_list     va;

    //
    //  format and output the message..
    //
    va_start( va, format );
#ifdef _MSC_VER
    int result = vsnprintf_s( str, size, _TRUNCATE, format, va );
#else
	// Reading the spec, the size does seem correct. The man pages
	// say it will aways be null terminated (whereas the strcpy is not.)
	// Pretty nervous about the implementation, so force a null after.
    int result = vsnprintf( str, size, format, va );
	str[size-1] = 0;
#endif
    va_end( va );

	return result;
}


static const char* SkipWhiteSpace( const char* p ) 
{
	while( p && *p && isspace( *p ) ) {
		++p;
	}
	return p;
}

static const char* FindWhiteSpace( const char* p ) 
{
	while( p && *p && !isspace( *p ) ) {
		++p;
	}
	return p;
}


StringLens StringLens::TrimWS() const
{
	const char* p = Start();
	while (p < End() && isspace(*p)) {
		++p;
	}
	return StringLens(p, End());
}


StringLens StringLens::TrimTrailingWS() const
{
	const char* p = End() - 1;
	while (p >= Start() && isspace(*p)) {
		--p;
	}
	return StringLens(Start(), p + 1);
}


StringLens StringLens::TrimBothWS() const
{
	return TrimWS().TrimTrailingWS();
}


StringLens StringLens::PastNL() const
{
	// go past \n or \r\n
	const char* p = Start();
	while (p < End() && (*p != '\n')) {
		++p;
	}
	if (p < End() && *p == '\n') {
		return StringLens(p + 1, End());
	}
	return StringLens(End(), End());
}


StringLens StringLens::BeforeNL() const
{
	const char* p = Start();
	while (p < End() && (*p != '\n')) {
		++p;
	}
	if (*p == '\n') {
		if (p > Start() && (*(p-1) == '\r')) {
			return StringLens(Start(), p - 1);
		}
		return StringLens(Start(), p);
	}
	return *this;
}


bool StringLens::EndsWith(const char* str) const
{
	size_t len = strlen(str);
	if (Length() < len) return false;
	return memcmp(End() - len, str, len) == 0;
}


bool StringLens::Equals(const char* str) const 
{
	if (!Start() || !str) return false;

	const char* p = Start();
	const char* q = str;
	while (p < End() && *p == *q) {
		++p;
		++q;
	}
	return *q == 0 && p == End();
}

bool StringLens::Equals(const StringLens& lens) const
{
	if (IsEmpty() || lens.IsEmpty()) return false;
	if (Length() != lens.Length()) return false;
	return strncmp(Start(), lens.Start(), Length()) == 0;
}


Tokenizer::Tokenizer(const char* in, const char* delim)
{
	buf[0] = 0;
	tail[0] = 0;

	if (!in) return;
	{
		const char* end = buf + MAX_CHAR - 1;
		const char* p = in;
		char* q = buf;

		bool wasSpace = false;
		while (*p && q < end) {
			if (isspace(*p)) {
				if (wasSpace) {
					++p;
					continue;
				}
				wasSpace = true;
				*q++ = ' ';			// all types of space become ' '
				p++;
			}
			else {
				wasSpace = false;
				*q++ = *p++;
			}
		}
		*q = 0;
	}

	// strtok is cool, but not thread safe.
	// That's...a little crazy in C++ 2011.

	size_t keyLen = strlen(delim);
	char* p = buf;

	// trim head & tail
	static const char* TRIM = " \n\t";
	while (*p && InSet(*p, TRIM))
		++p;
	size_t len = strlen(p);
	GLASSERT(strlen(buf) < MAX_CHAR);

	char* end = p + len - 1;
	while (end >= p && InSet(*end, TRIM)) {
		*end = 0;
		--end;
	}

	while (true) {
		tokens[nTokens++] = (uint8_t)(p - buf);

		char* q = strstr(p, delim);
		if (!q) break;

		p = q + keyLen;
	}

	if (nTokens > 1) {
		const char* second = buf + tokens[1];
		size_t nBytes = MAX_CHAR - 1 - tokens[1];
		strncpy_s(tail, MAX_TAIL, in + tokens[1], nBytes);
	}

	for (int i = 1; i < nTokens; ++i) {
		buf[tokens[i] - keyLen] = 0;
	}
//	for (int i = 0; i < nTokens; ++i) {
//		printf("tok %d %s\n", i, buf + tokens[i]);
//	}
}

void Tokenizer::Test()
{
	{
		Tokenizer tok(0);
		GLASSERT(tok.Size() == 0);
	}
	{
		const char* t = "  aaa  ";
		Tokenizer tok(t);
		GLASSERT(strcmp(tok.First(), "aaa") == 0);
		GLASSERT(strcmp(tok.Get(0), "aaa") == 0);
		GLASSERT(tok.FirstEquals("aaa"));
		GLASSERT(tok.GetEquals(0, "aaa"));
		GLASSERT(tok.Size() == 1);
		GLASSERT(*tok.Tail() == 0);
	}
	{
		const char* t = ",1,,3,";
		Tokenizer tok(t, ",");
		GLASSERT(tok.Size() == 5);
		GLASSERT(tok.GetEquals(0, ""));
		GLASSERT(tok.GetEquals(1, "1"));
		GLASSERT(tok.GetEquals(2, ""));
		GLASSERT(tok.GetEquals(3, "3"));
		GLASSERT(tok.GetEquals(4, ""));
	}
	{
		const char* t = "This is a test of the tail.";
		Tokenizer tok(t);
		GLASSERT(tok.Size() == 7);
		GLASSERT(tok.TailEquals("is a test of the tail."));
		GLASSERT(tok.FirstEquals("This"));
		GLASSERT(tok.GetEquals(1, "is"));
		GLASSERT(tok.GetEquals(2, "a"));
		GLASSERT(tok.GetEquals(3, "test"));
		GLASSERT(tok.GetEquals(4, "of"));
		GLASSERT(tok.GetEquals(5, "the"));
		GLASSERT(tok.GetEquals(6, "tail."));
		GLASSERT(!tok.GetEquals(6, "tail. "));
	}
	{
		const char* t = "v  33.1903 8.4008 11.8683\n";
		Tokenizer tok(t);
		GLASSERT(tok.Size() == 4);
		GLASSERT(tok.GetEquals(0, "v"));
		GLASSERT(tok.GetEquals(1, "33.1903"));
		GLASSERT(tok.GetEquals(2, "8.4008"));
		GLASSERT(tok.GetEquals(3, "11.8683"));
	}
	{
		char* t = new char[300];
		memset(t, ' ', 299);
		t[299] = 0;
		for (int i = 0; i < 298; i += 2) {
			t[i] = 'a';
		}
		GLASSERT(strlen(t) == 299);
		Tokenizer tok(t);
		delete[] t;

		GLASSERT(tok.Size() == 128);
		for (int i = 0; i < 128; i++) {
			GLASSERT(tok.GetEquals(i, "a"));
		}
	}
	{
		const char* test = "  Line 1\nLine 2\r\nLine 3";
		StringLens str(test);

		StringLens trim = str.BeforeNL();
		GLASSERT(trim.Equals("  Line 1"));

		StringLens str3 = str.PastNL().PastNL();
		StringLens str2 = str.PastNL().BeforeNL();
		StringLens str1 = str.BeforeNL().TrimWS();

		GLASSERT(str1.Equals("Line 1"));
		GLASSERT(str2.Equals("Line 2"));
		GLASSERT(str3.Equals("Line 3"));

		const char* tws1 = "  WS  ";
		const char* tws2 = "WS  ";
		const char* tws3 = "  WS";
		
		StringLens ws1 = StringLens(tws1).TrimWS().TrimTrailingWS();
		StringLens ws2 = StringLens(tws2).TrimWS().TrimTrailingWS();
		StringLens ws3 = StringLens(tws3).TrimWS().TrimTrailingWS();

		GLASSERT(ws1.Equals("WS"));
		GLASSERT(ws2.Equals("WS"));
		GLASSERT(ws3.Equals("WS"));

		const char* c2 = "This is a string.   ";
		StringLens a("This is a string.");
		StringLens b = StringLens(c2).TrimBothWS();
		GLASSERT(a.Equals(b));
	}
}

