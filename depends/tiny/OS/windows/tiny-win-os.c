/****************************************************** //
//               tiny-for-c v3 library                  //
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
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <windowsx.h>

#include <winsock2.h>

#include "timezoneapi.h"
#include "corecrt_wtime.h"

//////////////////////////////////////////////////////////////////////////
#pragma comment( lib ,"msimg32.lib" ) //! for ALPHABLEND

#pragma comment( lib ,"ws2_32.lib" ) //! for NETWORK

#pragma warning( disable : 4996 ) //! deprecation

//////////////////////////////////////////////////////////////////////////
#include <intrin.h>
#include <crtdbg.h>

//////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

//////////////////////////////////////////////////////////////////////////
#include "tiny-os.h"

#include "tiny-win-defs.h"

//////////////////////////////////////////////////////////////////////////
//! Tool functions
int32_t log2i( int32_t x )
{
	unsigned long i;

	if( !_BitScanReverse( &i, x ) ) return 0;

	return i;
}

int32_t log2ceil( int32_t x )
{
	return log2i(x) + (x&(x-1)?1:0);
}

//////////////////////////////////////////////////////////////////////////
const char_t *OsErrorGetText( OsError error )
{
	return (char_t*) _tcserror( error );
}

OsError HResultToError( HRESULT hr )
{
	switch( hr )
	{
	//TODO complete

	case NO_ERROR: return ENOERROR;

	case E_INVALIDARG: return EINVAL;
	case E_OUTOFMEMORY: return ENOMEM;
	case E_NOTIMPL: return ENOSYS;

	default:
		return EFAILED;
	}
}

OsError GetLastOsError( void ) { return HResultToError( GetLastError() ); }

//////////////////////////////////////////////////////////////////////////
static void cpuID( unsigned int i ,unsigned regs[4] )
{
#ifdef _WIN32
	__cpuid( (int*) regs ,(int) i );

#else
	asm volatile
		("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
		: "a" (i), "c" (0));
	// ECX is set to zero for CPUID function 4
#endif
}

OsError OsSystemGetInfo( struct OsSystemInfo *info )
{
	SYSTEM_INFO sysinfo;

	unsigned int regs[4];

	if( info == NULL ) return EINVAL;

	//-- get CPU info
	GetSystemInfo( &sysinfo );

	info->_processorCount = sysinfo.dwNumberOfProcessors;
	info->_logicalCoreCount = sysinfo.dwNumberOfProcessors;

	//-- get CPU features
	cpuID( 1 ,regs );

	if( (regs[3] & (1 << 28)) != 0 ) //! regs[3] = EDX = cup features
		info->_physicalCoreCount = info->_logicalCoreCount >> 1;
	else
		info->_physicalCoreCount = info->_logicalCoreCount;

	return ENOERROR;
}

OsError OsSystemSetGlobalTimer(uint32_t ms)
{
	//SetTimer(NULL, ms, NULL, NULL);
	return ENOERROR;
}


OsError OsSystemDoEvents( void )
{
	MSG msg;

	while( PeekMessage( &msg ,NULL ,0 ,0 ,PM_REMOVE ) > 0 )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );

		if( msg.message == WM_QUIT ) return EINTR;
	}

	return ENOERROR;
}

OsError OsSystemPostQuit( uint32_t exitCode )
{
	PostQuitMessage( exitCode );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
OsError OsMemoryGetInfo( struct OsMemoryInfo *info )
{
	MEMORYSTATUSEX memInfo;

	memInfo.dwLength = sizeof(memInfo);

	GlobalMemoryStatusEx( &memInfo );

	info->_memoryLoad = memInfo.dwMemoryLoad;
	info->_memoryPhysicalTotal = memInfo.ullTotalPhys;
	info->_memoryPhysicalAvail = memInfo.ullAvailPhys;
	info->_memoryVirtualTotal = memInfo.ullTotalVirtual;
	info->_memoryVirtualAvail = memInfo.ullAvailVirtual;

	return ENOERROR;
}

size_t __align_size( size_t size ,size_t pow2align )
{
	size_t align = ((size_t)1) << pow2align;

	size_t mod = size % align;

	return mod ? (size + align - mod) : size;
}

void *__align_ptr( void *ptr ,size_t pow2align ,size_t headerSize )
{
	size_t align = ((size_t) 1) << pow2align;

	void *p;

	if( ptr == NULL ) return NULL;

	if( align <= 1 ) return (void*) ((uint8_t*) ptr + headerSize);

	p = (void*) ((uintptr_t) ((uint8_t*) ptr + headerSize + align-1) & ~(align-1));

	return p;
}

//! memory block header are injected before allocation to provide auto type feature
//! header data is sizeof(MemBlockHeader) before the actual memory pointer 
struct MemBlockHeader
{
	void *_baseptr; //! allocation ptr (may have been moved around for alignment)
};

void *OsMemoryAlloc( size_t size ,int flags )
{
	size_t pow2align = MAX( (flags & 0x0ff) ,1 );
	size_t align = ((size_t)1) << pow2align;

	size_t headerSize = sizeof(struct MemBlockHeader) ,allocSize;

	struct MemBlockHeader *header;

	void *baseptr ,*alignptr;

	if( size == 0 ) return NULL;

	allocSize = headerSize + size + (align-1);

	baseptr = malloc( allocSize );
	 
	if( baseptr == NULL ) return NULL;

	alignptr = __align_ptr( baseptr ,pow2align ,headerSize );

	header = (struct MemBlockHeader *) ((uint8_t*) alignptr - headerSize);

	header->_baseptr = baseptr;

	if( flags & OS_MEMORY_ZERO )
		memset( alignptr ,0 ,size );

	return alignptr;
}

void *OsMemoryRealloc( void *memory ,size_t size ,int flags )
{
	size_t pow2align = MAX( (flags & 0x0ff) ,1 );
	size_t align = ((size_t)1) << pow2align; //TODO check if we need -1 ?

	size_t headerSize = sizeof(struct MemBlockHeader) ,allocSize;

	struct MemBlockHeader *header;

	void *baseptr,*alignptr;

	if( memory == NULL ) return NULL;

	header = (struct MemBlockHeader *) ((uint8_t*) memory - headerSize);

	allocSize = size + headerSize + align-1;

	if( size == 0 ) 
	{
		free( header->_baseptr );

		return NULL;
	}

	baseptr = realloc( header->_baseptr ,allocSize );

	if (baseptr == NULL ) return NULL;

	alignptr = __align_ptr(baseptr, pow2align ,headerSize );

	header = (struct MemBlockHeader *) ((uint8_t*) alignptr - headerSize);

	header->_baseptr = baseptr;

	if (flags & OS_MEMORY_ZERO)
		memset(alignptr, 0, size);

	return alignptr;
}

void OsMemoryFree( void *memory )
{
	struct MemBlockHeader *header;

	if( memory == NULL ) return;
	
	header = (struct MemBlockHeader *) ((uint8_t*) memory - sizeof(struct MemBlockHeader));

	free( header->_baseptr );
}

//////////////////////////////////////////////////////////////////////////
struct win_handle
{
	uint32_t _magic;

	HANDLE _handle;
};

//! works for any OsHandle type relying on a window HANDLE 
//  (keep "HANDLE _handle;" the first member for all handle struct below)
OsError win_closehandle( OsHandle *handle )
{
	struct win_handle *p = * (struct win_handle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_handle != INVALID_HANDLE_VALUE ) CloseHandle( p->_handle );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Threading

/*
//////////////////////////////////////////////////////////////////////////
//-- asynch
#define _CACHE_LINE_SIZE			64

#define __alignCache				__declspec(align(_CACHE_LINE_SIZE))
#define CACHE_PAD(_n_,_x_)			char _pad##_n_[_CACHE_LINE_SIZE - (sizeof(_x_)%_CACHE_LINE_SIZE)]

#ifdef PLATFORM_64BIT
 #define __align					__declspec (align(8))
 #define _ASYNCH_TYPE				unsigned __int64
 #define _ASYNCH_SIGNEDTYPE			signed __int64
 #define _ASYNCH_VARIABLE			__declspec (align(8)) volatile unsigned __int64

 #define _ASYNCH_ADD(__x,__y)		InterlockedAdd64( (LONGLONG*) &__x,__y)	//! returns the new added value
#else
 #define __align					__declspec (align(4))
 #define _ASYNCH_TYPE				unsigned long
 #define _ASYNCH_SIGNEDTYPE			signed long
 #define _ASYNCH_VARIABLE			__declspec (align(4)) volatile unsigned long

 #define _ASYNCH_ADD(__x,__y)		InterlockedAdd( (LONG*) &__x,__y)		//! returns the new added value
#endif

#define _ASYNCH_READ(__x)			InterlockedExchangeAdd((volatile LONG*)&__x,0)	//! returns the variable (initial) value
#define _ASYNCH_WRITE(__x,__y)		InterlockedExchange((volatile LONG*)&__x,__y)	//! returns the variable (initial) value
#define _ASYNCH_INCREMENT(__x)		InterlockedIncrement((volatile LONG*)&__x)		//! returns the new incremented value
#define _ASYNCH_DECREMENT(__x)		InterlockedDecrement((volatile LONG*)&__x)		//! returns the new decremented value
*/

