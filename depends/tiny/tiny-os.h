#pragma once

/****************************************************** //
//              tiny-for-c++ v3 library                 //
//              -----------------------                 //
//   Copyright (c) 2016-2024 | NEXTWave Technologies    //
//      <http://www.nextwave-techs.com/>                //
// ******************************************************/

//! Check if your project qualifies for a free license
//!   at http://nextwave-techs.com/license

//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//!        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.
//////////////////////////////////////////////////////////////////////////
#ifndef _TINY_OS_H_
#define _TINY_OS_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! NOTE 

//////////////////////////////////////////////////////////////////////////
//! Windows OS
//  > using alpha blend requires linking with "MSIMG32.LIB"
//  > network based on winsock2, linking with "WS2_32.LIB"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#if defined(_WIN32)
 #define PLATFORM_WINDOWS
#include <windows.h>

#elif defined(__linux__)
 #define PLATFORM_LINUX

#elif defined(__APPLE__)
 #define PLATFORM_APPLE

 #ifdef TARGET_OS_IPHONE
  #define PLATFORM_IOS
 #else
  #define PLATFORM_OSX
 #endif

#elif defined(__ANDROID__)
 #define PLATFORM_ANDROID

#else
 // #elif defined(__FreeBSD__) //! if support were to be added

 #pragma warning "tinyc did not found a supported platform"

#endif

//////////////////////////////////////////////////////////////////////////
//! _MSC_VER -> _WIN32 | _WIN64 
//! __GNUC__ -> __x86_64__ || __ppc64__
//! LINUX -> _LP64

#if _WIN32 || _WIN64
#if _WIN64
#define PLATFORM_64BIT
#else
#define PLATFORM_32BIT
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define PLATFORM_64BIT
#else
#define PLATFORM_32BIT
#endif
#endif

//////////////////////////////////////////////////////////////////////////
//! C++ version

#define CXX_PRE 1
#define CXX_98  199711L
#define CXX_11  201103L
#define CXX_14  201402L
#define CXX_17  201703L
#define CXX_20  202002L

//////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h> //! only using the error definition

//////////////////////////////////////////////////////////////////////////
#include <stdint.h>

/* 
//! here below the types used from <stdint.h>
	// => uncoment this section if <stdint.h> file is not available

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;
*/

#ifndef TINY_NOUNIT
 // #ifndef _WIN32
 typedef unsigned char		byte;
 typedef unsigned short	    word;
 typedef unsigned long		dword;
 typedef unsigned long long qword;
 
 #ifndef BYTE
  #define BYTE	byte
 #endif
 #ifndef WORD
  #define WORD	word
 #endif
 #ifndef DWORD
  #define DWORD	dword
 #endif
 #ifndef QWORD
  #define QWORD	qword 
 #endif

#endif // NOTINYUNIT
 
//////////////////////////////////////////////////////////////////////////
#ifndef TINY_NOMINMAX
 #ifndef MIN
  #define MIN(a,b) (((a)<(b))?(a):(b))
 #endif
 #ifndef MAX
  #define MAX(a,b) (((a)>(b))?(a):(b))
 #endif

 #ifndef __cplusplus
  #ifndef min
   #define min(a,b) (((a)<(b))?(a):(b))
  #endif
  #ifndef max
   #define max(a,b) (((a)>(b))?(a):(b))
  #endif
 #endif
#endif // NOMINMAX

#define CLAMP(a,b,c)	MAX(MIN((a),(c)),(b))

 //////////////////////////////////////////////////////////////////////////
#define TINY_STR2(x)		#x
#define TINY_STR(x)			TINY_STR2(x)

//////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_WINDOWS)
 #define I64_FMT "I64"
 #define I64_LFMT L"I64"

#elif defined(PLATFORM_APPLE) 
 #define I64_FMT "q"
 #define I64_LFMT L"q"

#else
 #define I64_FMT "ll"
 #define I64_LFMT L"ll"

#endif

//////////////////////////////////////////////////////////////////////////
#if defined(UNICODE) || defined(_UNICODE)
 typedef wchar_t char_t;
#else
 typedef char char_t;
#endif

#ifndef result_t
 typedef unsigned int result_t;
#endif

#ifndef offset_t
 typedef size_t offset_t;
#endif

//////////////////////////////////////////////////////////////////////////
#ifndef NOTINYCGUI
 #define _NATIVE_GUI_
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
 #include <tchar.h> //? correct

#else
 #define _ASSERT(a)		assert(a)

 #define stricmp    strcasecmp
 #define strnicmp   strncasecmp
 #define wcsicmp    wcscasecmp
 #define wcsnicmp   wcsncasecmp
 #define _strdup    strdup
 #define _wcsdup	wcsdup
 
 #define strcpy_s( _a, _size, _b)			(strcpy( _a, _b)!=NULL?0:EINVAL)
 #define wcscpy_s( _a, _size, _b)			(wcscpy( _a, _b)!=NULL?0:EINVAL)
 #define strcat_s(_a,_size,_b)				(strcat(_a,_b))
 #define wcsncat_s(_a,_size,_count,_b)		(wcsncat(_a,_count,_b))
 
 #define sprintf_s(_a, _size, ...)	          (sprintf(_a, ##__VA_ARGS__))
 #define vsprintf_s(_a, _size, ...)	          (vsprintf(_a, ##__VA_ARGS__))
 #define swprintf_s(_a, _size, ...)	          (swprintf(_a,_size, ##__VA_ARGS__))
 #define _snscanf_s(_a,_size, ...)             (sscanf(_a, ##__VA_ARGS__))
 #define _vsnprintf_s(_a,_size,_count,_b,...)  (_vsnprintf(_a,_size - 1,_b,##__VA_ARGS__))
 #define _vsnwprintf_s(_a,_size,_count,_b,...) (_vsnwprintf(_a,_size - 1,_b,##__VA_ARGS__))

 #if defined(UNICODE) || defined(_UNICODE)
  #define _T(x)			L##x
  #define _tcscpy_s       wcscpy_s
  #define _stprintf_s     swprintf_s
  #define _tcscat_s       wcscat_s
  #define _tcsncpy_s      wcsncpy_s
  #define _tcstok_s       wcstok_s
  #define _tgetcwd        _wgetcwd
  #define _vsntprintf_s   _vsnwprintf_s
  #define _tcsncat_s      wcsncat_s
  #define _i64tot_s       _i64tow_s
  #define _ui64tot_s      _ui64tow_s

  #define _tcsdup    	_wcsdup

 #else
  #define _T(x)         x
  #define _tcscpy_s		strcpy_s
  #define _stprintf_s	sprintf_s
  #define _tcscat_s		strcat_s
  #define _tcsncpy_s	strncpy_s
  #define _tcstok_s		strtok_s
  #define _tgetcwd		_getcwd
  #define _vsntprintf_s	_vsnprintf_s
  #define _tcsncat_s	strncat_s
  #define _i64tot_s		_i64toa_s
  #define _ui64tot_s	_ui64toa_s

  #define _tcsdup    	_strdup

 #endif	// UNICODE

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Pointer helpers

