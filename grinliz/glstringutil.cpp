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