//////////////////////////////////////////////////////////////////////////
//-- process
#define PROCESSHANDLE_MAGIC	0x0A326FB7E

struct ProcessHandle
{
    uint32_t _magic;

    HANDLE _handle;
};

static struct ProcessHandle *NewProcessHandle()
{
    const size_t size = sizeof(struct ProcessHandle);

    struct ProcessHandle *p = (struct ProcessHandle*) malloc(size);

    if( p == NULL ) return NULL;

    memset( p ,0 ,size );

    p->_magic = PROCESSHANDLE_MAGIC;

    p->_handle = 0;

    return p;
}

static struct ProcessHandle *CastProcessHandle( OsHandle handle )
{
    struct ProcessHandle *p = (struct ProcessHandle *) handle;

    if( p == NULL ) return NULL;

    if( p->_magic != PROCESSHANDLE_MAGIC ) return NULL;

    return p;
}

//-- process functions
OsError OsProcessRun( OsHandle *handle ,const char *path ,const char *argv[] ,const char *envp[] )
{
    struct ProcessHandle *p = NULL;

    if( handle == NULL ) return EINVAL;

    if( *handle != OS_INVALID_HANDLE ) return EEXIST;

    p = NewProcessHandle();

    if( p == NULL ) return ENOMEM;

    *handle = (OsHandle) p;

    return ENOERROR;
}

OsError OsProcessKill( OsHandle *handle ,int exitCode ) {
    struct ProcessHandle *p = CastProcessHandle( handle );

    if( p == NULL ) return EINVAL;

    if( p->_handle == 0 ) return EINVAL;

    TerminateProcess( p->_handle ,0 );

    return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
//-- thread
#define THREADHANDLE_MAGIC	0x02F5ACC15

struct ThreadHandle
{
	uint32_t _magic;

	HANDLE _handle;

	DWORD _id;

	OsEventFunction _function;

	void *_userData;
};

static struct ThreadHandle *NewThreadHandle( OsEventFunction function ,void *userData )
{
	const size_t size = sizeof(struct ThreadHandle);

	struct ThreadHandle *p = (struct ThreadHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = THREADHANDLE_MAGIC;

	p->_handle = INVALID_HANDLE_VALUE;

	p->_function = function;

	p->_userData = userData;

	return p;
}

static struct ThreadHandle *CastThreadHandle( OsHandle handle )
{
	struct ThreadHandle *p = (struct ThreadHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != THREADHANDLE_MAGIC ) return NULL;

	return p;
}

//////////////////////////////////////////////////////////////////////////
//> thread callbacks
static OsError CallThreadFunction( struct ThreadHandle *handle ,enum ExecuteAction action )
{
	struct OsEventMessage msg;

	if( handle==NULL ) return (DWORD) EINVAL;

	memset( (void*) &msg ,0 ,sizeof(msg) );

	msg.eventType = osExecuteEvent;

	msg.executeMessage.executeAction = action;

	return (DWORD) handle->_function( &msg ,handle->_userData );
}

static DWORD WINAPI _win_threadproc( void *parameter )
{
	struct ThreadHandle *handle = (struct ThreadHandle*) parameter;

	return (DWORD) CallThreadFunction( handle ,osExecuteStart );
}

//////////////////////////////////////////////////////////////////////////
//> thread functions
OsError OsThreadCreate( OsHandle *handle ,uint32_t createFlags ,OsEventFunction function ,void *userData )
{
	struct ThreadHandle *p = NULL;

	DWORD dwFlags = (createFlags & OS_THREAD_CREATESUSPENDED) ? CREATE_SUSPENDED : 0;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewThreadHandle( function ,userData );

	if( p == NULL ) return ENOMEM;

	p->_handle = CreateThread(
		NULL				// - security attributes
		,0					// - stack size
		,_win_threadproc	// - start routine
		,(void*) p			// - parameters for start routine
		,dwFlags			// - creation flag
		,&(p->_id)				// - thread id
		);

	if( p->_handle == INVALID_HANDLE_VALUE )
	{
		free( (void*) p );

		return GetLastOsError();
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

static int PlatformPriorityMap[] = {
	THREAD_PRIORITY_NORMAL ,THREAD_PRIORITY_IDLE ,THREAD_PRIORITY_LOWEST ,THREAD_PRIORITY_BELOW_NORMAL
	,THREAD_PRIORITY_NORMAL
	,THREAD_PRIORITY_ABOVE_NORMAL ,THREAD_PRIORITY_HIGHEST ,THREAD_PRIORITY_TIME_CRITICAL
};

OsError OsThreadSetPriority( OsHandle handle ,enum OsThreadPriority priority )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	int osPriority = PlatformPriorityMap[priority % (sizeof(PlatformPriorityMap)/sizeof(int)) ];

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( SetThreadPriority( p->_handle ,osPriority ) == TRUE ) return ENOERROR;

	return GetLastOsError();
}

OsError OsThreadSuspend( OsHandle handle )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( SuspendThread( p->_handle ) >= 0 ) return ENOERROR;
	
	return GetLastOsError();
}

OsError OsThreadResume( OsHandle handle )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( ResumeThread( p->_handle ) >= 0 ) return ENOERROR;

	return GetLastOsError();
}

OsError OsThreadTerminate( OsHandle handle ,uint32_t exitCode )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( TerminateThread( p->_handle ,(DWORD) exitCode ) == TRUE )
	{
		CallThreadFunction( p ,osExecuteTerminate );

		return ENOERROR;
	}

	return GetLastOsError(); 
}

OsError OsThreadStop( OsHandle handle ,int32_t timeoutMs )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	return CallThreadFunction( p ,osExecuteStop );
}

OsError OsThreadQuit( OsHandle handle )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( PostThreadMessage( p->_id ,WM_QUIT ,0 ,0 ) == TRUE ) return ENOERROR;

	return GetLastOsError();
}

OsError OsThreadGetExitCode( OsHandle handle ,uint32_t *exitCode )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( GetExitCodeThread( p->_handle ,(DWORD*) exitCode ) == TRUE ) return ENOERROR;
	
	return GetLastOsError();
}

OsError OsThreadGetSelfId( uint32_t *id )
{
	if( id == NULL ) return EINVAL;

	*id = (uint32_t) GetCurrentThreadId();

	return ENOERROR;
}

OsError OsThreadGetId( OsHandle handle ,uint32_t *id )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( id == NULL ) return EINVAL;

	*id = p->_id;

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//-- semaphore
#define SEMAPHOREHANDLE_MAGIC	0x0A91CB4A3

struct SemaphoreHandle
{
	uint32_t _magic;

	HANDLE _handle;

	ASYNCH_VARIABLE _locks; 
	//! NB using a lock before semaphore to avoid unnecessary windows function call (which might measurable overhead)
	//! AKA fast semaphore
};

static struct SemaphoreHandle *NewSemaphoreHandle( void )
{
	const size_t size = sizeof(struct SemaphoreHandle);

	struct SemaphoreHandle *p = (struct SemaphoreHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = SEMAPHOREHANDLE_MAGIC;

	p->_handle = INVALID_HANDLE_VALUE;

	return p;
}

static struct SemaphoreHandle *CastSemaphoreHandle( OsHandle handle )
{
	struct SemaphoreHandle *p = (struct SemaphoreHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != SEMAPHOREHANDLE_MAGIC ) return NULL;