#define SAFECALL(__p_)				if(__p_) __p_
#define SAFEFREE(__p_)				{ if(__p_) { free(__p_); p=NULL; } }
#define SAFEOSFREE(__p_)			{ if(__p_) { OSFREE(__p_); p=NULL; } }

#define ARRAYSIZEOF(__p_)		(sizeof(__p_) / sizeof(__p_[0]))
#define ARRAYLASTOF(__p_)		(__p_[ARRAYSIZEOF(__p_)-1])

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Tool structures

#define OS_RECT_WIDTH(__r_)		(__r_.right - __r_.left)
#define OS_RECT_HEIGHT(__r_)	(__r_.bottom - __r_.top)

struct OsPoint { int x ,y; };
struct OsRect { int left ,top ,right ,bottom; };
struct OsColor { uint8_t r ,g ,b ,a; };

typedef uint32_t OsColorRef;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
 #define TINYFUN extern "C"
#else
 #define TINYFUN
#endif

//////////////////////////////////////////////////////////////////////////
//! Tool functions

TINYFUN int32_t log2i( int32_t x );
TINYFUN int32_t log2ceil( int32_t x );

//////////////////////////////////////////////////////////////////////////
#ifndef ENOERROR
 #define ENOERROR			(0)
#endif

#ifndef EFAILED
 #define EFAILED				(255)
#endif

#define OS_SUCCEED(__err_)	(__err_==ENOERROR)
#define OS_FAILED(__err_)	(__err_!=ENOERROR)

typedef uint32_t OsError;

TINYFUN const char_t *OsErrorGetText( OsError error );

//////////////////////////////////////////////////////////////////////////
#define OS_INVALID_HANDLE	(0)

typedef void* OsHandle;

enum OsHandleType { osNotAnHandle=0 
	,osLibraryHandle ,osSemaphoreHandle ,osSectionHandle ,osThreadHandle ,osFileHandle ,osNetHandle 
	,osGuiWindowHandle ,osGuiFontHandle ,osGuiImageHandle
};

TINYFUN enum OsHandleType OsHandleGetType( OsHandle handle );
TINYFUN OsError OsHandleDestroy( OsHandle *handle );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! system

struct OsSystemInfo
{
	uint32_t _processorCount;
	uint32_t _logicalCoreCount;
	uint32_t _physicalCoreCount;
};

TINYFUN OsError OsSystemGetInfo( struct OsSystemInfo *info );
TINYFUN OsError OsSystemSetGlobalTimer( uint32_t ms );
TINYFUN OsError OsSystemDoEvents( void );
TINYFUN OsError OsSystemPostQuit( uint32_t exitCode );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! memory

#define OS_MEMORY_DEFAULT	1 //?0

#define OS_MEMORY_ALIGN(x)	(log2ceil(x)&0x0ff) 
//! x in macro argument is in bit (EG 64 -> 64 bit alignment)
//! align flag is 2^n (EG 6 -> 2^6 = 64 bit alignment)

#define OS_MEMORY_ZERO		0x010000	//! zero memory after allocation

TINYFUN void *OsMemoryAlloc( size_t size ,int flags );
TINYFUN void *OsMemoryRealloc( void *memory ,size_t size ,int flags );
TINYFUN void OsMemoryFree( void *memory );

//////////////////////////////////////////////////////////////////////////
// flags contains alignment OS_MEMORY_ALIGN(value) and initialization OS_MEMORY_ALIGN(x) information
#define OSALLOC(size, flags)		OsMemoryAlloc(size,flags)
#define OSREALLOC(mem,size,flags)	OsMemoryRealloc(mem,size,flags)
#define OSFREE(mem)					OsMemoryFree(mem)

//////////////////////////////////////////////////////////////////////////
struct OsMemoryInfo
{
	uint32_t _memoryLoad; //! in percent (0..100)
	uint64_t _memoryPhysicalTotal; //! total physical memory (in bytes)
	uint64_t _memoryPhysicalAvail; //! available physical memory (in bytes)
	uint64_t _memoryVirtualTotal; //! total virtual memory (in bytes)
	uint64_t _memoryVirtualAvail; //! available virtual memory (in bytes)
};

TINYFUN OsError OsMemoryGetInfo( struct OsMemoryInfo *info );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! library

//TODO

// void LoadLibrary( const char *name );
// GetProcAddress
//? CallProcedure
//> require the DLL to have a 'Dispatch' function
//> use warpper DLL to encapsulate libraries that do not expose one (EG foreign libs)
//? auto generate those dispatch warper/function ?

//////////////////////////////////////////////////////////////////////////
//! Clipboard

//TODO

//////////////////////////////////////////////////////////////////////////
//! Asynch (lock less)

