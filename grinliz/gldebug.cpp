/*
Copyright (c) 2000-2007 Lee Thomason (www.grinninglizard.com)

Grinning Lizard Utilities. Note that software that uses the 
utility package (including Lilith3D and Kyra) have more restrictive
licences which applies to code outside of the utility package.


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

#include "gldebug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <DbgHelp.h>

void WinDebugBreak()
{
    DebugBreak();
}

#endif	//_WIN32


#if 0
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <DbgHelp.h>
#endif	//_WIN32

#ifdef ANDROID_NDK
#include <android/log.h>
#endif

#include <mutex>
#include "gldebug.h"
#include <stdio.h>
#include <time.h>
#include <memory.h>

bool gDebugging = true;
std::recursive_mutex allocatorMutex;

void SetCheckGLError(bool error)
{
	gDebugging = error;
}


#ifdef DEBUG

#ifdef GRINLIZ_DEBUG_MEM

size_t memTotal = 0;
size_t memWatermark = 0;
long memNewCount = 0;
long memDeleteCount = 0;
long memMallocCount = 0;
long memFreeCount = 0;
unsigned long MEM_MAGIC0 = 0xbaada55a;
unsigned long MEM_MAGIC1 = 0xbaada22a;
unsigned long MEM_DELETED0 = 0x12345678;
unsigned long MEM_DELETED1 = 0x9ABCDEF0;
unsigned long idPool = 0;
bool checking = false;
int distribution[32] = { 0 };

static const int NAME_SIZE = 32;

struct MemCheckHead
{
	size_t size;
	bool arrayType;
	unsigned long id;
	char stack[NAME_SIZE];
	int line;

	MemCheckHead* next;
	MemCheckHead* prev;
	unsigned long magic;
};

MemCheckHead* root = 0;

struct MemCheckTail
{
	unsigned long magic;
};


struct MTrack
{
	const void* mem;
	size_t size;
};

MTrack* mtrackRoot = 0;
unsigned long nMTrack = 0;
unsigned long mTrackAlloc = 0;

uint32_t logBase2(uint32_t v)
{
    // I don't love this implementation, and I
    // don't love the table alternatives either.
    uint32_t r = 0;
    while (v >>= 1) {
        ++r;
    }
    if (r == 24) {
        int debug = 1;
    }
    return r;
}

#ifdef GRINLIZ_DEBUG_MEM

void* Malloc( size_t size ) {
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	void* v = malloc( size );
	TrackMalloc( v, size );
	return v;
}


void* Realloc( void* v, size_t size ) {
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	if ( v ) {
		TrackFree( v );
	}
	v = realloc( v, size );
	TrackMalloc( v, size );
	return v;
}


void Free( void* v ) {
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	TrackFree( v );
	free( v );
}


void TrackMalloc( const void* mem, size_t size )
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	if ( nMTrack == mTrackAlloc ) {
		mTrackAlloc += 64;
		mtrackRoot = (MTrack*)realloc( mtrackRoot, mTrackAlloc*sizeof(MTrack) );
	}
	mtrackRoot[ nMTrack ].mem = mem;
	mtrackRoot[ nMTrack ].size = size;
	++nMTrack;

	memTotal += size;
	if ( memTotal > memWatermark )
		memWatermark = memTotal;
	
	memMallocCount++;
    uint32_t log2 = logBase2(size);
	GLASSERT( log2 < 32 );
	distribution[ log2 ] += 1;
}


void TrackFree( const void* mem )	
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	for( unsigned long i=0; i<nMTrack; ++i ) {
		if ( mtrackRoot[i].mem == mem ) {
			memTotal -= mtrackRoot[i].size;
			memFreeCount++;
			mtrackRoot[i] = mtrackRoot[nMTrack-1];
			--nMTrack;
			return;
		}
	}
	GLASSERT( 0 );
}


/*
http://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
void PrintStack()
{
     unsigned int   i;
     void         * stack[ 100 ];
     unsigned short frames;
     SYMBOL_INFO  * symbol;
     HANDLE         process;

     process = GetCurrentProcess();

     SymInitialize( process, NULL, TRUE );

     frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
     symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
     symbol->MaxNameLen   = 255;
     symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

     for( i = 0; i < frames; i++ )
     {
         SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

         GLOUTPUT(( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address ));
     }

     free( symbol );
}
*/