	return p;
}

//////////////////////////////////////////////////////////////////////////
//> semaphore function
OsError OsSemaphoreCreate( OsHandle *handle ,uint32_t initialCount ,const char_t *name )
{
	struct SemaphoreHandle *p = NULL;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewSemaphoreHandle();

	if( p == NULL ) return ENOMEM;

	p->_locks = - (ASYNCH_STYPE) initialCount;

	p->_handle = CreateSemaphore( NULL ,0 ,65535 ,name );

	if( p->_handle == INVALID_HANDLE_VALUE )
	{
		free( (void*) p );

		return GetLastOsError();
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

OsError OsSemaphoreLock( OsHandle handle ,int32_t timeoutMs )
{
	struct SemaphoreHandle *p = CastSemaphoreHandle( handle );

	DWORD dwMillisecond = (timeoutMs < 0) ? INFINITE : timeoutMs;

	DWORD dw = WAIT_OBJECT_0;

	if( p == NULL ) return EINVAL;

	if( (ASYNCH_STYPE) ASYNCH_INCREMENT(p->_locks) > 0 )
		dw = WaitForSingleObject( p->_handle ,timeoutMs );

	return (dw == WAIT_OBJECT_0) ? ENOERROR : (dw == WAIT_TIMEOUT) ? ETIME :  EFAILED;
}

OsError OsSemaphoreUnlock( OsHandle handle )
{
	struct SemaphoreHandle *p = CastSemaphoreHandle( handle );

	DWORD dw = WAIT_OBJECT_0;

	if( p == NULL ) return EINVAL;

	if( (ASYNCH_STYPE) ASYNCH_DECREMENT(p->_locks) >= 0 )
		ReleaseSemaphore( p->_handle ,1 ,NULL );

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//-- critical section
#define CRITICALSECTIONHANDLE_MAGIC	0x035F37CD5

struct CriticalSectionHandle
{
	uint32_t _magic;

	CRITICAL_SECTION _cs;
};

static struct CriticalSectionHandle *NewCriticalSectionHandle( void )
{
	const size_t size = sizeof(struct CriticalSectionHandle);

	struct CriticalSectionHandle *p = (struct CriticalSectionHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = CRITICALSECTIONHANDLE_MAGIC;

	return p;
}

static struct CriticalSectionHandle *CastCriticalSectionHandle( OsHandle handle )
{
	struct CriticalSectionHandle *p = (struct CriticalSectionHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != CRITICALSECTIONHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteCriticalSectionHandle( OsHandle *handle )
{
	struct CriticalSectionHandle *p = (struct CriticalSectionHandle *) *handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL || p->_magic != CRITICALSECTIONHANDLE_MAGIC ) return EINVAL;

	DeleteCriticalSection( &(p->_cs) );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//> critical section function
OsError OsCriticalSectionCreate( OsHandle *handle )
{
	struct CriticalSectionHandle *p = NULL;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewCriticalSectionHandle();

	if( p == NULL ) return ENOMEM;

	InitializeCriticalSection( &(p->_cs) );

	*handle = (OsHandle) p;

	return ENOERROR;
}

OsError OsCriticalSectionTryEnter( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

	return (TryEnterCriticalSection( &(p->_cs) ) == TRUE) ? ENOERROR : EFAILED;

	return ENOERROR;
}

OsError OsCriticalSectionEnter( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

	EnterCriticalSection( &(p->_cs) );

	return ENOERROR;
}

OsError OsCriticalSectionLeave( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

	LeaveCriticalSection( &(p->_cs) );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//! timing

static OsTimerTime _frequencyMs;

static int _timerInitialized = 0;

OsTimerTime OsTimerGetResolution( void )
{
	LARGE_INTEGER frequency;

	BOOL havetime = TRUE;

	if( _timerInitialized == 0 )
	{
		_timerInitialized = 1;

		havetime = QueryPerformanceFrequency( &frequency );

		_ASSERT( havetime );

		_frequencyMs = frequency.QuadPart / 1000;
	}

	return _frequencyMs;
}

OsTimerCycle OsTimerGetTimeNow( void )
{
	LARGE_INTEGER now;

	BOOL havetime = QueryPerformanceCounter( &now );

	_ASSERT( havetime );

	return (OsTimerTime) now.QuadPart;
}

OsTimerCycle OsTimerGetElapsed(OsTimerCycle fromTime , OsTimerCycle toTime )
{
	return (OsTimerTime) (toTime - fromTime);
}

OsTimerTime OsTimerConvertToMillisec( OsTimerCycle cycles )
{
	_ASSERT( _timerInitialized != 0 ); //TODO this is not nice, hidden bug maker

	return cycles / _frequencyMs;
}

OsTimerTime OsTimerConvertToNanosec(OsTimerCycle cycles)
{
	_ASSERT(_timerInitialized != 0); //TODO this is not nice, hidden bug maker

	return (1000 * cycles) / _frequencyMs;
}

//////////////////////////////////////////////////////////////////////////
OsError OsWaitForObject( OsHandle handle ,int32_t timeoutMs )
{
	struct ThreadHandle *p = CastThreadHandle( handle );
	//TODO wait for other handle as well

	DWORD dwMillisecond = (timeoutMs < 0) ? INFINITE : timeoutMs;

	HRESULT hr = WaitForSingleObject( p->_handle ,dwMillisecond );

	switch( hr )
	{
	case WAIT_OBJECT_0: return ENOERROR;
	case WAIT_TIMEOUT: return ETIMEDOUT;
	case WAIT_ABANDONED: return EINTR;
	default: return EFAILED;
	}
}

OsError OsSleep( int32_t delayMs )
{
	Sleep( (DWORD) delayMs );

	return ENOERROR;
}


/*
For physical drive and not partitions
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

BOOL GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY *pdg)
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;  // handle to the drive to be examined 
	BOOL bResult = FALSE;                 // results flag
	DWORD junk = 0;                     // discard results

	hDevice = CreateFileW(wszPath,          // drive to open
		0,                // no access to the drive
		FILE_SHARE_READ | // share mode
		FILE_SHARE_WRITE,
		NULL,             // default security attributes
		OPEN_EXISTING,    // disposition
		0,                // file attributes
		NULL);            // do not copy file attributes

	if (hDevice == INVALID_HANDLE_VALUE)    // cannot open the drive
	{
		return (FALSE);
	}

	bResult = DeviceIoControl(hDevice,                       // device to be queried
		IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
		NULL, 0,                       // no input buffer
		pdg, sizeof(*pdg),            // output buffer
		&junk,                         // # bytes returned
		(LPOVERLAPPED)NULL);          // synchronous I/O

	CloseHandle(hDevice);

	return (bResult);
}
int wmain(int argc, wchar_t *argv[])
{
	DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure
	BOOL bResult = FALSE;      // generic results flag
	ULONGLONG DiskSize = 0;    // size of the drive, in bytes

	bResult = GetDriveGeometry(wszDrive, &pdg);

	if (bResult)
	{
		wprintf(L"Drive path      = %ws\n", wszDrive);
		wprintf(L"Cylinders       = %I64d\n", pdg.Cylinders);
		wprintf(L"Tracks/cylinder = %ld\n", (ULONG)pdg.TracksPerCylinder);
		wprintf(L"Sectors/track   = %ld\n", (ULONG)pdg.SectorsPerTrack);
		wprintf(L"Bytes/sector    = %ld\n", (ULONG)pdg.BytesPerSector);

		DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
			(ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;
		wprintf(L"Disk size       = %I64d (Bytes)\n"
			L"                = %.2f (Gb)\n",
			DiskSize, (double)DiskSize / (1024 * 1024 * 1024));
	}
	else
	{
		wprintf(L"GetDriveGeometry failed. Error %ld.\n", GetLastError());
	}

	return ((int)bResult);
}*/

OsError OsFileGetDriveList(char_t *drivenames, size_t sizeOfDrivenames)
{
	GetLogicalDriveStringsA((DWORD)sizeOfDrivenames, drivenames);
	
	/*std::vector<std::string> arrayOfDrives;
	char* szDrives = new char[MAX_PATH]();
	if (GetLogicalDriveStringsA(MAX_PATH, szDrives));
	for (int i = 0; i < 100; i += 4)
		if (szDrives[i] != (char)0)
			arrayOfDrives.push_back(std::string{ szDrives[i],szDrives[i + 1],szDrives[i + 2] });
	delete[] szDrives;
	return arrayOfDrives;*/
	return ENOERROR;
}

OsError OsFileGetDriveInfo(const char_t *drivename, struct OsFileDriveInfo *info)
{
	/*char VolumeSerial[9] = { 0 };// 8 hex digits + zero termination
	DWORD VolumeSerialNumber;

	if (GetVolumeInformationA(NULL, NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL)) {
		sprintf_s(VolumeSerial, 9, "%08lX", VolumeSerialNumber);
		printf("Volume serial number: %8.8s.\n", VolumeSerial);
		return ENOERROR;
	}
	else {
		printf("GetVolumeInformationA() error: %08lX\n", GetLastError());
		return EFAILED;
	}*/

// Tiny
/*OS_DRIVE_UNKNOWN	0
OS_DRIVE_FIXED		1
OS_DRIVE_REMOVABLE	2
OS_DRIVE_REMOTE		3
OS_DRIVE_CDROM		4
OS_DRIVE_RAMDISK	5*/

// Win32
/*DRIVE_UNKNOWN 0 = > 0
DRIVE_NO_ROOT_DIR 1 = > 0
DRIVE_REMOVABLE 2 = > 2
DRIVE_FIXED 3 = > 1
DRIVE_REMOTE 4 = > 3
DRIVE_CDROM 5 = > 4
DRIVE_RAMDISK 6 = > 5*/


	int win32to_tiny[] = { 0, 0, 2, 1, 3, 4, 5 };
	info->driveType = GetDriveType(drivename);
	info->driveType = win32to_tiny[info->driveType];


	/*[out] LPDWORD lpSectorsPerCluster,
		[out] LPDWORD lpBytesPerSector,
		[out] LPDWORD lpNumberOfFreeClusters,
		[out] LPDWORD lpTotalNumberOfClusters*/

	/*uint64_t sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
	GetDiskFreeSpaceEx(drivename, &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters);
	info->freeSize = numberOfFreeClusters * sectorsPerCluster * bytesPerSector;
	info->totalSize = totalNumberOfClusters * sectorsPerCluster * bytesPerSector;*/

	ULARGE_INTEGER bytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;
	GetDiskFreeSpaceEx(drivename, &bytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes);

	info->freeSize = totalNumberOfFreeBytes.QuadPart;// TODO 32 bit version
	info->totalSize = totalNumberOfBytes.QuadPart;

	memcpy(info->rootPathname, drivename, 4);

	return ENOERROR;
	//printf("%llu %llu %llu\n", p1, p2, p3);
}

OsError OsFileGetAttributes(const char_t *filename, struct OsFileAttributes *attributes)
{
	WIN32_FILE_ATTRIBUTE_DATA fInfo;
	GetFileAttributesEx(filename, GetFileExInfoStandard, &fInfo);

	if (fInfo.dwFileAttributes == INVALID_FILE_ATTRIBUTES) return ENODATA;// File not found

	attributes->archive = ((fInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0);
	attributes->directory = ((fInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
	attributes->hidden = ((fInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0);
	attributes->readonly = ((fInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);
	attributes->system = ((fInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0);

	return ENOERROR;
}

OsError OsFileGetFileInfo(const char_t *filename, struct OsFileFileInfo *info)
{
	WIN32_FILE_ATTRIBUTE_DATA fInfo;
	GetFileAttributesEx(filename, GetFileExInfoStandard, &fInfo);

	if (fInfo.dwFileAttributes == INVALID_FILE_ATTRIBUTES) return ENODATA;// File not found

	info->_attributes.archive = (fInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0;
	info->_attributes.directory = (fInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	info->_attributes.hidden = (fInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
	info->_attributes.readonly = (fInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
	info->_attributes.system = (fInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;

	FileTimeToSystemTime(&fInfo.ftCreationTime, (SYSTEMTIME*)&info->creationTime);
	FileTimeToSystemTime(&fInfo.ftLastAccessTime, (SYSTEMTIME*)&info->lastAccessTime);
	FileTimeToSystemTime(&fInfo.ftLastWriteTime, (SYSTEMTIME*)&info->lastWriteTime);

	LARGE_INTEGER s;
	s.HighPart = fInfo.nFileSizeHigh;
	s.LowPart = fInfo.nFileSizeLow;
	info->_filesize = s.QuadPart;

	strcpy_s(info->filename, 512, filename);

	return ENOERROR;
}

OsError OsFileGetSizeByName(const char_t *filename, uint64_t* size)
{
	WIN32_FILE_ATTRIBUTE_DATA fInfo;
	if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &fInfo))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		return EFAILED;
	}
	LARGE_INTEGER s;
	s.HighPart = fInfo.nFileSizeHigh;
	s.LowPart = fInfo.nFileSizeLow;
	*size = s.QuadPart;
	return ENOERROR;
}

OsError OsFileCopy(const char_t *oldFileName, const char_t *newFileName, int failifexist)
{
	if (!CopyFile(oldFileName, newFileName, (BOOL)failifexist))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		return EFAILED;
	}

	return ENOERROR;
}

OsError OsFileMove(const char_t *oldFileName, const char_t *newFileName)
{
	if (!MoveFile(oldFileName, newFileName/*, FALSE*/))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		return EFAILED;
	}

	return ENOERROR;
}

//! can also be used to rename a file
//////////////////////////////////////////////////////////////////////////
OsError OsFileDelete(const char_t *path)
{
	if (!DeleteFile(path))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		return EFAILED;
	}

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
/*OsError OsFileCreateDir(const char_t *path)
{
	if (!CreateDirector(path))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		if (err == ERROR_ALREADY_EXISTS)
			return EALREADY;
	
		return EFAILED;
	}

	return ENOERROR;
}*/

//////////////////////////////////////////////////////////////////////////
char_t *win_findPathSep(const char_t *path)
{
	char_t *chr_bs = _tcschr(path, _T('\\'));
	char_t *chr_fs = _tcschr(path, _T('/'));

	if (chr_bs == NULL) return chr_fs;
	if (chr_fs == NULL) return chr_bs;

	return (chr_bs < chr_fs) ? chr_bs : chr_fs;
}

OsError OsFileCreateDir(const char_t *path)
{
	char_t folder[MAX_PATH];

	ZeroMemory(folder, MAX_PATH * sizeof(char_t));

	char_t *end = win_findPathSep(path);

	while (end != NULL)
	{
		_tcsncpy(folder, path, end - path + 1);

		if (!CreateDirectory(folder, NULL))
		{
			DWORD err = GetLastError();

			if (err == ERROR_PATH_NOT_FOUND)
				return ENODATA;
		}

		end = win_findPathSep(++end);
	}

	return ENOERROR;
}

/*#include "shellapi.h"

bool DeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin)
{
	int len = _tcslen(lpszDir);
	TCHAR* pszFrom = malloc(sizeof(TCHAR) * (len + 4)); //4 to handle wide char
										 //_tcscpy(pszFrom, lpszDir); //todo:remove warning//;//convet wchar to char*
	wcscpy_s(pszFrom, len + 2, lpszDir);
	pszFrom[len] = 0;
	pszFrom[len + 1] = 0;

	SHFILEOPSTRUCT fileop;
	fileop.hwnd = NULL;    // no status display
	fileop.wFunc = FO_DELETE;  // delete operation
	fileop.pFrom = pszFrom;  // source file name as double null terminated string
	fileop.pTo = NULL;    // no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;  // do not prompt the user

	if (!noRecycleBin)
		fileop.fFlags |= FOF_ALLOWUNDO;

	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle = NULL;
	fileop.hNameMappings = NULL;

	int ret = SHFileOperation(&fileop); //SHFileOperation returns zero if successful; otherwise nonzero 
	free(pszFrom);
	return (0 == ret);
}*/


OsError OsFileRemoveDir(const char_t *path)
{
	if (!RemoveDirectory(path))
	{
		DWORD err = GetLastError();

		if (err == ERROR_PATH_NOT_FOUND)
			return ENODATA;

		return EFAILED;
	}
	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
void win_FindDataToStruct( const WIN32_FIND_DATA *data ,struct OsFileFindFile *findfile )
{
	findfile->_info._attributes.directory = ((data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? 1 :0;
	findfile->_info._attributes.archive = ((data->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0) ? 1 :0;
	findfile->_info._attributes.hidden = ((data->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0) ? 1 :0;
	findfile->_info._attributes.readonly = ((data->dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) ? 1 :0;
	findfile->_info._attributes.system = ((data->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0) ? 1 :0;

	FileTimeToSystemTime(&data->ftCreationTime, (SYSTEMTIME*)&findfile->_info.creationTime);
	FileTimeToSystemTime(&data->ftLastAccessTime, (SYSTEMTIME*)&findfile->_info.lastAccessTime);
	FileTimeToSystemTime(&data->ftLastWriteTime, (SYSTEMTIME*)&findfile->_info.lastWriteTime);

	LARGE_INTEGER s;
	s.HighPart = data->nFileSizeHigh;
	s.LowPart = data->nFileSizeLow;
	findfile->_info._filesize = s.QuadPart;

	strcpy_s( findfile->_info.filename ,512 ,data->cFileName );
}

OsError OsFileFindFirst( const char_t *filemask ,struct OsFileFindFile *findfile )
{
	HANDLE hFind;

	WIN32_FIND_DATA data;

	hFind = FindFirstFile( filemask ,&data );

	if( hFind == INVALID_HANDLE_VALUE )
		return EFAILED;

	findfile->handle = hFind;
	
	win_FindDataToStruct( &data ,findfile );

	return ENOERROR;
}

OsError OsFileFindNext( struct OsFileFindFile *findfile )
{
	HANDLE hFind = findfile->handle;

	WIN32_FIND_DATA data;

	if( FindNextFile( hFind ,&data ) == FALSE )
		return EFAILED;

	win_FindDataToStruct( &data ,findfile );

	return ENOERROR;
}

OsError OsFileFindClose( struct OsFileFindFile *findfile )
{
	HANDLE hFind = findfile->handle;

	FindClose( hFind );

	findfile->handle = 0;

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//! file

#define FILEHANDLE_MAGIC	0x0A48BC441

struct FileHandle
{
	uint32_t _magic;

	HANDLE _handle;
};

static struct FileHandle *NewFileHandle( void )
{
	const size_t size = sizeof(struct FileHandle);

	struct FileHandle *p = (struct FileHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = FILEHANDLE_MAGIC;

	p->_handle = INVALID_HANDLE_VALUE;

	return p;
}

static struct FileHandle *CastFileHandle( OsHandle handle )
{
	struct FileHandle *p = (struct FileHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != FILEHANDLE_MAGIC ) return NULL;

	return p;
}

//////////////////////////////////////////////////////////////////////////
DWORD _win_mapaccess( int access )
{
	return ((access & OS_ACCESS_READ) ? GENERIC_READ : 0)
		| ((access & OS_ACCESS_WRITE) ? GENERIC_WRITE : 0)
		| ((access & OS_ACCESS_EXECUTE) ? GENERIC_EXECUTE : 0);
}

DWORD _win_mapshare( int share )
{
	return ((share & OS_SHARE_READ) ? FILE_SHARE_READ : 0)
		| ((share & OS_SHARE_WRITE) ? FILE_SHARE_WRITE : 0)
		| ((share & OS_SHARE_DELETE) ? FILE_SHARE_DELETE : 0);
}

DWORD _win_mapcreate( int create )
{
	switch( create )
	{
	case OS_CREATE_DONT: return OPEN_EXISTING;
	case OS_CREATE_NOEXIST: return OPEN_ALWAYS;
	case OS_CREATE_NEW: return CREATE_NEW;
	case OS_CREATE_TRUNCATE: return TRUNCATE_EXISTING;
	case OS_CREATE_ALWAYS: return CREATE_ALWAYS;

	default: return OPEN_EXISTING;
	}
}

DWORD _win_mapattributes( struct OsFileAttributes *attributes )
{
	if( attributes == NULL ) return FILE_ATTRIBUTE_NORMAL;

	return ((attributes->archive != 0) ? FILE_ATTRIBUTE_ARCHIVE : 0)
		| ((attributes->hidden != 0) ? FILE_ATTRIBUTE_HIDDEN : 0)
		| ((attributes->readonly != 0) ? FILE_ATTRIBUTE_READONLY : 0)
		| ((attributes->system != 0) ? FILE_ATTRIBUTE_SYSTEM : 0);
}

OsError OsFileOpen( OsHandle *handle ,const char_t *filename ,int access ,int share ,int create ,struct OsFileAttributes *attributes )
{
	struct FileHandle *p = NULL;

	DWORD dwDesiredAccess = _win_mapaccess(access);

	DWORD dwShareMode = _win_mapshare(share);

	DWORD dwCreationDisposition = _win_mapcreate(create);

	DWORD dwFlagsAndAttributes = _win_mapattributes(attributes);

	if( handle == NULL || filename == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewFileHandle();

	if( p == NULL ) return ENOMEM;

	p->_handle = CreateFile(
		filename				// - name of the file to be created or opened
		,dwDesiredAccess		// - requested access to the file
		,dwShareMode			// - requested sharing mode of the file
		,NULL					// - security attributes
		,dwCreationDisposition	// - action to take on a file that exists or does not exist
		,dwFlagsAndAttributes	// - file attributes and flags
		,NULL					// - template file
		);

	if( p->_handle == INVALID_HANDLE_VALUE )
	{
		free( (void*) p );

		return GetLastOsError();
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

OsError OsFileRead( OsHandle handle ,uint8_t *memory ,size_t size ,size_t *readSize )
{
	struct FileHandle *p = CastFileHandle( handle );

	DWORD read = 0;

	if( p == NULL || memory == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	//TODO read the entire readSize
	//CHECK if we keep readSize in param ?

	if( ReadFile( p->_handle ,(void*) memory ,(DWORD) size ,&read ,NULL ) == FALSE )
		return GetLastOsError();

	if( readSize ) *readSize = read;

	return ENOERROR;
}

OsError OsFileWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,size_t *writeSize )
{
	struct FileHandle *p = CastFileHandle( handle );

	DWORD write = 0;

	if( p == NULL || memory == NULL ) return EINVAL;

	if( p->_handle == INVALID_HANDLE_VALUE ) return EINVAL;

	if( WriteFile( p->_handle ,(const void*) memory ,(DWORD) size , &write ,NULL ) == FALSE )
		return GetLastOsError();

	if( writeSize ) *writeSize = write;

	return ENOERROR;
}

OsError OsFileGetSize( OsHandle handle ,uint64_t *size )
{
	struct FileHandle *p = CastFileHandle( handle );

	LARGE_INTEGER filesize;

	if( p == NULL || size == NULL ) return EINVAL;

	if( GetFileSizeEx( p->_handle ,&filesize ) == 0 ) return EFAILED;

	if( size ) *size = filesize.QuadPart;

	return ENOERROR;
}

OsError OsFileSeek( OsHandle handle ,size_t distance ,int seekpos )
{
	return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
//! Network

#define NETHANDLE_MAGIC	0x0D4F7A93B

struct NetHandle
{
	uint32_t _magic;

	SOCKET _socket;
};

static struct NetHandle *NewNetHandle( void )
{
	const size_t size = sizeof(struct NetHandle);

	struct NetHandle *p = (struct NetHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = NETHANDLE_MAGIC;

	return p;
}

static struct NetHandle *CastNetHandle( OsHandle handle )
{
	struct NetHandle *p = (struct NetHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != NETHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteNetHandle( OsHandle *handle )
{
	struct NetHandle *p = * (struct NetHandle **) handle;

	if( *handle == OS_INVALID_HANDLE ) return ENOERROR;

	if( p == NULL ) return EINVAL;

	if( p->_socket != INVALID_SOCKET ) closesocket( p->_socket );

	*handle = OS_INVALID_HANDLE;

	free( p );

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
static ASYNCH_VARIABLE _win_wsaInitialize = 0;

static ASYNCH_VARIABLE _win_wsaReady = 0;

CRITICAL_SECTION _win_cs_net;

/* int _win_wsaerrormap( int error )
{
	// WSANOTINITIALISED ... // etc
} */

int _win_netinit( void )
{
	static int _wsaInitialized = 0;
	
	int error = ENOERROR;

	if( _wsaInitialized != 0 ) return ENOERROR;

	if( ASYNCH_INCREMENT( _win_wsaInitialize ) == 1 )
	{
		WSADATA wsaData;

		error = WSAStartup( MAKEWORD(2,0) ,&wsaData );

		ASYNCH_INCREMENT(_win_wsaReady);
	}

	while( ASYNCH_READ(_win_wsaReady ) == 0 );

	return error;
}

void _win_addrin( SOCKADDR_IN *sin ,const char *ip )
{
	const char *c = NULL;
	
	sin->sin_family = AF_INET;

	if( ip == NULL )
	{
		sin->sin_addr.s_addr = INADDR_ANY;
		sin->sin_port = htons(8888);
	}

	else if( (c = strchr( ip ,':' )) == NULL )
	{
		sin->sin_addr.s_addr = inet_addr( ip );
		sin->sin_port = htons(8888);
	}

	else
	{
		char addr[32];

		int i = MIN( (int) (c-ip) ,31);

		int port = atoi( c+1 );

		strncpy_s( addr ,32 ,ip ,i ); addr[i] = 0;

		sin->sin_addr.s_addr = inet_addr( addr );

		sin->sin_port = htons(port);
	}
}

//////////////////////////////////////////////////////////////////////////
//-- server

TINYFUN OsError OsNetListen( OsHandle *handle ,const char_t *localAddress )
{
	struct NetHandle *p = NULL;

	SOCKADDR_IN sin;

	int wsaError = 0;

	if( handle == NULL ) return EINVAL;

	p = NewNetHandle();

	if( p == NULL ) return ENOMEM;

	wsaError = _win_netinit();

	if( wsaError != 0 ) return EFAILED; //! TODO map error

	p->_socket = socket( AF_INET ,SOCK_STREAM ,0 );

	_win_addrin( &sin ,localAddress );

	// ioctlsocket( socket ,FIONBIO ,&bio );

	bind( p->_socket ,(SOCKADDR*) &sin ,sizeof(sin) );

	if( listen( p->_socket ,0 ) == SOCKET_ERROR )
	{
		wsaError = WSAGetLastError();

		// if( wsaError == WSAEWOULDBLOCK )

		// WSAENOTSOCK // 10038

		// WSAECONNREFUSED // 10061

		//? closesocket ?
		free( p );

		return EFAILED; //! TODO map error
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN OsError OsNetAccept( OsHandle listenHandle ,OsHandle *handle ,int32_t timeoutMs )
{
	struct NetHandle *listen = CastNetHandle( listenHandle );

	struct NetHandle *p = NULL;

	SOCKADDR_IN csin;

	int csin_size = sizeof(csin);

	int wsaError = 0;

	if( handle == NULL ) return EINVAL;

	p = NewNetHandle();

	if( p == NULL ) return ENOMEM;

	p->_socket = accept( listen->_socket ,(SOCKADDR*) &csin ,&csin_size );

	if( p->_socket == INVALID_SOCKET )
	{
		// WSAGetLastError + map

		free( p );

		return EFAILED;
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//-- client

TINYFUN OsError OsNetConnect( OsHandle *handle ,const char_t *peerAddress ,const char *localAddress ,int32_t timeoutMs )
{
	struct NetHandle *p = NULL;

	SOCKADDR_IN sin;

	int wsaError = 0;

	if( handle == NULL || peerAddress == NULL ) return EINVAL;

	p = NewNetHandle();

	if( p == NULL ) return ENOMEM;

	wsaError = _win_netinit();

	if( wsaError != 0 ) return EFAILED; //! TODO map error

	//TODO local address ?
	p->_socket = socket( AF_INET ,SOCK_STREAM ,0 );

	_win_addrin( &sin ,peerAddress );
	/* sin.sin_addr.s_addr = inet_addr( peerAddress );
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888); */

	// ioctlsocket( socket ,FIONBIO ,&bio );

	if( connect( p->_socket ,(SOCKADDR*) &sin ,sizeof(sin) ) != 0 )
	{
		wsaError = WSAGetLastError();

		// if( wsaError == WSAEWOULDBLOCK )

		// WSAENOTSOCK // 10038

		// WSAECONNREFUSED // 10061

		//? closesocket ?
		free( p );

		return EFAILED; //! TODO map error
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//-- common
TINYFUN OsError OsNetRead( OsHandle handle ,uint8_t *memory ,size_t size ,size_t *readSize ,int32_t timeoutMs )
{
	struct NetHandle *p = CastNetHandle( handle );

	int read = 0 ,toread = (int) size;

	//TODO if memory NULL accept and just skip the data
	if( p == NULL || memory == NULL || size <= 0 ) return EINVAL;

	do 
	{
		if( toread != (int) size )
			toread = toread;

		read = recv( p->_socket ,memory + (int) size - toread ,toread ,0 );

		if( read == 0 ) //! socket was gracefully closed
		{
			if( readSize != NULL ) { *readSize = (int) size - toread; return ENOERROR; }

			return EINTR;
		}

		if( read == SOCKET_ERROR )
			return EFAILED; //TODO map wsa error 

		toread -= read;

	} while( toread != 0 );

	if( readSize != NULL ) *readSize = (size_t) size;

	return ENOERROR;
}

TINYFUN OsError OsNetWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,int32_t timeoutMs )
{
	struct NetHandle *p = CastNetHandle( handle );

	int wsaError = 0;

	if( p == NULL || memory == NULL || size <= 0 ) return EINVAL;

	wsaError = send( p->_socket ,memory ,(int) size ,0 );

	if( wsaError <= 0 )
		return EFAILED; //TODO map wsa error 

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
#define WIN_GETCONSOLE	if( _hconsole == INVALID_HANDLE_VALUE ) _hconsole = GetStdHandle(STD_OUTPUT_HANDLE);

HANDLE _hconsole = INVALID_HANDLE_VALUE;

WORD _consoleAttrib = 0;

// #include <stdio.h>
// #include <fcntl.h>
// #include <io.h>

TINYFUN OsError OsConsoleWindowCreate( const char_t *name )
{
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;

	OsError error = ENOERROR;

	HANDLE hin;
	// long herror; ///? TODO

	int hstd;

	FILE *fp = NULL;

	if( AllocConsole() == FALSE )
		error = GetLastOsError(); //! IE not closing

	if( _hconsole == INVALID_HANDLE_VALUE )
		_hconsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if( GetConsoleScreenBufferInfo( _hconsole ,&csbInfo ) == TRUE )
		_consoleAttrib = csbInfo.wAttributes;

	if( error != ENOERROR ) return error;

	if( name != NULL ) SetConsoleTitle( name ); 

	//! redirect unbuffered STDOUT to console
	hstd = _open_osfhandle( (intptr_t) _hconsole ,_O_TEXT );
	fp = _fdopen( hstd ,"w" );
	*stdout = *fp;
	setvbuf( stdout ,NULL ,_IONBF ,0 );

	//! same for input
	hin = GetStdHandle(STD_INPUT_HANDLE);

	hstd = _open_osfhandle( (intptr_t) hin ,_O_TEXT );
	fp = _fdopen( hstd ,"r" );
	*stdin = *fp;
	setvbuf( stdin ,NULL ,_IONBF ,0 );

	return ENOERROR;
}

TINYFUN OsError OsConsoleWindowClose( void )
{
	if( FreeConsole() == FALSE )
		return GetLastOsError();

	return ENOERROR;
}

/* //NB for windows :
#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.

#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
*/

OsColorRef _win_maptextcolor_tocolor( BYTE color )
{
	OsColorRef rgb_colors[] = { OS_COLOR_BLACK, OS_COLOR_DARKBLUE, OS_COLOR_DARKGREEN, OS_COLOR_LIGHTBLUE,
		OS_COLOR_DARKRED, OS_COLOR_MAGENTA, OS_COLOR_ORANGE, OS_COLOR_LIGHTGRAY, OS_COLOR_GRAY,
		OS_COLOR_BLUE, OS_COLOR_GREEN, OS_COLOR_CYAN, OS_COLOR_RED, OS_COLOR_PURPLE/*OS_COLOR_PINK*/, OS_COLOR_YELLOW, OS_COLOR_WHITE };
	if (color >= 16) return OS_COLOR_WHITE;

	return rgb_colors[color];
}

BYTE _win_maptextcolor_tobyte( OsColorRef color )
{
	switch (color)
	{
		case OS_COLOR_BLACK: return 0;
		case OS_COLOR_DARKBLUE: return 1;
		case OS_COLOR_DARKGREEN: return 2;
		case OS_COLOR_LIGHTBLUE: return 3;
		case OS_COLOR_DARKRED: return 4;
		case OS_COLOR_MAGENTA: return 5;
		case OS_COLOR_ORANGE: return 6;
		case OS_COLOR_LIGHTGRAY: return 7;
		case OS_COLOR_GRAY: return 8;
		case OS_COLOR_BLUE: return 9;
		case OS_COLOR_GREEN: return 10;
		case OS_COLOR_CYAN: return 11;
		case OS_COLOR_RED: return 12;
		case OS_COLOR_PURPLE /*OS_COLOR_PINK*/: return 13;
		case OS_COLOR_YELLOW: return 14;
		case OS_COLOR_WHITE: return 15;
		default : return 15;
	}

	BYTE b = (BYTE) (color & OS_COLOR_BLUE);

	BYTE g = (BYTE) ((color & OS_COLOR_GREEN) >> 8);

	BYTE r = (BYTE) ((color & OS_COLOR_RED) >> 16);

	BYTE bgr = 0;
		
	if( (b+g+r) > 192 ) bgr = 8;

	if( (b & 0x0c0) != 0 ) bgr |= 1;
	if( (g & 0x0c0) != 0 ) bgr |= 2;
	if( (r & 0x0c0) != 0 ) bgr |= 4;

	if (color == OS_COLOR_GRAY)
		return 8;

	return bgr;
}

TINYFUN OsError OsConsoleGetInfo( struct OsConsoleInfo *info )
{
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;

	WIN_GETCONSOLE;

	if( GetConsoleScreenBufferInfo( _hconsole ,&csbInfo ) == FALSE )
		return GetLastOsError();

	if( info == NULL ) return EINVAL;

	info->width = csbInfo.dwSize.X;
	info->height = csbInfo.dwSize.Y;

	info->posx = csbInfo.dwCursorPosition.X;
	info->posy = csbInfo.dwCursorPosition.Y;

	info->forecolor = _win_maptextcolor_tocolor( (BYTE) (csbInfo.wAttributes & 0x00f) );
	info->backcolor = _win_maptextcolor_tocolor( (BYTE) ((csbInfo.wAttributes & 0x0f0) << 4) );

	return ENOERROR;
}

TINYFUN void OsConsoleSetPosition( int x ,int y )
{
	COORD pos;

	WIN_GETCONSOLE;

	pos.X = x;
	pos.Y = y;

	SetConsoleCursorPosition( _hconsole ,pos );
}

TINYFUN void OsConsoleSetColor( int select ,OsColorRef color )
{
	int attribs = _consoleAttrib;

	if( _hconsole == INVALID_HANDLE_VALUE )
	{
		CONSOLE_SCREEN_BUFFER_INFO csbInfo;

		_hconsole = GetStdHandle(STD_OUTPUT_HANDLE);

		if( GetConsoleScreenBufferInfo( _hconsole ,&csbInfo ) == TRUE )
			attribs = _consoleAttrib = csbInfo.wAttributes;
	}

	FlushConsoleInputBuffer( _hconsole );

	if( select == OS_SELECT_FORECOLOR || select == OS_SELECT_TEXTCOLOR )
	{
		attribs = attribs ^ (attribs&0x00f);

		attribs = attribs | ( _win_maptextcolor_tobyte( color ) );
	}
	else // BACKCOLOR / FILLCOLOR
	{
		attribs = attribs ^ (attribs&0x0f0);

		attribs = attribs | ( _win_maptextcolor_tobyte( color ) << 4 );
	}

	_consoleAttrib = attribs;

	SetConsoleTextAttribute( _hconsole ,attribs );
}

TINYFUN void OsConsoleTextOut( const char_t *string ,int length )
{
	DWORD charsWritten = 0;

	WIN_GETCONSOLE;

	if( string == NULL ) return;

	if( length < 0 ) length = (int) _tcslen( string );

	WriteConsole( _hconsole ,string ,length ,&charsWritten ,NULL );
}

TINYFUN void OsConsoleTextGet( char_t *string ,int *length ,int maxchar )
{
	DWORD charsRead = 0;

	WIN_GETCONSOLE;

	if( string == NULL || maxchar < 1 ) return;

	ReadConsole( _hconsole ,string ,maxchar-1 ,&charsRead ,NULL );

	if( length ) *length = charsRead;

	string[charsRead] = 0;
}

//////////////////////////////////////////////////////////////////////////
#include <CommDlg.h>


#define DEFAULT_BUTTON(val) ((val >> 4) & 0xF)
#define ICON(val) ((val >> 8) & 0xF)

TINYFUN int OsDialogNativeMessageBox(OsHandle owner, const char_t *text, const char_t *caption, int type, uint16_t flags)
{
	unsigned int win_type = 0;
	switch (type & 0XF)
	{
	case OS_GUIMSGBOX_OK:
		win_type = MB_OK; win_type |= MB_DEFBUTTON1; break;
	case OS_GUIMSGBOX_OKCANCEL:
		win_type = MB_OKCANCEL; 
		if (DEFAULT_BUTTON(type) == 0) win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_OKBUTTON)
			win_type |= MB_DEFBUTTON1;
		else win_type |= MB_DEFBUTTON2;
		break;
	case OS_GUIMSGBOX_RETRYCANCEL:
		win_type = MB_RETRYCANCEL; break;
		if (DEFAULT_BUTTON(type) == 0) win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_RETRYBUTTON)
			win_type |= MB_DEFBUTTON1;
		else win_type |= MB_DEFBUTTON2;
	case OS_GUIMSGBOX_YESNO:
		win_type = MB_YESNO; break;
		if (DEFAULT_BUTTON(type) == 0) win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_YESBUTTON)
			win_type |= MB_DEFBUTTON1;
		else win_type |= MB_DEFBUTTON2;
	case OS_GUIMSGBOX_YESNOCANCEL:
		win_type = MB_YESNOCANCEL;
		if (DEFAULT_BUTTON(type) == 0) win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_YESBUTTON)
			win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_NOBUTTON) 
			win_type |= MB_DEFBUTTON2;
		else win_type |= MB_DEFBUTTON3;
		break;
	case OS_GUIMSGBOX_CANCELRETRYCONTINUE:
		win_type = MB_CANCELTRYCONTINUE; 
		if (DEFAULT_BUTTON(type) == 0) win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_CANCELBUTTON)
			win_type |= MB_DEFBUTTON1;
		else if (DEFAULT_BUTTON(type) == OS_GUIMSGBOX_RETRYBUTTON)
			win_type |= MB_DEFBUTTON2;
		else win_type |= MB_DEFBUTTON3;
		break;
	case OS_GUIMSGBOX_HELP:
		win_type = MB_HELP; win_type |= MB_DEFBUTTON1; break;
	default:
		win_type = MB_OK; win_type |= MB_DEFBUTTON1; break;
	};

	switch(ICON(type))
	{
	case OS_GUIMSGBOX_ICONEXCLAMATION: win_type |= MB_ICONEXCLAMATION; break;
	case OS_GUIMSGBOX_ICONWARNING: win_type |= MB_ICONWARNING; break;
	case OS_GUIMSGBOX_ICONINFORMATION: win_type |= MB_ICONINFORMATION; break;
	case OS_GUIMSGBOX_ICONASTERISK: win_type |= MB_ICONASTERISK; break;
	case OS_GUIMSGBOX_ICONQUESTION: win_type |= MB_ICONQUESTION; break;
	case OS_GUIMSGBOX_ICONSTOP: win_type |= MB_ICONSTOP; break;
	case OS_GUIMSGBOX_ICONERROR: win_type |= MB_ICONERROR; break;
	case OS_GUIMSGBOX_ICONHAND: win_type |= MB_ICONHAND; break;
	default:win_type |= MB_ICONINFORMATION;
	}

	int res = MessageBox(owner, text, caption, win_type);

	switch (res)
	{
	case IDABORT: return OS_GUIMSGBOX_ABORTBUTTON;
	case IDCANCEL: return OS_GUIMSGBOX_CANCELBUTTON;
	case IDCONTINUE: return OS_GUIMSGBOX_CONTINUEBUTTON;
	case IDIGNORE: return OS_GUIMSGBOX_IGNOREBUTTON;
	case IDNO: return OS_GUIMSGBOX_NOBUTTON;
	case IDOK: return OS_GUIMSGBOX_OKBUTTON;
	case IDTRYAGAIN:
	case IDRETRY: return OS_GUIMSGBOX_RETRYBUTTON;
	case IDYES: return OS_GUIMSGBOX_YESBUTTON;
	}
	return OS_GUIMSGBOX_CANCELBUTTON;
}

TINYFUN OsError OsDialogNativeFileSelect( OsHandle owner ,const char_t *directory ,const char_t *filters ,int defaultFilter ,char_t *filenames ,int length ,uint16_t flags )
{
	OPENFILENAME ofn;
	// DWORD err;

	filenames[0] = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL; // owner; // translate
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filters; // TEXT("TIFF Files\0*.TIFF;*.TIF\0test file\0*.test\0All Files\0*.*\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = defaultFilter;
	ofn.lpstrFile = filenames;
	ofn.nMaxFile = length;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("TIFF");
	ofn.FlagsEx = 0;

	if( flags & OS_GUIDIALOG_FILEOPEN )
	{
		if( GetOpenFileName( &ofn ) ) return ENOERROR; else return EFAILED;
	}

	else if( flags & OS_GUIDIALOG_FILESAVE )
	{
		if( GetSaveFileName( &ofn ) ) return ENOERROR; else return EFAILED;
	}

	// err = CommDlgExtendedError();
	
	return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! GUI front end 

//! link with required sub system (GDI, X11, OpenGL ,... )

//////////////////////////////////////////////////////////////////////////
//#define _OPENGL_GUI_ //_NATIVE_GUI_

#ifdef _NATIVE_GUI_
 extern struct OsGuiSystemTable *_guiSystemGDI;
#else
 static struct OsGuiSystemTable *_guiSystemGDI = NULL;
#endif

#ifdef _OPENGL_GUI_
 // extern struct OsGuiSystemTable *_guiSystemOPENGL;
  extern struct OsGuiSystemTable *_guiSystemGL;
#else
 // static struct OsGuiSystemTable *_guiSystemOPENGL = NULL;
 static struct OsGuiSystemTable *_guiSystemGL = NULL;
#endif

#ifdef _OPENGLES_GUI_
 extern struct OsGuiSystemTable *_guiSystemOPENGLES;
#else
 static struct OsGuiSystemTable *_guiSystemOPENGLES = NULL;
#endif

#ifdef _DIRECTX_GUI_
 extern struct OsGuiSystemTable *_guiSystemDIRECTX;
#else
 static struct OsGuiSystemTable *_guiSystemDIRECTX = NULL;
#endif

//! native system is default
static int _guiSystemCurrentId = OS_GUISYSTEMID_NATIVE;

static struct OsGuiSystemTable *guiGetSystemFromId( int guiSystemId )
{
	switch( guiSystemId )
	{
	case OS_GUISYSTEMID_CURRENT:
		return guiGetSystemFromId(_guiSystemCurrentId);

	case OS_GUISYSTEMID_NATIVE: return _guiSystemGDI;
	case OS_GUISYSTEMID_OPENGL: return _guiSystemGL; // _guiSystemOPENGL;
	case OS_GUISYSTEMID_OPENGLES: return _guiSystemOPENGLES;
	case OS_GUISYSTEMID_DIRECTX: return _guiSystemDIRECTX;
	default: return NULL;
	};
}

TINYFUN OsError OsGuiSetCurrentSystem( int guiSystemId )
{
	struct OsGuiSystemTable *gui = guiGetSystemFromId( guiSystemId );

	if( gui == NULL ) return ENOSYS;

	_guiSystemCurrentId = guiSystemId;

	return ENOERROR;
}

TINYFUN OsError OsGuiGetCurrentSystem( int *guiSystemId )
{
	if( guiSystemId == NULL ) return EINVAL;

	*guiSystemId = _guiSystemCurrentId;

	return ENOERROR;
}

TINYFUN OsError OsGuiGetSystemTable( int guiSystemId ,struct OsGuiSystemTable **guiSystemTable )
{
	struct OsGuiSystemTable *gui = guiGetSystemFromId( guiSystemId );

	if( gui == NULL ) return ENOSYS;

	if( guiSystemTable == NULL ) return EINVAL;

	*guiSystemTable = gui;

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
/* #define GUISYSTEMHANDLE_MAGIC	0x06F5CC4BD

struct GuiSystemHandle
{
	uint32_t _magic;

	struct OsGuiSystemTable *_guiSystem;
};*/

static struct GuiSystemHandle *CastGuiSystemHandle( OsHandle handle )
{
	struct GuiSystemHandle *p = (struct GuiSystemHandle *) handle;

	if( p == NULL ) return OS_INVALID_HANDLE;

	if( p->_magic != GUISYSTEMHANDLE_MAGIC ) return OS_INVALID_HANDLE;

	return p;
}

//////////////////////////////////////////////////////////////////////////
TINYFUN OsError OsGuiWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData )
{
	struct OsGuiSystemTable *guiTable = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( guiTable == NULL ) return ENOSYS;

	return guiTable->_WindowCreate( handle ,name ,properties ,eventFunction ,userData );
}

/*
TINYFUN void OsGuiWindowRefresh( OsHandle handle ,int update )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_WindowRefresh( handle ,update );
}
*/

TINYFUN void OsGuiWindowRefresh( OsHandle handle ,const struct OsRect *updateRect ,int flags )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_WindowRefresh( handle ,updateRect ,flags );
}

TINYFUN void OsGuiWindowShow( OsHandle handle ,int visible )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_WindowShow( handle ,visible );
}

TINYFUN void OsGuiMouseSetCursor( OsHandle handle ,int cursorId )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_MouseSetCursor( handle ,cursorId );
}

TINYFUN void OsGuiMouseCapture( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_MouseCapture( handle );
}

TINYFUN void OsGuiMouseRelease( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_MouseRelease( handle );
}

TINYFUN void OsGuiGetClientArea( OsHandle handle ,struct OsPoint *size )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_GetClientArea( handle ,size );
}

TINYFUN void OsGuiSetColor( OsGuiContext context ,int selectColor ,OsColorRef color )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_SetColor( context ,selectColor ,color );
}

TINYFUN void OsGuiRegionSetArea( OsGuiContext context ,int left ,int top ,int right ,int bottom ,int useOffset )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_RegionSetArea( context ,left ,top ,right ,bottom ,useOffset );
}

TINYFUN void OsGuiRegionGetArea( OsGuiContext context ,struct OsRect *area )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_RegionGetArea( context ,area );
}

TINYFUN void OsGuiRegionSetOffset( OsGuiContext context ,int x ,int y )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_RegionSetOffset( context ,x ,y );
}

TINYFUN void OsGuiRegionSetScale( OsGuiContext context ,float x ,float y )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_RegionSetScale( context ,x ,y );
}

TINYFUN void OsGuiRegionGetSize( OsGuiContext context ,int *width ,int *height )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_RegionGetSize( context ,width ,height );
}

TINYFUN void OsGuiPointToCoords( OsGuiContext context ,int surfaceForP ,const struct OsPoint *p ,int surfaceForCoords ,struct OsPoint *coords )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_PointToCoords( context ,surfaceForP ,p ,surfaceForCoords ,coords );
}

TINYFUN OsError OsGuiFontCreate( OsHandle *handle ,const char_t *faceName ,int pointSize ,int weight ,int style ,int family )
{
	struct OsGuiSystemTable *guiTable = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( guiTable == NULL ) return ENOSYS;

	return guiTable->_FontCreate( handle ,faceName ,pointSize ,weight ,style ,family );
}

TINYFUN void OsGuiFontCalcSize( OsHandle handle ,const char_t *text ,struct OsPoint *size )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_FontCalcSize( handle ,text ,size );
}

TINYFUN void OsGuiSetFont( OsGuiContext context ,OsHandle fontHandle )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_SetFont( context ,fontHandle );
}

TINYFUN OsError OsGuiImageCreate( OsHandle *handle ,int width ,int height )
{
	struct OsGuiSystemTable *guiTable = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( guiTable == NULL ) return ENOSYS;

	return guiTable->_ImageCreate( handle ,width ,height );
}

TINYFUN void OsGuiImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return; }

	p->_guiTable->_ImageGetInfo( handle ,info );
}

TINYFUN OsError OsGuiImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return EINVAL; }

	return p->_guiTable->_ImageWrite( handle ,pixelValues ,size );
}

TINYFUN void OsGuiSetAlphaBlend( OsGuiContext context ,float alpha )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_SetAlphaBlend( context ,alpha );
}

TINYFUN void OsGuiDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawImage( context ,left ,top ,right ,bottom ,handle ,imageLeft ,imageTop ,imageRight ,imageBottom );
}

TINYFUN void OsGuiDrawRectangle( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawRectangle( context ,left ,top ,right ,bottom );
}

TINYFUN void OsGuiDrawEllipse( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawEllipse( context ,left ,top ,right ,bottom );
}

TINYFUN void OsGuiDrawLine( OsGuiContext context ,int left ,int top ,int right ,int bottom )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawLine( context ,left ,top ,right ,bottom );
}

TINYFUN void OsGuiDrawText( OsGuiContext context ,const char_t *text ,int left ,int top ,int right ,int bottom ,int align ,struct OsRect *rect )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawText( context ,text ,left ,top ,right ,bottom ,align ,rect );
}

TINYFUN void OsGuiDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawPolygon( context ,npoints ,points );
}

TINYFUN int OsGuiResourceGetType( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return 0; }

	return p->_guiTable->_ResourceGetType( handle );
}

TINYFUN OsError OsGuiResourceLoadFromMemory( OsHandle *handle ,int resourceTypeHint ,uint8_t *memory ,const void *resourceInfo )
{
	struct OsGuiSystemTable *gui = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( gui == NULL ) return ENOSYS;

	return gui->_ResourceLoadFromMemory( handle ,resourceTypeHint ,memory ,resourceInfo );
}

TINYFUN OsError OsGuiResourceLoadFromFile( OsHandle *handle ,int resourceTypeHint ,const char_t *filename )
{
	struct OsGuiSystemTable *gui = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( gui == NULL ) return ENOSYS;

	return gui->_ResourceLoadFromFile( handle ,resourceTypeHint ,filename );
}

TINYFUN OsError OsGuiResourceLoadFromApp( OsHandle *handle ,int resourceTypeHint ,int resourceId ,const char_t *application )
{
	struct OsGuiSystemTable *gui = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

	if( gui == NULL ) return ENOSYS;

	return gui->_ResourceLoadFromApp( handle ,resourceTypeHint ,resourceId ,application );
}

//////////////////////////////////////////////////////////////////////////
enum OsHandleType guiHandleGetType( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return osNotAnHandle; }

	return p->_guiTable->_HandleGetType( handle );
}

static OsError guiHandleDestroy( OsHandle *handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( *handle );

	if( p == OS_INVALID_HANDLE ) { _ASSERT( FALSE ); return EINVAL; }

	return p->_guiTable->_HandleDestroy( handle );
}

//////////////////////////////////////////////////////////////////////////
TINYFUN enum OsHandleType OsHandleGetType( OsHandle handle )
{
	struct AnyHandle *p = (struct AnyHandle *) handle;

	if( p == NULL ) return osNotAnHandle;

	switch( p->_magic )
	{
	case THREADHANDLE_MAGIC: return osThreadHandle;
	case SEMAPHOREHANDLE_MAGIC: return osSemaphoreHandle;
	case CRITICALSECTIONHANDLE_MAGIC: return osSectionHandle;
	case FILEHANDLE_MAGIC: return osFileHandle;
	case NETHANDLE_MAGIC: return osNetHandle;
	case GUISYSTEMHANDLE_MAGIC: return guiHandleGetType( handle );
	default:
		return osNotAnHandle;
	}
};

TINYFUN OsError OsHandleDestroy( OsHandle *handle )
{
	struct AnyHandle *p = * (struct AnyHandle **) handle;

	if( p == NULL ) return EINVAL;

	switch( p->_magic )
	{
	case THREADHANDLE_MAGIC:
	case SEMAPHOREHANDLE_MAGIC:
	case FILEHANDLE_MAGIC:
		return win_closehandle( handle );

	case CRITICALSECTIONHANDLE_MAGIC: 
		return DeleteCriticalSectionHandle( handle );
	case NETHANDLE_MAGIC: 
		return DeleteNetHandle( handle );
	case GUISYSTEMHANDLE_MAGIC: 
		return guiHandleDestroy( handle );

	default:
		return EINVAL;
	}
}

//////////////////////////////////////////////////////////////////////////
//