//////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_WINDOWS

 #define __align_n(n)				__declspec (align(n))

 #ifdef PLATFORM_64BIT
  #define ASYNCH_TYPE				__int64

  #define ASYNCH_ADD(__x,__y)		InterlockedAdd64((LONGLONG*)&__x,__y)	//! returns the new added value
  #define ASYNCH_READ(__x)			InterlockedExchangeAdd64((ASYNCH_DECL*)&__x,0)	//! returns the variable (initial) value
  #define ASYNCH_WRITE(__x,__y)		InterlockedExchange64((ASYNCH_DECL*)&__x,__y)		//! returns the variable (initial) value
  #define ASYNCH_INCREMENT(__x)		InterlockedIncrement64((ASYNCH_DECL*)&__x)		//! returns the new incremented value
  #define ASYNCH_DECREMENT(__x)		InterlockedDecrement64((ASYNCH_DECL*)&__x)		//! returns the new decremented value

 #else
  #define ASYNCH_TYPE				long

  //#define _ASYNCH_ADD(__x,__y)		InterlockedAdd(&__x,__y)		//! returns the new added value
  #define ASYNCH_ADD(__x,__y)		InterlockedExchangeAdd(&__x,__y)		//! returns the variable (initial) value
  #define ASYNCH_READ(__x)			InterlockedExchangeAdd((ASYNCH_DECL*)&__x,0)	//! returns the variable (initial) value
  #define ASYNCH_WRITE(__x,__y)		InterlockedExchange((ASYNCH_DECL*)&__x,__y)		//! returns the variable (initial) value
  #define ASYNCH_INCREMENT(__x)		InterlockedIncrement((ASYNCH_DECL*)&__x)		//! returns the new incremented value
  #define ASYNCH_DECREMENT(__x)		InterlockedDecrement((ASYNCH_DECL*)&__x)		//! returns the new decremented value

 #endif

//////////////////////////////////////////////////////////////////////////
#elif defined(PLATFORM_LINUX)

 #define __align_n(n)				__attribute__((aligned(n)))

 //////////////////////////////////////////////////////////////////////////
 #ifdef PLATFORM_64BIT
  #define ASYNCH_TYPE				long long
 #else
  #define ASYNCH_TYPE				long
 #endif

 #define ASYNCH_ADD(__x,__y)		__sync_fetch_and_add((ASYNCH_DECL*) &__x,__y)	//! returns the new added value
 #define ASYNCH_READ(__x)			__sync_fetch_and_add((ASYNCH_DECL*)&__x,0)	//! returns the variable (initial) value
 //!TOCHECK __sync_synchronize(); //IE is only acquire __sync_lock_test_and_set
 #define ASYNCH_WRITE(__x,__y)		__sync_lock_test_and_set((ASYNCH_DECL*)&__x,__y)	//! returns the variable (initial) value
 #define ASYNCH_INCREMENT(__x)		__sync_add_and_fetch((ASYNCH_DECL*)&__x,1)		//! returns the new incremented value
 #define ASYNCH_DECREMENT(__x)		__sync_add_and_fetch((ASYNCH_DECL*)&__x,-1)		//! returns the new decremented value

//////////////////////////////////////////////////////////////////////////
#else
 #pragma warning "[WARNING] tiny for C - no asynch object support for this platform"

#endif

//////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_64BIT
 // #define __align					__align_n(8)
#else
 // #define __align					__align_n(4)
#endif

//////////////////////////////////////////////////////////////////////////
#define ASYNCH_UTYPE			unsigned ASYNCH_TYPE
#define ASYNCH_STYPE			signed ASYNCH_TYPE
#define ASYNCH_DECL				volatile ASYNCH_TYPE
#define ASYNCH_VARIABLE			__align ASYNCH_TYPE

//////////////////////////////////////////////////////////////////////////
#define __cache_line_size			64
#define __align_cache				__align_n(__cache_line_size)
#define CACHE_PAD(_n_,_x_)			char _pad##_n_[__cache_line_size - (sizeof(_x_)%__cache_line_size)]

//////////////////////////////////////////////////////////////////////////
//! Process

TINYFUN OsError OsProcessRun( OsHandle *handle ,const char *path ,const char *argv[] ,const char *envp[] );
TINYFUN OsError OsProcessKill( OsHandle *handle ,int exitCode );

//TODO //+ redirect input/output

//////////////////////////////////////////////////////////////////////////
//! Threading

struct OsEventMessage;

typedef OsError (*OsEventFunction)( const struct OsEventMessage *eventMessage ,void *userData );

//////////////////////////////////////////////////////////////////////////
#define OS_TIMEOUT_INFINITE		((uint32_t)-1)

#define OS_THREAD_CREATENORMAL		((uint32_t) 0) 
#define OS_THREAD_CREATESUSPENDED	((uint32_t) 1)

enum OsThreadPriority { osPriorityUnknown=0 ,osPriorityIdle ,osPriorityLowest ,osPriorityBelowNormal ,osPriorityNormal ,osPriorityAboveNormal ,osPriorityHighest ,osPriorityCritical };

//-- thread
TINYFUN OsError OsThreadCreate( OsHandle *handle ,uint32_t createFlags ,OsEventFunction function ,void *userData );
TINYFUN OsError OsThreadSetPriority( OsHandle handle ,enum OsThreadPriority priority );
TINYFUN OsError OsThreadSuspend( OsHandle handle );
TINYFUN OsError OsThreadResume( OsHandle handle );
TINYFUN OsError OsThreadTerminate( OsHandle handle ,uint32_t exitCode );
TINYFUN OsError OsThreadStop( OsHandle handle ,int32_t timeoutMs );
// TODO ThreadWait
TINYFUN OsError OsThreadQuit( OsHandle handle );
TINYFUN OsError OsThreadGetExitCode( OsHandle handle ,uint32_t *exitCode );
TINYFUN OsError OsThreadGetSelfId( uint32_t *id );
TINYFUN OsError OsThreadGetId( OsHandle handle ,uint32_t *id );

//-- semaphore
TINYFUN OsError OsSemaphoreCreate( OsHandle *handle ,uint32_t initialCount ,const char_t *name );
TINYFUN OsError OsSemaphoreLock( OsHandle handle ,int32_t timeoutMs );
TINYFUN OsError OsSemaphoreUnlock( OsHandle handle );

//-- event
//TODO

//-- critical section
TINYFUN OsError OsCriticalSectionCreate( OsHandle *handle );
TINYFUN OsError OsCriticalSectionTryEnter( OsHandle handle );
TINYFUN OsError OsCriticalSectionEnter( OsHandle handle );
TINYFUN OsError OsCriticalSectionLeave( OsHandle handle );

//-- synch function
TINYFUN OsError OsWaitForObject( OsHandle handle ,int32_t timeoutMs );
//! thread, (TODO) semaphore
TINYFUN OsError OsSleep( int32_t delayMs );

