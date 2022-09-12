/*
Copyright (c) 2000 Lee Thomason (www.grinninglizard.com)
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



#ifndef GRINLIZ_STRINGUTIL_INCLUDED
#define GRINLIZ_STRINGUTIL_INCLUDED

#ifdef _MSC_VER
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "gldebug.h"
#include "glcontainer.h"


namespace grinliz 
{

inline bool StrEqual( const char* a, const char* b ) 
{
	if ( a && b ) {
		if ( a == b ) 
			return true;
		else if ( strcmp( a, b ) == 0 )
			return true;
	}
	return false;
}


inline bool StrStartsWith(const char* a, const char* substr)
{
	if (a && substr) {
		if (a == substr)
			return true;
		const char* p = a;
		const char* q = substr;
		while (*p && *q && (*p == *q)) {
			++p;
			++q;
		}
		if (*q == 0) return true;
	}
	return false;
}


// Reimplements SAFE strcpy, safely cross-compiler. Always returns a null-terminated string.
// Here for compatibility. strcpy_s is clearly the way to go.
// dst: destination string
// src: source string
// bufferSize: size of the dst buffer. Will always be null terminated. 'char buf[N]' will have buf[N-1] = 0
void StrCpy(char* dst, const char* src, size_t bufferSize);
void StrCpy(char* dst, const char* src, const char* srcEnd, size_t bufferSize);
int SNPrintf(char *str, size_t size, const char *format, ...);


/*
	A class that wraps a c-array of characters.
*/
template< int ALLOCATE >
class CStr
{
public:
	CStr()							{	GLASSERT(sizeof(*this) == ALLOCATE );		// not required for class to work, but certainly the intended design
										buf[0] = 0; 
									}
	CStr( const char* src )			{	GLASSERT(sizeof(*this) == ALLOCATE );
										*buf = 0;
										if ( src ) {
											GLASSERT( strlen( src ) < (ALLOCATE-1));
											StrCpy( buf, src, ALLOCATE ); 
										}
										Validate();
									}
	CStr( const CStr<ALLOCATE>& other ) {
										memcpy( buf, other.buf, ALLOCATE );
										Validate();
									}

	~CStr()	{}

	const char* c_str()	const			{ return buf; }
	int size() const					{ return (int)strlen( buf ); }
	bool empty() const					{ return buf[0] == 0; }

	int Length() const 					{ return strlen( buf ); }
	int Capacity() const				{ return ALLOCATE-1; }
	int Allocated() const				{ return ALLOCATE; }
	void ClearBuf()						{ memset( buf, 0, ALLOCATE ); }
	void Clear()						{ buf[0] = 0; }

	void Format( const char* format, ...) 
	{
		va_list     va;
		va_start( va, format );
	#ifdef _MSC_VER
		vsnprintf_s( buf, ALLOCATE, _TRUNCATE, format, va );
	#else
		vsnprintf( buf, ALLOCATE, format, va );
	#endif
		va_end( va );
		Validate();
	}

	void AppendFormat( const char* format, ... )
	{
		va_list     va;
		va_start( va, format );
		int s = size();
		if ( s < Capacity() ) {
			#ifdef _MSC_VER
				vsnprintf_s( buf+s, ALLOCATE-s, _TRUNCATE, format, va );
			#else
				vsnprintf( buf+s, ALLOCATE-s, format, va );
			#endif
		}
		va_end( va );
		Validate();
	}

	bool operator==( const char* str ) const						{ return buf && str && strcmp( buf, str ) == 0; }
	bool operator!=( const char* str ) const						{ return !(*this == str); }
	char operator[]( int i ) const									{ GLASSERT( i>=0 && i<ALLOCATE-1 ); return buf[i]; }
	char& operator[]( int i ) 										{ GLASSERT( i>=0 && i<ALLOCATE-1 ); return buf[i]; }
	template < class T > bool operator==( const T& str ) const		{ return buf && strcmp( buf, str.c_str() ) == 0; }

	void operator=( const char* src )	{	
		if (src && *src) {
			GLASSERT( strlen( src ) < (ALLOCATE-1) );
			StrCpy( buf, src, ALLOCATE ); 
		}
		else {
			buf[0] = 0;
		}
		Validate();
	}
	
	void operator=( int value ) {
		SNPrintf( buf, ALLOCATE, "%d", value );
		Validate();
	}

	void operator+=( const char* src ) {
		GLASSERT( src );
		if ( src ) {
			int len = size();
			if ( len < ALLOCATE-1 )
				StrCpy( buf+len, src, ALLOCATE-len );
		}
		Validate();
	}

	void operator+=( int c ) {
		GLASSERT( c > 0 );
		int len = size();
		if ( len < ALLOCATE-1 ) {
			buf[len] = (char)c;
			buf[len+1] = 0;
		}
		Validate();
	}

#ifdef DEBUG
	void Validate() {
		GLASSERT( strlen(buf) < ALLOCATE );	// strictly less - need space for null terminator
	}
#else
	void Validate() {}
#endif
	char buf[ALLOCATE];
};


// View of character data. Data needs to be constant. 
// Not null-terminated. Useful, but now replaced by std::string_view
class StringLens
{
public:
	StringLens(const char* p) {
		start = p;
		end = p + strlen(p);
	}
	StringLens(const char* p, int len) {
		start = p;
		end = p + len;
	}
	StringLens(const char* _start, const char* _end) : start(_start), end(_end) {}
	StringLens(const StringLens& lens) { start = lens.start; end = lens.end; }

	const char* Start() const { return start; }
	const char* End() const { return end; }
	size_t Length() const { return end - start; }
	bool IsEmpty() const { return start == end; }

	StringLens TrimWS() const;
	StringLens TrimTrailingWS() const;
	StringLens TrimBothWS() const;
	StringLens PastNL() const;
	StringLens BeforeNL() const;

	bool EndsWith(const char* str) const;

	bool Equals(const char* str) const;
	bool Equals(const StringLens& lens) const;

private:
	const char* start = 0;
	const char* end = 0;
};


};	// namespace grinliz


#endif