#ifdef GRINLIZ_STACKTRACE
void GetAllocator(char* name, int n)
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);

	void         * stack[100];
	unsigned short frames;
	U8				symbolMem[sizeof(SYMBOL_INFO)+256];

	SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolMem;
	HANDLE         process;

	memset(symbol, 0, sizeof(SYMBOL_INFO)+256);
	process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	frames = CaptureStackBackTrace(3, 1, stack, NULL);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	SymFromAddr(process, (DWORD64)(stack[0]), 0, symbol);
	//GLOUTPUT(( "%s\n", symbol->Name ));
	if (name && n) {
		int c = n - 1 < (int)symbol->NameLen ? n - 1 : symbol->NameLen;
		memcpy(name, symbol->Name, c);
		name[c] = 0;
	}
}
#endif

void* DebugNew( size_t size, bool arrayType, const char* name, int line )
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	void* mem = 0;
	if ( size == 0 )
		size = 1;

	GLASSERT( size );
	size_t allocateSize = size + sizeof(MemCheckHead) + sizeof(MemCheckTail);
	mem = malloc( allocateSize );

	MemCheckHead* head = (MemCheckHead*)(mem);
	MemCheckTail* tail = (MemCheckTail*)((unsigned char*)mem+sizeof(MemCheckHead)+size);
	void* body = (void*)((unsigned char*)mem+sizeof(MemCheckHead));
	GLASSERT( body );

	head->size = size;
	head->arrayType = arrayType;
	head->id = idPool++;
	head->line = line;

	head->magic = MEM_MAGIC0;
	tail->magic = MEM_MAGIC1;
	head->prev = head->next = 0;

	distribution[ logBase2(size) ] += 1;
#ifdef GRINLIZ_STACKTRACE
	if ( !name ) {
		GetAllocator( head->stack, NAME_SIZE );
	}
	else if ( *name ) {
		strncpy( head->stack, name, NAME_SIZE );
		head->stack[NAME_SIZE-1] = 0;
	}
#else
	if ( name ) {
		strncpy( head->stack, name, NAME_SIZE );
		head->stack[NAME_SIZE-1] = 0;
	}
#endif

	if ( checking )
	{
		++memNewCount;
		memTotal += size;
		if ( memTotal > memWatermark )
			memWatermark = memTotal;

		if ( root )
			root->prev = head;
		head->next = root;
		head->prev = 0;
		root = head;
	}

	// #BREAKHERE
	if ( head->id == 24651 ) {	// size 304
		int debug = 1;
	}

#ifdef GRINLIZ_DEBUG_MEM_DEEP
	MemHeapCheck();
#endif
	return body;
}


void DebugDelete( void* mem, bool arrayType )
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
#ifdef GRINLIZ_DEBUG_MEM_DEEP
	MemHeapCheck();
#endif
	if ( mem ) {
		MemCheckHead* head = (MemCheckHead*)((unsigned char*)mem-sizeof(MemCheckHead));
		MemCheckTail* tail = (MemCheckTail*)((unsigned char*)mem+head->size);

		// For debugging, so if the asserts do fire, we still have a copy of the values.
		MemCheckHead aHead = *head;
		MemCheckTail aTail = *tail;

		GLASSERT( head->magic == MEM_MAGIC0 );
		GLASSERT( tail->magic == MEM_MAGIC1 );
		
		// This doesn't work as I expect.
		// array allocation of primitive types: uses single form.
		// array allocation of class types: uses array form.
		// so an array destructor is meaningless - could go either way.
		// can detect array destructor of single construct...
		//GLASSERT( head->arrayType == arrayType || !head->arrayType );

		if ( head->prev )
			head->prev->next = head->next;
		else
			root = head->next;

		if ( head->next )
			head->next->prev = head->prev;
		
		if ( checking )
		{
			++memDeleteCount;
			memTotal -= head->size;
		}
		head->magic = MEM_DELETED0;
		tail->magic = MEM_DELETED1;
		free(head);
	}
}
#endif

void* operator new( size_t size ) 
{
	return DebugNew( size, false, 0, 0 );
}