//////////////////////////////////////////////////////////////////////////
//! high resolution timer

typedef uint64_t OsTimerCycle;
typedef uint64_t OsTimerTime;

TINYFUN OsTimerCycle OsTimerGetResolution( void ); //! returns cycles per seconds 
TINYFUN OsTimerCycle OsTimerGetTimeNow( void );
TINYFUN OsTimerCycle OsTimerGetElapsed( OsTimerCycle fromCycle ,OsTimerCycle toCycle );
TINYFUN OsTimerTime OsTimerConvertToMs( OsTimerCycle cycles );
TINYFUN OsTimerTime OsTimerConvertToNanosec( OsTimerCycle cycles );
//! must call OsTimerGetResolution before calling this function
//TODO .. not

//////////////////////////////////////////////////////////////////////////
//! file (drive)

#define OS_DRIVE_UNKNOWN	0
#define OS_DRIVE_FIXED		1
#define OS_DRIVE_REMOVABLE	2
#define OS_DRIVE_REMOTE		3
#define OS_DRIVE_CDROM		4
#define OS_DRIVE_RAMDISK	5

static const char* os_drive_type[] = {"Unkwown Drive", "Fixed Drive", "Removable Drive", "Remote Drive", "CDROM Drive", "RAM Disk Drive"};

struct OsFileDriveInfo
{
	int driveType;

	size_t totalSize;
	size_t freeSize;

	const char rootPathname[512]; // TODO .. ? MAX_PATH
};

TINYFUN OsError OsFileGetDriveList( char_t *drivenames ,size_t sizeOfDrivenames );
TINYFUN OsError OsFileGetDriveInfo( const char_t *drivename ,struct OsFileDriveInfo *info );

//////////////////////////////////////////////////////////////////////////
//! file (directory)

struct OsFileAttributes
{
    unsigned int directory : 1;
    unsigned int archive : 1;
    unsigned int hidden : 1;
    unsigned int readonly : 1;
    unsigned int system : 1;
    unsigned int reserved : 27;
};

struct OsFileTime
{
    word wYear;
    word wMonth;
    word wDayOfWeek;
    word wDay;
    word wHour;
    word wMinute;
    word wSecond;
    word wMilliseconds;
};

struct OsFileFileInfo
{
	struct OsFileAttributes _attributes;

	struct OsFileTime creationTime;
	struct OsFileTime lastAccessTime;
	struct OsFileTime lastWriteTime;

	size_t _filesize;

	char_t filename[512]; // TODO ... ? MAX_PATH
};

TINYFUN OsError OsFileGetAttributes( const char_t *filename ,struct OsFileAttributes *attributes );
TINYFUN OsError OsFileGetFileInfo( const char_t *filename ,struct OsFileFileInfo *info );
TINYFUN OsError OsFileGetSizeByName( const char_t *filename ,uint64_t *size );
TINYFUN OsError OsFileCopy( const char_t *oldFileName ,const char_t *newFileName ,int failifexist );
TINYFUN OsError OsFileMove( const char_t *oldFileName ,const char_t *newFileName ); //! can also be used to rename a file
TINYFUN OsError OsFileDelete( const char_t *fileName );

//////////////////////////////////////////////////////////////////////////
TINYFUN OsError OsFileCreateDir( const char_t *path );
TINYFUN OsError OsFileRemoveDir( const char_t *path );
//+ get current dir

//////////////////////////////////////////////////////////////////////////
//! file (find)

struct OsFileFindFile
{
	void *handle;

	struct OsFileFileInfo _info;
};

TINYFUN OsError OsFileFindFirst( const char_t *filemask ,struct OsFileFindFile *findfile );
TINYFUN OsError OsFileFindNext( struct OsFileFindFile *findfile );
TINYFUN OsError OsFileFindClose( struct OsFileFindFile *findfile );

//////////////////////////////////////////////////////////////////////////
//! file 

#define OS_ACCESS_LIST		1
#define OS_ACCESS_READ		2
#define OS_ACCESS_WRITE		4
#define OS_ACCESS_EXECUTE	8
#define OS_ACCESS_ALL		15

#define OS_SHARE_NONE		0
#define OS_SHARE_READ		1
#define OS_SHARE_WRITE		2
#define OS_SHARE_DELETE		4
#define OS_SHARE_ALL		7

#define OS_CREATE_DONT		0		//! fail if doesn't exist, open if exist
#define OS_CREATE_NOEXIST	1		//! create if doesn't exist, open if exist
#define OS_CREATE_NEW		3		//! create if doesn't exist, fail if exist
#define OS_CREATE_TRUNCATE	4		//! fail if doesn't exist, truncate if exist
#define OS_CREATE_ALWAYS	5		//! create if doesn't exist, truncate if exist

#define OS_SEEK_BEGIN		0
#define OS_SEEK_CURPOS		1
#define OS_SEEK_END			2

TINYFUN OsError OsFileOpen( OsHandle *handle ,const char_t *filename ,int access ,int share ,int create ,struct OsFileAttributes *attributes );
TINYFUN OsError OsFileRead( OsHandle handle ,uint8_t *memory ,size_t size ,size_t *readSize );
TINYFUN OsError OsFileWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,size_t *writeSize );
TINYFUN OsError OsFileFlush( OsHandle handle );
TINYFUN OsError OsFileGetSize( OsHandle handle ,uint64_t *size );
TINYFUN OsError OsFileSetSize( OsHandle handle ,uint64_t size );
TINYFUN OsError OsFileSeek( OsHandle handle ,size_t distance ,int seekpos );
TINYFUN OsError OsFileTell( OsHandle handle ,size_t *position );

//////////////////////////////////////////////////////////////////////////
//! network

//? TODO enum local LAN address ?

//-- server
TINYFUN OsError OsNetListen( OsHandle *handle ,const char_t *localAddress );
TINYFUN OsError OsNetAccept( OsHandle listenHandle ,OsHandle *handle ,int32_t timeoutMs );
//? TODO get connection info

//-- client
TINYFUN OsError OsNetConnect( OsHandle *handle ,const char_t *peerAddress ,const char *localAddress ,int32_t timeoutMs );

//-- option
#define OS_NETPARAM_READTIMEOUT		1
#define OS_NETPARAM_WRITETIMEOUT	2
//TODO

TINYFUN OsError OsNetConnectionSetParam( OsHandle handle ,int paramId ,void *value );
TINYFUN OsError OsNetConnectionGetInfo( OsHandle handle ,int info );

//-- operation
TINYFUN OsError OsNetRead( OsHandle handle ,uint8_t *memory ,size_t size ,size_t *readSize ,int32_t timeoutMs ); //TODO remove timeoutMs (in SetParam)
TINYFUN OsError OsNetWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,int32_t timeoutMs );

//? TODO flush

//////////////////////////////////////////////////////////////////////////
//? TODO

// (asynch comm for file / net)
// TINYFUN OsError OsDataAsynch( OsHandle *handle ,OsEventFunction handler ,void *userData );

//////////////////////////////////////////////////////////////////////////
//? TODO
// TINYFUN OsError OsDataRead( OsHandle handle ,uint8_t *memory ,size_t size );
// TINYFUN OsError OsDataWrite( OsHandle handle ,const uint8_t *memory ,size_t size );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! GUI

//////////////////////////////////////////////////////////////////////////
#define OS_MAX_MOUSEPOS		8

typedef void* OsGuiContext; //! got from a render event

enum OsEventType { osNoEvent=0 ,osRenderEvent ,osMouseEvent ,osKeyboardEvent ,osExecuteEvent ,osConnectEvent ,osDataEvent ,osTimerEvent ,osUserEvent=256 };
enum OsKeyAction { osNoKeyAction=0 ,osKeyDown ,osKeyUp ,osKeyChar };
enum OsKeyMode { osKeyModeInsert ,osKeyModeOverride };

struct OsKeyState {
	uint32_t shift : 1;
	uint32_t ctrl : 1;
	uint32_t alt : 1;
	//...

} static const _noKeyState = {0,0,0};

inline int keyStateValue( struct OsKeyState s ) {
    return s.shift | (s.ctrl << 1) | (s.alt << 2);
}

typedef unsigned int OsKeyCode;

enum OsMouseButton {
    osNoMouseButton=0
	,osLeftMouseButton=1 ,osMiddleMouseButton=2 ,osRightMouseButton=4 
	,osMoreMouseButton1=8 
	
} static const _noMouseButton = osNoMouseButton;

enum OsMouseAction {
    osNoMouseAction=0
//-- OS mouse action
    ,osMouseMove ,osMouseButtonDown ,osMouseButtonUp ,osMouseDoubleClick
//-- User mouse action (not sent by OS to window function
	,osMouseHit ,osMouseEnter ,osMouseLeave ,osMouseClick ,osMouseWheelUp ,osMouseWheelDown
	,osMouseGrab ,osMouseDrag ,osMouseDrop ,osMouseCancel //! source side drag&drop message
	,osMousePreview ,osMouseAccept //! target side drag&drop message
    ,osMouseCaptureLost //! capture
};

enum OsExecuteAction { //! AKA : "Create ,Close ,Destroy" for objects
    osNoExecuteAction=0
    ,osExecuteStart ,osExecuteStop ,osExecuteTerminate
};

enum OsTimerAction {
    osNoTimerAction=0
    ,osTimerGlobal=1
};

typedef unsigned int OsEventTime; //! ms

#define osExecuteCreate osExecuteStart
#define osExecuteClose osExecuteStop
#define osExecuteDestroy osExecuteTerminate

struct OsRenderEventMessage { OsGuiContext context; struct OsRect *updateRect; int resized; };
struct OsMouseEventMessage { struct OsKeyState keyState; enum OsMouseButton mouseButton; enum OsMouseAction mouseAction; int points; struct OsPoint *pos; };
struct OsKeyboardEventMessage { struct OsKeyState keyState; enum OsMouseButton mouseButton; enum OsKeyAction keyAction; OsKeyCode keyCode; char_t c; };
struct OsExecuteEventMessage { enum OsExecuteAction executeAction; }; //! execution started (EG form thread)
struct OsDataEventMessage { void *_none; }; //! data was received (EG from network connection)
struct OsTimerEventMessage { enum OsTimerAction timerAction; OsEventTime now; OsEventTime last; };

struct OsEventMessage
{
	enum OsEventType eventType;

	int64_t t ,dt;

	union { 
		struct OsExecuteEventMessage executeMessage;
		struct OsRenderEventMessage renderMessage;
		struct OsMouseEventMessage mouseMessage;
		struct OsKeyboardEventMessage keyboardMessage;
        struct OsTimerEventMessage timerMessage;
	};
};

//////////////////////////////////////////////////////////////////////////
#define OS_KEYCODE_GOTFOCUS		((OsKeyCode) -16)
#define OS_KEYCODE_LOSTFOCUS	((OsKeyCode) -17)

#define OS_KEYCODE_BACKSPACE	8
#define OS_KEYCODE_TAB			9
#define OS_KEYCODE_ENTER		13
#define OS_KEYCODE_RETURN		13
#define OS_KEYCODE_ESCAPE		27
#define OS_KEYCODE_SPACE		32
#define OS_KEYCODE_END			35
#define OS_KEYCODE_HOME			36
#define OS_KEYCODE_LEFT			37
#define OS_KEYCODE_UP			38
#define OS_KEYCODE_RIGHT		39
#define OS_KEYCODE_DOWN			40
#define OS_KEYCODE_INSERT		45
#define OS_KEYCODE_DELETE		46
#define OS_KEYCODE_F(_x_)       (66+_x_)

//////////////////////////////////////////////////////////////////////////
//! colors

#define OS_RGB(r,g,b)		((OsColorRef) (((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16))|(((uint32_t)0x0ff)<<24) )
#define OS_RGBA(r,g,b,a)	((OsColorRef) (((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16))|(((uint32_t)(uint8_t)(a))<<24) )

#define OS_RGB_RED(c)		((OsColorRef) ((uint8_t)(c)) )
#define OS_RGB_GREEN(c)		((OsColorRef) ((uint8_t)((uint16_t)(c)>>8)) )
#define OS_RGB_BLUE(c)		((OsColorRef) ((uint8_t)((uint32_t)(c)>>16)) )
#define OS_RGB_ALPHA(c)		((OsColorRef) ((uint8_t)((uint32_t)(c)>>24)) )