void* operator new[]( size_t size ) 
{
	return DebugNew( size, true, 0, 0 );
}

void operator delete[]( void* mem ) 
{
	DebugDelete( mem, true );
}

void operator delete( void* mem ) 
{
	DebugDelete( mem, false );
}

void MemLeakCheck()
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	GLOUTPUT((	"MEMORY REPORT: watermark=%dk =%dM new count=%d. delete count=%d. %d allocations leaked.\n",
				(int)(memWatermark/1024), (int)(memWatermark/(1024*1024)),
				(int)memNewCount, (int)memDeleteCount, (int)(memNewCount-memDeleteCount) ));
	GLOUTPUT((	"               malloc count=%d. free count=%d. %d allocations leaked.\n",
				(int)memMallocCount, (int)memFreeCount, (int)(memMallocCount-memFreeCount) ));

	for( int i=0; i<30; ++i ) {
		U32 m32 = 1<<i;
		U32 div = 1;
		char suffix = 'b';
		if ( m32 >= 1024 && m32 < (1024*1024) ) {
			suffix = 'k';
			div = 1024;
		}
		else if ( m32 >= (1024*1024) ) {
			suffix = 'M';
			div = 1024*1024;
		}
		GLOUTPUT(( "  distribution[%2d] %3d%c nAlloc=%5d mem=%8.1fM\n",
				   i, 
				   m32/div, suffix,
				   distribution[i], 
				   (float)(distribution[i]*m32)/(float)(1024*1024) ));
	}

	for( MemCheckHead* node = root; node; node=node->next )
	{
		GLOUTPUT(( "  size=%d %s id=%d name=%s line=%d\n",
					(int)node->size, (node->arrayType) ? "array" : "single", (int)node->id,
					(*node->stack) ? node->stack :  "(null)", 
					node->line ));
	}		  

	for( unsigned long i=0; i<nMTrack; ++i ) {
		GLOUTPUT(( "  malloc size=%d\n", mtrackRoot[i].size ));
	}

	/*
		If these fire, then a memory leak has been detected. The library doesn't track
		the source. The best way to find the source is to break in the allocator,
		search for: #BREAKHERE. Each allocation has a unique ID and is printed out above.
		Once you know the ID of the thing you are leaking, you can track the source of the leak.

		It's not elegant, but it does work.
	*/
	GLASSERT( memNewCount-memDeleteCount == 0 && !root && !nMTrack );
	GLASSERT( memMallocCount - memFreeCount == 0 );

	// If this fires, the code isn't working or you never allocated memory:
	GLASSERT( memNewCount );
}

void MemHeapCheck()
{
	std::lock_guard<std::recursive_mutex> lock(allocatorMutex);
	// FIXME: add in walk of malloc/free list
	unsigned long size = 0;
	for ( MemCheckHead* head = root; head; head=head->next )
	{
		MemCheckTail* tail = (MemCheckTail*)((unsigned char*)head + sizeof(MemCheckHead) + head->size);

		GLASSERT( head->magic == MEM_MAGIC0 );
		GLASSERT( tail->magic == MEM_MAGIC1 );
		size += head->size;
	}
	GLASSERT( size == memTotal );
}

void MemStartCheck()
{
	checking = true;
}


#endif // GRINLIZ_DEBUG_MEM
#endif // DEBUG

FILE* relFP = 0;

void SetReleaseLog(FILE* fp)
{
	GLASSERT(!relFP);
	relFP = fp;
}




FILE* logFP = 0;

void logprintf( const char* format, ... )
{
    va_list     va;
    char		buffer[1024];

    //
    //  format and output the message..
    //
    va_start( va, format );
#ifdef _MSC_VER
    vsprintf_s( buffer, 1024, format, va );
#else
    vsnprintf( buffer, 1024, format, va );
#endif
    va_end( va );

	if ( !logFP ) {
		logFP = fopen( "test_log.txt", "w" );
	}
	fprintf( logFP, "%s", buffer );
}

#endif

#if defined(DEBUG) && defined(ANDROID_NDK)
void dprintf( const char* format, ... )
{
    va_list     va;

    //
    //  format and output the message..
    //
    va_start( va, format );
	__android_log_vprint( ANDROID_LOG_INFO, "grinliz", format, va );
    va_end( va );

}
#endif