#define OS_BGR2RGB(c)		OS_RGBA(OS_RGB_BLUE(c),OS_RGB_GREEN(c),OS_RGB_RED(c),OS_RGB_ALPHA(c))

#define OS_NOCOLOR          ((uint32_t)0x000000000)
#define OS_HASCOLOR(c)      (OS_RGB_ALPHA(c) > 0)

#define OS_COLOR_NONE           ((uint32_t)0x000000000)
#define OS_COLOR_TRANSPARENT	((uint32_t)0x000000000)
#define OS_COLOR_OPAQUE			((uint32_t)0x0ff000000)

#define OS_COLOR_SPECIAL16		((uint32_t)0x000cd0000) //! a transparent color used to hold a 16bit value
#define OS_COLOR_SPECIAL8		((uint32_t)0x000cefe00) //! a transparent color used to hold a 8bit value

#define OS_COLOR_WHITE			((uint32_t)0x0ffffffff)
#define OS_COLOR_RED			((uint32_t)0x0ff0000ff) //! simple colors
#define OS_COLOR_GREEN			((uint32_t)0x0ff00ff00)
#define OS_COLOR_BLUE			((uint32_t)0x0ffff0000)

#define OS_COLOR_YELLOW			((uint32_t)0x0ff00ffff) //! simple 2 pure colors
#define OS_COLOR_MAGENTA        ((uint32_t)0x0ffff00ff)
#define OS_COLOR_CYAN           ((uint32_t)0x0ffffff00)

//-- red
#define OS_COLOR_RUBY           ((uint32_t)0x0ff1e119b)
#define OS_COLOR_CARMINE        ((uint32_t)0x0ff180096)
#define OS_COLOR_SALMON         ((uint32_t)0x0ff7280fa)
#define OS_COLOR_CRIMSON        ((uint32_t)0x0ff3c14dc)
#define OS_COLOR_SANGRIA        ((uint32_t)0x0ff0a0092)
#define OS_COLOR_BURGUNDY       ((uint32_t)0x0ff200080)
#define OS_COLOR_ROSEWOOD       ((uint32_t)0x0ff0b0065)

#define OS_COLOR_PURPLE         ((uint32_t)0x0ff800080) //! complex colors
#define OS_COLOR_OLIVE          ((uint32_t)0x0ff008080)
#define OS_COLOR_ORANGE         ((uint32_t)0x0ff0080ff)
#define OS_COLOR_PINK           ((uint32_t)0x0ffff80ff)

//-- dark
#define OS_COLOR_DARKRED		((uint32_t)0x0ff000080)
#define OS_COLOR_DARKGREEN		((uint32_t)0x0ff008000)
#define OS_COLOR_DARKBLUE		((uint32_t)0x0ff800000)

//-- light
#define OS_COLOR_LIGHTRED		((uint32_t)0x0ff0000c0)
#define OS_COLOR_LIGHTGREEN		((uint32_t)0x0ff00c000)
#define OS_COLOR_LIGHTBLUE		((uint32_t)0x0ffc00000)

//-- shades
#define OS_COLOR_GRAY			((uint32_t)0x0ff808080)
#define OS_COLOR_LIGHTGRAY		((uint32_t)0x0ffc0c0c0)
#define OS_COLOR_DARKGRAY       ((uint32_t)0x0ff404040)
#define OS_COLOR_BLACK			((uint32_t)0x0ff000000)

#define OS_SELECT_FORECOLOR		1
#define OS_SELECT_FILLCOLOR		4
#define OS_SELECT_TEXTCOLOR		3
#define OS_SELECT_BACKCOLOR		2

//////////////////////////////////////////////////////////////////////////
//! Text windowing

struct OsConsoleInfo
{
	int	width ,height;
	int posx, posy;

	OsColorRef forecolor;
	OsColorRef backcolor;
};

//! NB: a process can only have one console, use GUI window with OS_GUIWINDOW_CONSOLE
//   for a portable way to use more than one console per process

TINYFUN OsError OsConsoleWindowCreate( const char_t *name );
TINYFUN OsError OsConsoleWindowClose( void );
TINYFUN OsError OsConsoleGetInfo( struct OsConsoleInfo *info );
TINYFUN void OsConsoleSetPosition( int x ,int y );
TINYFUN void OsConsoleSetColor( int selectColor ,OsColorRef color );
TINYFUN void OsConsoleTextOut( const char_t *string ,int length );
TINYFUN void OsConsoleTextGet( char_t *string ,int *length ,int maxchar );

//??
// TINYFUN void OsGuiConsoleTextGet( OsHandle handle, char_t *string ,int maxchar );
// TINYFUN void OsGuiConsoleTextOut( OsHandle handle, const char_t *string ,int length );

//////////////////////////////////////////////////////////////////////////
//! Native dialogs & controls

// TODO check common features with linux
#define OS_GUIMSGBOX_OK				1
#define OS_GUIMSGBOX_OKCANCEL		2
#define OS_GUIMSGBOX_RETRYCANCEL	3
#define OS_GUIMSGBOX_YESNO			4
#define OS_GUIMSGBOX_YESNOCANCEL	5
#define OS_GUIMSGBOX_CANCELRETRYCONTINUE 6
#define OS_GUIMSGBOX_HELP 7

#define OS_GUIMSGBOX_OKBUTTON 1
#define OS_GUIMSGBOX_CANCELBUTTON 2
#define OS_GUIMSGBOX_ABORTBUTTON 3
#define OS_GUIMSGBOX_CONTINUEBUTTON 4
#define OS_GUIMSGBOX_IGNOREBUTTON 5
#define OS_GUIMSGBOX_YESBUTTON 6
#define OS_GUIMSGBOX_NOBUTTON 7
#define OS_GUIMSGBOX_RETRYBUTTON 8
#define OS_GUIMSGBOX_HELPBUTTON 9

#define OS_GUIMSGBOX_ICONEXCLAMATION 1
#define OS_GUIMSGBOX_ICONWARNING 2
#define OS_GUIMSGBOX_ICONINFORMATION 3
#define OS_GUIMSGBOX_ICONASTERISK 4
#define OS_GUIMSGBOX_ICONQUESTION 5
#define OS_GUIMSGBOX_ICONSTOP 6
#define OS_GUIMSGBOX_ICONERROR 7
#define OS_GUIMSGBOX_ICONHAND 8

#define OS_GUIMSGBOX_CONFIG3(type, default_button, icon) type + (default_button << 4 ) + (icon << 8)

TINYFUN int OsDialogNativeMessageBox( OsHandle owner ,const char_t *text ,const char_t *caption , int type ,uint16_t flags );


#define OS_GUIDIALOG_FILEOPEN			1
#define OS_GUIDIALOG_FILESAVE			2

#define OS_GUIDIALOG_FILEPROMPTCREATE	0x10
#define OS_GUIDIALOG_FILEMULTISELECT	0x11

TINYFUN OsError OsDialogNativeFileSelect( OsHandle owner ,const char_t *directory ,const char_t *filters ,int defaultFilter ,char_t *filenames ,int length ,uint16_t flags );

//? FilePrint / Preview

//? Show dialog loaded from load resource handle

//? Add menu to a window from load resource handle

//////////////////////////////////////////////////////////////////////////
//! Graphic system

#define OS_GUISYSTEMID_CURRENT		0

#define OS_GUISYSTEMID_NATIVE		1 //! GDI / X11
#define OS_GUISYSTEMID_OPENGL		2
#define OS_GUISYSTEMID_OPENGLES		3
#define OS_GUISYSTEMID_DIRECTX		4
//! SDL

struct OsGuiSystemTable;

TINYFUN OsError OsGuiSetCurrentSystem( int guiSystemId );
TINYFUN OsError OsGuiGetCurrentSystem( int *guiSystemId );
TINYFUN OsError OsGuiGetSystemTable( int guiSystemId ,struct OsGuiSystemTable **guiSystemTable );

//! font and image created using C interface will be created for the current system
//  use systemTable to use mixed systems or set current system as required

//////////////////////////////////////////////////////////////////////////
//! Graphic windowing

#define OS_WINDOWSTYLE_NORMAL		0
#define OS_WINDOWSTYLE_DIALOG		1
#define OS_WINDOWSTYLE_TOOLBOX		2
#define OS_WINDOWSTYLE_CONSOLE		3
#define OS_WINDOWSTYLE_FULLSCREEN   4
#define OS_WINDOWSTYLE_TRANSPARENT  8

#define OS_WINDOWSTYLE_SIZEABLE		32
#define OS_WINDOWSTYLE_HIDDEN		64 //TODO
#define OS_WINDOWSTYLE_HSCROLL		128
#define OS_WINDOWSTYLE_VSCROLL		256

#define OS_WINDOWSTYLE_ONTOP		512

#define OS_WINDOWFLAG_NORMAL		0
#define OS_WINDOWFLAG_DIRECTDRAW	1 //! IE default windows are double buffered
#define OS_WINDOWFLAG_IMGUI			2

struct OsGuiWindowProperties
{
	const char_t *title;

	int defaultWidth ,defaultHeight;

	int style;

	int flags;

	OsColorRef backgroundColor;
};

TINYFUN OsError OsGuiWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData );
//TODO name window return already created window if they exist (+ saved pos ?)

//////////////////////////////////////////////////////////////////////////
//-- GUI common

//-- visibility
#define OS_REFRESH_NORMAL		0 //! window will be updated with next redraw
#define OS_REFRESH_RESIZED		1 //! window size or layout (IE child view) was modified
#define OS_REFRESH_CONTENT		2 //! window content was modified (NB user information)
#define OS_REFRESH_BACKGROUND	4 //! window background should be updated (repainted)
#define OS_REFRESH_UPDATE		128 //! window should be updated (redrawn) asap

TINYFUN void OsGuiWindowRefresh( OsHandle handle ,const struct OsRect *updateRect ,int flags );

#define OS_VISIBLE_HIDDEN		0
#define OS_VISIBLE_NORMAL		1

TINYFUN void OsGuiWindowShow( OsHandle handle ,int visible );

//-- input
#define OS_CURSOR_NOCURSOR		0
#define OS_CURSOR_ARROW			1
#define OS_CURSOR_WAIT			2
#define OS_CURSOR_CROSS			3
#define OS_CURSOR_BEAM			40
#define OS_CURSOR_CANNOT		41 // slashed circle
#define OS_CURSOR_UPARROW		4 //? beam
#define OS_CURSOR_SIZEALL		5
#define OS_CURSOR_SIZETOPLEFT	6
#define OS_CURSOR_SIZETOPRIGHT	7
#define OS_CURSOR_SIZEWIDTH		8
#define OS_CURSOR_SIZEHEIGHT	9

TINYFUN void OsGuiMouseSetCursor( OsHandle handle ,int cursorId );
TINYFUN void OsGuiMouseUseCursor( OsHandle handle ,OsHandle cursorHandle );
TINYFUN void OsGuiMouseCapture( OsHandle handle );
TINYFUN void OsGuiMouseRelease( OsHandle handle );

//-- properties
TINYFUN void OsGuiGetClientArea( OsHandle handle ,struct OsPoint *size );

//////////////////////////////////////////////////////////////////////////
//-- gui context

#define OS_SURFACE_DESKTOP		0
#define OS_SURFACE_SCREEN		1
#define OS_SURFACE_WINDOW		2
#define OS_SURFACE_CLIENT		3
#define OS_SURFACE_REGION		4

//> color
TINYFUN void OsGuiSetColor( OsGuiContext context ,int selectColor ,OsColorRef color );

//> region (clipping)
TINYFUN void OsGuiRegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset ); 
//! in OS_SURFACE_CLIENT coordinates 
TINYFUN void OsGuiRegionGetArea( OsGuiContext context ,struct OsRect *area );
TINYFUN void OsGuiRegionSetOffset( OsGuiContext context ,int x ,int y ); //! translation
TINYFUN void OsGuiRegionSetScale( OsGuiContext context ,float x ,float y ); //! scaling
TINYFUN void OsGuiPointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords );

//-- font
#define OS_FONTWEIGHT_ANY			0
#define OS_FONTWEIGHT_THIN			1
#define OS_FONTWEIGHT_ULTRALIGHT	2
#define OS_FONTWEIGHT_LIGHT			3
#define OS_FONTWEIGHT_NORMAL		4
#define OS_FONTWEIGHT_MEDIUM		5
#define OS_FONTWEIGHT_SEMIBOLD		6
#define OS_FONTWEIGHT_BOLD			7
#define OS_FONTWEIGHT_ULTRABOLD		8
#define OS_FONTWEIGHT_HEAVY			9

#define OS_FONTSTYLE_NORMAL			0
#define OS_FONTSTYLE_ITALIC			1
#define OS_FONTSTYLE_UNDERLINE		2
#define OS_FONTSTYLE_STRIKEOUT		4

#define OS_FONTPITCH_ANY			0
#define OS_FONTPITCH_FIXED			1
#define OS_FONTPITCH_VARIABLE		2

TINYFUN OsError OsGuiFontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int pitch );
TINYFUN void OsGuiFontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size );
TINYFUN void OsGuiSetFont( OsGuiContext context ,OsHandle fontHandle );

//-- image 
#define OS_IMAGE_USEDIM			-1 //?

#define OS_IMAGE_RGB24			1 //? pixel format

struct OsGuiImageInfo
{
	int width;

	int height;

	int pixelFormat;
};

TINYFUN OsError OsGuiImageCreate( OsHandle *handle ,int width ,int height );
TINYFUN void OsGuiImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info );
TINYFUN OsError OsGuiImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size );
TINYFUN void OsGuiSetAlphaBlend( OsGuiContext context ,float alpha );

//-- render
TINYFUN void OsGuiDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom );
TINYFUN void OsGuiDrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom );
TINYFUN void OsGuiDrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom );
TINYFUN void OsGuiDrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom );
TINYFUN void OsGuiDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points );
//? draw shaded rectangle + access image pixel

#define OS_ALIGN_NORMAL			0 //! top|left
#define OS_ALIGN_LEFT			0
#define OS_ALIGN_CENTERH		1
#define OS_ALIGN_RIGHT			2
#define OS_ALIGN_TOP			0
#define OS_ALIGN_CENTERV		4
#define OS_ALIGN_BOTTOM			8
#define OS_ALIGN_CENTER         (OS_ALIGN_CENTERH|OS_ALIGN_CENTERV)

TINYFUN void OsGuiDrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect );

//-- resource
#define OS_GUIRESOURCETYPE_ANY			0
#define OS_GUIRESOURCETYPE_CURSOR		1
#define OS_GUIRESOURCETYPE_ICON			2 //! incl. animated if supported
#define OS_GUIRESOURCETYPE_IMAGE		3
#define OS_GUIRESOURCETYPE_DIALOG		4
#define OS_GUIRESOURCETYPE_FONT			5
#define OS_GUIRESOURCETYPE_TEXTDOC		6 //! text document, eg. html/xml
#define OS_GUIRESOURCETYPE_MENU			7
#define OS_GUIRESOURCETYPE_STRINGS		8 //! string table

//!NB resource generic function only useful to load image or font at the moment

TINYFUN int OsGuiResourceGetType( OsHandle handle );

//! resourceType parameters provide hint for loading the resource
//  which might be mandatory for some system/resource

TINYFUN OsError OsGuiResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo );
TINYFUN OsError OsGuiResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename );
TINYFUN OsError OsGuiResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct OsGuiSystemTable
{
	OsError (*_WindowCreate)( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData );
	void (*_WindowRefresh)( OsHandle handle ,const struct OsRect *updateRect ,int flags );
	OsError (*_WindowSwapBuffers)( OsHandle *handle );
	void (*_WindowShow)( OsHandle handle ,int visible );

	void (*_MouseSetCursor)( OsHandle handle ,int cursorId );
	void (*_MouseCapture)( OsHandle handle );
	void (*_MouseRelease)( OsHandle handle );

	void (*_GetClientArea)( OsHandle handle ,struct OsPoint *size );
	void (*_SetColor)( OsGuiContext context ,int selectColor ,OsColorRef color );
	void (*_RegionSetArea)( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset ); 
	void (*_RegionGetArea)( OsGuiContext context ,struct OsRect *area );
    //! TODO: GetOffset + GetScale => RegionGetTransform ... => 3D ???
	void (*_RegionSetOffset)( OsGuiContext context ,int x ,int y );
	void (*_RegionSetScale)( OsGuiContext context ,float x ,float y );
	void (*_RegionGetSize)( OsGuiContext context ,int *width ,int *height );
	void (*_PointToCoords)( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords );
	
	OsError (*_FontCreate)( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int pitch );
	void (*_FontCalcSize)( OsHandle handle ,const char_t *text ,struct OsPoint *size );
	void (*_SetFont)( OsGuiContext context ,OsHandle fontHandle );

	OsError (*_ImageCreate)( OsHandle *handle ,int width ,int height );
	void (*_ImageGetInfo)( OsHandle handle ,struct OsGuiImageInfo *info );
	OsError (*_ImageWrite)( OsHandle handle ,const uint8_t *pixelValues ,size_t size );

    //TODO: get/set draw style ..
	void (*_SetAlphaBlend)( OsGuiContext context ,float alpha );
	void (*_DrawImage)( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom );
	void (*_DrawRectangle)( OsGuiContext context ,int left ,int top ,int right ,int bottom );
	void (*_DrawEllipse)( OsGuiContext context ,int left ,int top ,int right ,int bottom );
	void (*_DrawLine)( OsGuiContext context ,int left ,int top ,int right ,int bottom );
	void (*_DrawText)( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect );
	void (*_DrawPolygon)( OsGuiContext context ,int npoints ,const struct OsPoint *points ); //TODO up one

	int (*_ResourceGetType)( OsHandle handle );
	OsError (*_ResourceLoadFromMemory)( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo );
	OsError (*_ResourceLoadFromFile)( OsHandle *handle ,int resourceTypeHint ,const char_t *filename );
	OsError (*_ResourceLoadFromApp)( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application );
	enum OsHandleType (*_HandleGetType)( OsHandle handle );
	OsError (*_HandleDestroy)( OsHandle *handle );
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#endif // _TINY_OS_H_
//