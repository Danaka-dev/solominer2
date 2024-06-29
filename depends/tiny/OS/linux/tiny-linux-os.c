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
#define SIZEOFARRAY(__p_)       (sizeof(__p_)/sizeof(__p_[0]))
#define LASTOFARRAY(__p_)       (__p_[SIZEOFARRAY(__p_)-1])

#define max(x,y)    (((x) > (y)) ? (x) :(y))
#define min(x,y)    (((x) < (y)) ? (x) :(y))

//////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define __USE_GNU
#include <pthread.h>
#include <X11/Xlib.h>

//////////////////////////////////////////////////////////////////////////
#include "../../tiny-os.h"

#include "tiny-linux-defs.h"

//////////////////////////////////////////////////////////////////////////
//! Tool functions

int32_t log2i( int32_t x )
{
    int32_t i;

    if( x == 0 ) return 0;

    i = __builtin_clz( (unsigned int) x );

    return 32 - i;
}

int32_t log2ceil( int32_t x )
{
    return log2i(x) + (x&(x-1)?1:0);
}

//////////////////////////////////////////////////////////////////////////
const char_t *OsErrorGetText( OsError error )
{
	return (char_t*) strerror( error ); //TODO adapt to char_t ... UNICODE
}

//////////////////////////////////////////////////////////////////////////
static void cpuID( unsigned int i ,unsigned regs[4] )
{
#ifdef _WIN32
	__cpuid( (int*) regs ,(int) i );

#else
	/* asm volatile
		("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
		: "a" (i), "c" (0)); */
	// ECX is set to zero for CPUID function 4
#endif
}

OsError OsSystemGetInfo( struct OsSystemInfo *info )
{
    unsigned int regs[4];
    
	info->_logicalCoreCount = (uint32_t) sysconf( _SC_NPROCESSORS_ONLN );

	//-- get CPU features
	cpuID( 1 ,regs );

	if( (regs[3] & (1 << 28)) != 0 ) //! regs[3] = EDX = cup features
		info->_physicalCoreCount = info->_logicalCoreCount >> 1;
	else
		info->_physicalCoreCount = info->_logicalCoreCount;

	return ENOERROR;
}

// Display *_linux_peekdisplay( void );

// Display *_linux_getdisplay( void );

struct GuiWindowHandle;

struct xWindowNode
{
    Window _xwindow;
    
    struct GuiWindowHandle *_handle;
    
    struct xWindowNode *_next;
};

__thread struct xWindowNode *_xWindowHead = NULL;

OsError LinuxAddWindowToThread( Window window ,struct GuiWindowHandle *handle )
{
    struct xWindowNode *p = _xWindowHead;  
    
    struct xWindowNode *n = (struct xWindowNode*) malloc( sizeof(struct xWindowNode) );
    
    if( n == NULL ) return ENOMEM;
    
    n->_xwindow = window;
    n->_handle = handle;
    n->_next = p;
    
    _xWindowHead = n;
    
    return ENOERROR;
}

//TODO remove

extern OsError _linux_guiwindowproc( struct GuiWindowHandle *handle ,XEvent *event );

static OsTimerTime g_osSystemGlobalTimerInterval = 0;

OsError OsSystemSetGlobalTimer( uint32_t ms ) {
    OsTimerTime frequencyMs = OsTimerGetResolution();

    g_osSystemGlobalTimerInterval = ms * frequencyMs;
}

OsError OsSystemDoEvents( void ) {
    Display *xdisplay = NULL;
    
    XEvent xevent;
    
    // Bool doExpose = False;
    
    long eventMask =
        ExposureMask | StructureNotifyMask
        | SubstructureNotifyMask | VisibilityChangeMask // | PropertyChangeMask
        | EnterWindowMask | LeaveWindowMask
        | ButtonPress | ButtonReleaseMask | ButtonMotionMask
        | KeyPressMask | KeyReleaseMask // | KeymapStateMask
        | PointerMotionMask // | ColormapChangeMask
    ;
                          
    //! using per thread window logic (match win32)
    struct xWindowNode *p = _xWindowHead;
    struct xWindowNode *pprev = NULL;

    static OsTimerTime lastTime = 0;

    OsTimerTime now = OsTimerGetTimeNow();

    if( _linux_peekdisplay() == NULL ) return ENOERROR;
    
    xdisplay = _linux_getdisplay();

///-- events
    while( XPending( xdisplay ) ) {
        XNextEvent( xdisplay ,&xevent );

        p = _xWindowHead;

        while( p != NULL ) {
            if( xevent.xany.window == p->_xwindow && XFilterEvent( &xevent ,p->_xwindow ) == False ) {
                _linux_guiwindowproc( p->_handle ,&xevent );

                if( xevent.type == DestroyNotify ) {
                    if( pprev == NULL )
                        _xWindowHead = _xWindowHead->_next;
                    else
                        pprev->_next = p->_next;
                }

                break; //! found event owner
            }

            pprev = p; p = p->_next;
        }
    }

    //TODO return interrupt on global quit

///-- timers
    if( lastTime == 0 || OsTimerGetElapsed(lastTime,now) > g_osSystemGlobalTimerInterval ) {
        xevent.type = GenericEvent;

        xevent.xclient.message_type = XTIME_EVENT;
        xevent.xclient.send_event = False;

        xevent.xclient.format = 32;
        xevent.xclient.data.l[0] = (long) (OsEventTime) OsTimerConvertToMs(now);
        xevent.xclient.data.l[1] = (long) (OsEventTime) OsTimerConvertToMs(lastTime);

        p = _xWindowHead;

        while( p != NULL ) {
            _linux_guiwindowproc( p->_handle ,&xevent );
            p = p->_next;
        }

        lastTime = now;
    }

	return ENOERROR;
}

OsError OsSystemPostQuit( uint32_t exitCode )
{
	// PostQuitMessage( exitCode );
    // TODO

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
void getMemory(
        int* currRealMem, int* peakRealMem,
        int* currVirtMem, int* peakVirtMem) {

    // stores each word in status file
    char buffer[1024] = "";

    // linux file contains this-process info
    FILE* file = fopen("/proc/self/status", "r");

    // read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {

        if (strcmp(buffer, "VmRSS:") == 0) {
            fscanf(file, " %d", currRealMem);
        }
        if (strcmp(buffer, "VmHWM:") == 0) {
            fscanf(file, " %d", peakRealMem);
        }
        if (strcmp(buffer, "VmSize:") == 0) {
            fscanf(file, " %d", currVirtMem);
        }
        if (strcmp(buffer, "VmPeak:") == 0) {
            fscanf(file, " %d", peakVirtMem);
        }
    }
    fclose(file);
}
*/

OsError OsMemoryGetInfo( struct OsMemoryInfo *info ) {
    /* MEMORYSTATUSEX memInfo;

    memInfo.dwLength = sizeof(memInfo);

    GlobalMemoryStatusEx( &memInfo );

    info->_memoryLoad = memInfo.dwMemoryLoad;
    info->_memoryPhysicalTotal = memInfo.ullTotalPhys;
    info->_memoryPhysicalAvail = memInfo.ullAvailPhys;
    info->_memoryVirtualTotal = memInfo.ullTotalVirtual;
    info->_memoryVirtualAvail = memInfo.ullAvailVirtual;
    */

    //TODO

    return ENOEXEC;
}

size_t __align_size( size_t size ,size_t pow2align )
{
    size_t align = ((size_t)1) << (pow2align-1);

    size_t mod = size % align;

    return mod ? (size + align - mod) : size;
}

void *__align_ptr( void *ptr ,size_t pow2align ,size_t headerSize )
{
    size_t align = ((size_t) 1) << (pow2align-1);

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
    size_t align = ((size_t)1) << (pow2align-1);

    size_t headerSize = sizeof(struct MemBlockHeader);
    size_t allocSize = headerSize + size + (align-1);

    struct MemBlockHeader *header;

    void *baseptr ,*alignptr;

    if( size == 0 ) return NULL;

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
    size_t align = ((size_t)1) << (pow2align-1);

    size_t headerSize = sizeof(struct MemBlockHeader) ,allocSize;

    struct MemBlockHeader *header;

    void *newptr ,*alignptr;

    if( memory == NULL ) return NULL;

    header = (struct MemBlockHeader *) ((uint8_t*) memory - headerSize);

    allocSize = size + headerSize + align-1;

    if( size == 0 )
    {
        free( header->_baseptr );

        return NULL;
    }

    newptr = realloc( header->_baseptr ,allocSize );

    if( newptr == NULL ) return NULL;

    alignptr = __align_ptr( newptr ,pow2align ,headerSize );

    header = (struct MemBlockHeader *) ((uint8_t*) alignptr - headerSize);

    header->_baseptr = newptr;

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
//! Threading

//////////////////////////////////////////////////////////////////////////
//-- asynch

// #define _CACHE_LINE_SIZE			64
// #define __alignCache				__attribute__((aligned(_CACHE_LINE_SIZE)))
// #define CACHE_PAD(_n_,_x_)			char _pad##_n_[_CACHE_LINE_SIZE - (sizeof(_x_)%_CACHE_LINE_SIZE)]

//////////////////////////////////////////////////////////////////////////
/* #ifdef _LP64
 #define __align					__attribute__((aligned(8)))
 #define _ASYNCH_DECL               volatile long long
 #define _ASYNCH_TYPE				unsigned long long
 #define _ASYNCH_SIGNEDTYPE			signed long long
 #define _ASYNCH_VARIABLE			__attribute__((aligned(8))) volatile unsigned long long

#else
 #define __align					__attribute__((aligned(4)))
 #define _ASYNCH_DECL               volatile long
 #define _ASYNCH_TYPE				unsigned long
 #define _ASYNCH_SIGNEDTYPE			signed long
 #define _ASYNCH_VARIABLE			__attribute__((aligned(4))) volatile unsigned long

#endif

#define _ASYNCH_ADD(__x,__y)		__sync_fetch_and_add((_ASYNCH_DECL*) &__x,__y)	//! returns the new added value
#define _ASYNCH_READ(__x)			__sync_fetch_and_add((_ASYNCH_DECL*)&__x,0)	//! returns the variable (initial) value
//!TOCHECK __sync_synchronize(); //IE is only acquire __sync_lock_test_and_set
#define _ASYNCH_WRITE(__x,__y)		__sync_lock_test_and_set((_ASYNCH_DECL*)&__x,__y)	//! returns the variable (initial) value
#define _ASYNCH_INCREMENT(__x)		__sync_add_and_fetch((_ASYNCH_DECL*)&__x,1)		//! returns the new incremented value
#define _ASYNCH_DECREMENT(__x)		__sync_add_and_fetch((_ASYNCH_DECL*)&__x,-1)		//! returns the new decremented value
*/

//////////////////////////////////////////////////////////////////////////
//-- process
#define PROCESSHANDLE_MAGIC	0x0A326FB7E

struct ProcessHandle
{
    uint32_t _magic;

    pid_t _handle;
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

    switch( p->_handle = fork() ) {

        case 0: //! child
        {
            int i=0; if( envp ) while( envp[i] != NULL ) {
                putenv( (char*) envp[i] );
            }

            exit( execvp( path ,(char * const*) argv ) );

            exit( EXIT_SUCCESS );

            break;
        }

        default: //! parent
            return (p->_handle > 0) ? ENOERROR : EFAILED;
    }

    return EFAILED;
}

OsError OsProcessKill( OsHandle *handle ,int exitCode ) {
    struct ProcessHandle *p = CastProcessHandle( handle );

    if( p == NULL ) return EINVAL;

    if( p->_handle == 0 ) return EINVAL;

    kill( p->_handle ,SIGKILL );

    return ENOSYS;
}

//////////////////////////////////////////////////////////////////////////
//-- thread
#define THREADHANDLE_MAGIC	0x02F5ACC15

struct ThreadHandle
{
	uint32_t _magic;

	pthread_t _handle;

	// DWORD _id;

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

	p->_handle = 0;

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

static OsError CallThreadFunction( struct ThreadHandle *handle ,enum OsExecuteAction action )
{
	struct OsEventMessage msg;

	if( handle==NULL ) return EINVAL;

	memset( (void*) &msg ,0 ,sizeof(msg) );

	msg.eventType = osExecuteEvent;

	msg.executeMessage.executeAction = action;

	return handle->_function( &msg ,handle->_userData );
}

static void *_linux_threadproc( void *parameter )
{
	struct ThreadHandle *handle = CastThreadHandle( parameter );

    intptr_t p = (intptr_t) CallThreadFunction( handle ,osExecuteStart );
    
	return (void*) p;
}

//-- thread functions
OsError OsThreadCreate( OsHandle *handle ,uint32_t flags ,OsEventFunction function ,void *userData )
{
	struct ThreadHandle *p = NULL;

    if( flags & OS_THREAD_CREATESUSPENDED ) return ENOSYS;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewThreadHandle( function ,userData );

	if( p == NULL ) return ENOMEM;

    int error = pthread_create( &(p->_handle) ,NULL ,_linux_threadproc ,(void*) p );

	if( error != ENOERROR )
	{
		free( (void*) p );

		return error;
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

/* static int PlatformPriorityMap[] = {
	THREAD_PRIORITY_NORMAL ,THREAD_PRIORITY_IDLE ,THREAD_PRIORITY_LOWEST ,THREAD_PRIORITY_BELOW_NORMAL
	,THREAD_PRIORITY_NORMAL
	,THREAD_PRIORITY_ABOVE_NORMAL ,THREAD_PRIORITY_HIGHEST ,THREAD_PRIORITY_TIME_CRITICAL
}; */

OsError OsThreadSetPriority( OsHandle handle ,enum OsThreadPriority priority )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	//int osPriority = PlatformPriorityMap[priority % (sizeof(PlatformPriorityMap)/sizeof(int)) ];

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

    //TODO pthread_setschedprio()
    
    return ENOSYS;
}

OsError OsThreadSuspend( OsHandle handle )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;
	
    //TODO
    
	return ENOSYS;
}

OsError OsThreadResume( OsHandle handle )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

    //TODO
    
	return ENOSYS;
}

OsError OsThreadTerminate( OsHandle handle ,uint32_t exitCode )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

	return (pthread_kill( p->_handle ,SIGKILL ) == 0) ? ENOERROR : EFAILED;
}

OsError OsThreadStop( OsHandle handle ,int32_t timeoutMs )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

    //TODO

	return ENOSYS;
}

OsError OsThreadQuit( OsHandle handle )
{
    struct ThreadHandle *p = CastThreadHandle( handle );

    if( p == NULL ) return EINVAL;

    if( p->_handle == 0 ) return EINVAL;

    return (pthread_cancel( p->_handle ) == 0) ? ENOERROR : EFAILED;
}

OsError OsThreadGetExitCode( OsHandle handle ,uint32_t *exitCode )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

    //TOOD
	
	return ENOSYS;
}

OsError OsThreadGetId( OsHandle handle ,uint32_t *id )
{
	struct ThreadHandle *p = CastThreadHandle( handle );

	if( p == NULL ) return EINVAL;

	if( p->_handle == 0 ) return EINVAL;

	if( id == NULL ) return EINVAL;

	*id = p->_handle;

	return ENOSYS;
}

OsError OsThreadGetSelfId( uint32_t *id )
{
	if( id == NULL ) return EINVAL;

	*id = (uint32_t) pthread_self();

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//-- semaphore
#define SEMAPHOREHANDLE_MAGIC	0x0A91CB4A3

struct SemaphoreHandle
{
	uint32_t _magic;

	sem_t *_handle;
};

static struct SemaphoreHandle *NewSemaphoreHandle( void )
{
	const size_t size = sizeof(struct SemaphoreHandle);

	struct SemaphoreHandle *p = (struct SemaphoreHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = SEMAPHOREHANDLE_MAGIC;

    assert( p->_handle == SEM_FAILED );

	return p;
}

static struct SemaphoreHandle *CastSemaphoreHandle( OsHandle handle )
{
	struct SemaphoreHandle *p = (struct SemaphoreHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != SEMAPHOREHANDLE_MAGIC ) return NULL;

	return p;
}

//-- semaphore function
OsError OsSemaphoreCreate( OsHandle *handle ,uint32_t initialCount ,const char_t *name )
{
	struct SemaphoreHandle *p = NULL;

    OsError error = ENOERROR;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewSemaphoreHandle();

	if( p == NULL ) return ENOMEM;

    if( name == NULL )
    {
        p->_handle = (sem_t*) malloc(sizeof(sem_t)); //! TODO properly free this
        
        if( sem_init( p->_handle ,0 ,0 ) == -1 )
            error = errno;
    }
    else
    {
        p->_handle = sem_open( name ,O_CREAT ,O_RDWR ,0 );
        
        if( p->_handle == SEM_FAILED )
            error = errno;
    }

	if( error != ENOERROR )
		free( (void*) p );

    else 
        *handle = (OsHandle) p;

	return error;
}

OsError OsSemaphoreLock( OsHandle handle ,int32_t timeoutMs )
{
	struct SemaphoreHandle *p = CastSemaphoreHandle( handle );

    struct timespec abstime;
    
    int retval = 0;
    
	if( p == NULL ) return EINVAL;
    
    if( p->_handle == SEM_FAILED ) return EINVAL;
    
    if( timeoutMs != OS_TIMEOUT_INFINITE )
    {
        abstime.tv_sec = timeoutMs / 1000;
        
        abstime.tv_nsec = timeoutMs - (1000*abstime.tv_sec);
        
        retval = sem_timedwait( p->_handle ,&abstime );
    }
    else
        retval = sem_wait( p->_handle );
        
    if( retval != 0 )
        return errno;
        
    return ENOERROR;
}

OsError OsSemaphoreUnlock( OsHandle handle )
{
	struct SemaphoreHandle *p = CastSemaphoreHandle( handle );

    int retval = 0;
    
	if( p == NULL ) return EINVAL;
    
    if( p->_handle == SEM_FAILED ) return EINVAL;

    return sem_post( p->_handle );
    
    return (retval == 0) ? ENOERROR : errno;
}

//////////////////////////////////////////////////////////////////////////
//-- critical section
#define CRITICALSECTIONHANDLE_MAGIC	0x035F37CD5

// static pthread_mutex_t _mutex_initializer = PTHREAD_MUTEX_INITIALIZER; // RECURSIVE

struct CriticalSectionHandle
{
	uint32_t _magic;

	pthread_mutex_t _handle;
};

static struct CriticalSectionHandle *NewCriticalSectionHandle( void )
{
	const size_t size = sizeof(struct CriticalSectionHandle);

	struct CriticalSectionHandle *p = (struct CriticalSectionHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = CRITICALSECTIONHANDLE_MAGIC;

    pthread_mutex_init( &p->_handle ,NULL );
    
	return p;
}

static struct CriticalSectionHandle *CastCriticalSectionHandle( OsHandle handle )
{
	struct CriticalSectionHandle *p = (struct CriticalSectionHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != CRITICALSECTIONHANDLE_MAGIC ) return NULL;

	return p;
}

//-- critical section function
OsError OsCriticalSectionCreate( OsHandle *handle )
{
	struct CriticalSectionHandle *p = NULL;

	if( handle == NULL ) return EINVAL;

	if( *handle != OS_INVALID_HANDLE ) return EEXIST;

	p = NewCriticalSectionHandle();

	if( p == NULL ) return ENOMEM;

    *handle = (OsHandle) p;

	return ENOERROR;
}

OsError OsCriticalSectionTryEnter( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

    return pthread_mutex_trylock( &p->_handle );
}

OsError OsCriticalSectionEnter( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

	return pthread_mutex_lock( &p->_handle );
}

OsError OsCriticalSectionLeave( OsHandle handle )
{
	struct CriticalSectionHandle *p = CastCriticalSectionHandle( handle );

	if( p == NULL ) return EINVAL;

    return pthread_mutex_unlock( &p->_handle );
}

//////////////////////////////////////////////////////////////////////////
//! timing

static OsTimerTime _frequencyMs;

// static int _timerInitialized = 0;

OsTimerCycle OsTimerGetResolution( void )
{
	struct timespec res;

	int retval = 0;

    retval = clock_getres( CLOCK_REALTIME ,&res );
    
    assert( retval == 0 );

    OsTimerCycle timerTime = res.tv_sec;
    
    // timerTime = timerTime * 1000 * 1000 + res.tv_nsec;
    timerTime = res.tv_nsec;
    
    _frequencyMs = timerTime * 1000*1000 ;
    
    return _frequencyMs;
}

OsTimerCycle OsTimerGetTimeNow( void )
{
    struct timespec now;
    
    clock_gettime( CLOCK_REALTIME ,&now );

    OsTimerCycle timerTime = now.tv_sec;
    
    timerTime = timerTime * 1000*1000*1000 + now.tv_nsec;
    
    return timerTime; //! in nanoseconds
}

OsTimerCycle OsTimerGetElapsed( OsTimerCycle fromTime ,OsTimerCycle toTime )
{
    return (OsTimerTime) (toTime - fromTime);
}

OsTimerTime OsTimerConvertToMs( OsTimerCycle cycles )
{
    return (OsTimerTime) (cycles / _frequencyMs);
}

OsTimerTime OsTimerConvertToNanosec( OsTimerCycle cycles ) {
    return (OsTimerTime) (cycles);
}

//////////////////////////////////////////////////////////////////////////
// static pthread_mutex_t _thread_wait_mutex = PTHREAD_MUTEX_INITIALIZER;

OsError OsWaitForObject( OsHandle handle ,int32_t timeoutMs )
{
	struct ThreadHandle *p = CastThreadHandle( handle );
	//TODO wait for other handle as well
    
    struct timespec timeUntil;  
    
    int retval = 0;
    
    if( p == NULL ) return EINVAL;

    if( timeoutMs != OS_TIMEOUT_INFINITE )
    {
        clock_gettime( CLOCK_REALTIME ,&timeUntil );
        
        timeUntil.tv_sec += timeoutMs / 1000;
        
        timeUntil.tv_nsec += timeoutMs % 1000;
        
        retval = pthread_timedjoin_np( p->_handle ,NULL ,&timeUntil );
    }
    else
    {
        //sleep( 5000 );
        
        retval = pthread_join( p->_handle ,NULL );
    }
        
    return (retval == 0) ? ENOERROR : EFAILED;
}

OsError OsSleep( int32_t delayMs ) {
	usleep(delayMs);
}

//////////////////////////////////////////////////////////////////////////
#define FILEHANDLE_MAGIC	0x0A48BC441

struct FileHandle
{
	uint32_t _magic;

	FILE *_file;
};

static struct FileHandle *NewFileHandle( void )
{
	const size_t size = sizeof(struct FileHandle);

	struct FileHandle *p = (struct FileHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = FILEHANDLE_MAGIC;

	return p;
}

static struct FileHandle *CastFileHandle( OsHandle handle )
{
	struct FileHandle *p = (struct FileHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != FILEHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteFileHandle( OsHandle *handle )
{
	struct FileHandle *p = (struct FileHandle *) handle;

    if( handle == OS_INVALID_HANDLE ) return ENOERROR;
    
	if( p == NULL ) return EINVAL;

	if( p->_file != NULL ) fclose( p->_file );

    *handle = OS_INVALID_HANDLE;
    
    free( p );
    
	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
#include <unistd.h>

const char *_c_createFile( const char_t *filename )
{
    FILE *f = fopen( filename ,"w+b" );

    if( f != NULL ) fclose( f );
}

const char *_c_mapfilemode( const char_t *filename ,int accessMode ,int createMode )
{
/*
#define OS_CREATE_DONT		0		//! fail if doesn't exist, open if exist
#define OS_CREATE_NOEXIST	1		//! create if doesn't exist, open if exist
#define OS_CREATE_NEW		3		//! create if doesn't exist, fail if exist
#define OS_CREATE_TRUNCATE	4		//! fail if doesn't exist, truncate if exist
#define OS_CREATE_ALWAYS	6		//! create if doesn't exist, truncate if exist
*/

    int exists = (int) ( access( filename ,F_OK ) == 0 );

    if( exists ) {
        switch( createMode ) {
            case OS_CREATE_NEW:
                return NULL;
        }
    } else {
        switch( createMode ) {
            case OS_CREATE_NOEXIST:
                _c_createFile( filename );
                break;

            case OS_CREATE_TRUNCATE:
            case OS_CREATE_DONT:
                return NULL;
        }
    }

    switch( createMode )
    {
    case OS_CREATE_DONT:
    case OS_CREATE_NOEXIST:
        if( (accessMode & OS_ACCESS_WRITE) !=0 )
            return "r+b";
        else
            return "rb";

    case OS_CREATE_NEW:
    case OS_CREATE_TRUNCATE:
    case OS_CREATE_ALWAYS:
        if( (accessMode & OS_ACCESS_WRITE) !=0 )
            return "w+b";
        else
            return "wb";

    default:
        return NULL;
    }

}

TINYFUN OsError OsFileOpen( OsHandle *handle ,const char_t *filename ,int access ,int share ,int create ,struct OsFileAttributes *attributes )
{
	struct FileHandle *p = NULL;

    const char *modes = _c_mapfilemode( filename ,access ,create );
    
	if( handle == NULL || filename == NULL ) return EINVAL;

    if( *handle != OS_INVALID_HANDLE ) return EEXIST;
    
    if( modes == NULL ) return ENOSYS;
    
    p = NewFileHandle();
    
    if( p == NULL ) return ENOMEM;   
   
    p->_file = fopen( filename ,modes );
    
    if( p->_file == NULL )
    {
        free( p );
        
        return errno;
    }
    
    *handle = (OsHandle) p;
    
	return ENOERROR;
}

TINYFUN OsError OsFileRead( OsHandle handle ,uint8_t *memory ,size_t size ,size_t *readSize )
{
    struct FileHandle *p = CastFileHandle( handle );
    
    size_t read = 0;
    
    if( p == NULL || memory == NULL ) return EINVAL;
    
    if( p->_file == NULL ) return EINVAL;
    
    read = size ? fread( memory ,1 ,size ,p->_file ) : 0;
    
    if( readSize ) *readSize = read;
    
    return (read == size) ? ENOERROR : EFAILED;
}

TINYFUN OsError OsFileWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,size_t *writeSize )
{
    struct FileHandle *p = CastFileHandle( handle );
    
    size_t write = 0;
    
    if( p == NULL || memory == NULL ) return EINVAL;
    
    if( p->_file == NULL ) return EINVAL;
    
    write = size ? fwrite( memory ,1 ,size ,p->_file ) : 0;
    
    if( writeSize ) *writeSize = write;

    return (write == size) ? ENOERROR : EFAILED;
}

TINYFUN OsError OsFileFlush( OsHandle handle )
{
    struct FileHandle *p = CastFileHandle( handle );

    size_t write = 0;

    if( p == NULL ) return EINVAL;

    if( p->_file == NULL ) return EINVAL;

    return fflush( p->_file ) == 0 ? ENOERROR : EFAILED;
}

TINYFUN OsError OsFileGetSize( OsHandle handle ,uint64_t *size )
{
    struct FileHandle *p = CastFileHandle( handle );

    size_t pos = 0;

    if( p == NULL || size == NULL ) return EINVAL;

    if( p->_file == NULL ) return EINVAL;

    pos = ftell( p->_file ); //! get file pointer

    fseek( p->_file ,0 ,SEEK_END ); //! seek to end of file

    *size = (uint64_t) ftell(  p->_file ); //! get position

    fseek( p->_file ,(long) pos ,SEEK_SET ); //! restore file pointer

    return ENOERROR;
}

TINYFUN OsError OsFileSetSize( OsHandle handle ,uint64_t size ) {
    struct FileHandle *p = CastFileHandle( handle );

    size_t pos = 0;

    if( p == NULL ) return EINVAL;

    if( p->_file == NULL ) return EINVAL;

    fflush( p->_file );

    //TODO support for 64 bit file size

    return ftruncate( fileno(p->_file) ,(off_t) size ) == 0 ? ENOERROR : EFAILED;
}

int _c_mapSeekPos( int seekpos ) {
    switch( seekpos ) {
        default:
        case OS_SEEK_BEGIN : return SEEK_SET;
        case OS_SEEK_CURPOS : return SEEK_CUR;
        case OS_SEEK_END : return SEEK_END;
    }
}

TINYFUN OsError OsFileSeek( OsHandle handle ,size_t distance ,int seekpos )
{
    struct FileHandle *p = CastFileHandle( handle );

    int whence = _c_mapSeekPos(seekpos);

    if( p == NULL ) return EINVAL;

    if( p->_file == NULL ) return EINVAL;

    return fseek( p->_file ,(long) distance ,whence ) == 0 ? ENOERROR : EFAILED;
}

TINYFUN OsError OsFileTell( OsHandle handle ,size_t *position )
{
    struct FileHandle *p = CastFileHandle( handle );

    if( p == NULL || position == NULL ) return EINVAL;

    if( p->_file == NULL ) return EINVAL;

    *position = (long) ftell( p->_file );

    return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//! Network

#define NETHANDLE_MAGIC	0x0D4F7A93B

struct NetHandle
{
	uint32_t _magic;

	int _socket;
};

static struct NetHandle *NewNetSocketHandle( void )
{
	const size_t size = sizeof(struct NetHandle);

	struct NetHandle *p = (struct NetHandle*) malloc(size);

	if( p == NULL ) return NULL;

	memset( p ,0 ,size );

	p->_magic = NETHANDLE_MAGIC;

	return p;
}

static struct NetHandle *CastNetSocketHandle( OsHandle handle )
{
	struct NetHandle *p = (struct NetHandle *) handle;

	if( p == NULL ) return NULL;

	if( p->_magic != NETHANDLE_MAGIC ) return NULL;

	return p;
}

static OsError DeleteNetHandle( OsHandle *handle )
{
	struct NetHandle *p = (struct NetHandle *) handle;

    if( handle == OS_INVALID_HANDLE ) return ENOERROR;
    
	if( p == NULL ) return EINVAL;

	if( p->_socket > 0 ) close( p->_socket );

    *handle = OS_INVALID_HANDLE;
    
    free( p );
    
	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
static ASYNCH_VARIABLE _win_wsaInitialize = 0;

static ASYNCH_VARIABLE _win_wsaReady = 0;

//CRITICAL_SECTION _win_cs_net;

/* int _win_wsaerrormap( int error )
{
	// WSANOTINITIALISED ... // etc
} */

/*int _win_netinit( void )
{
	static int _wsaInitialized = 0;
	
	int error = ENOERROR;

	if( _wsaInitialized != 0 ) return ENOERROR;

	if( _ASYNCH_INCREMENT( _win_wsaInitialize ) == 1 )
	{
		WSADATA wsaData;

		error = WSAStartup( MAKEWORD(2,0) ,&wsaData );

		_ASYNCH_INCREMENT(_win_wsaReady);
	}

	while( _ASYNCH_READ(_win_wsaReady ) == 0 );

	return error;
}*/

//////////////////////////////////////////////////////////////////////////
//-- server

TINYFUN OsError OsNetListen( OsHandle *handle ,const char_t *localAddress )
{
	struct NetHandle *p = NULL;

	struct sockaddr_in sin;

	if( handle == NULL ) return EINVAL;

	p = NewNetSocketHandle();

	if( p == NULL ) return ENOMEM;

	//TODO local address ?
	p->_socket = socket( AF_INET ,SOCK_STREAM ,0 );

	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);

	// ioctlsocket( socket ,FIONBIO ,&bio );

	bind( p->_socket ,(struct sockaddr*) &sin ,sizeof(sin) );

	if( listen( p->_socket ,0 ) < 0 ) // ? 3
	{
		free( p );

		return EFAILED; //! TODO map error
	}

	*handle = (OsHandle) p;

	return ENOERROR;
}

TINYFUN OsError OsNetAccept( OsHandle listenHandle ,OsHandle *handle ,int32_t timeoutMs )
{
	struct NetHandle *listen = CastNetSocketHandle( listenHandle );

	struct NetHandle *p = NULL;

	struct sockaddr_in csin;

	socklen_t csin_size = sizeof(csin);

	if( handle == NULL ) return EINVAL;

	p = NewNetSocketHandle();

	if( p == NULL ) return ENOMEM;

	p->_socket = accept( listen->_socket ,(struct sockaddr*) &csin ,&csin_size );

	if( p->_socket < 0 )
	{
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

	struct sockaddr_in sin;

	if( handle == NULL || peerAddress == NULL ) return EINVAL;

	p = NewNetSocketHandle();

	if( p == NULL ) return ENOMEM;

	//TODO local address ?
	p->_socket = socket( AF_INET ,SOCK_STREAM ,0 );

	sin.sin_addr.s_addr = inet_addr( peerAddress );
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);

	// ioctlsocket( socket ,FIONBIO ,&bio );

	if( connect( p->_socket ,(struct sockaddr*) &sin ,sizeof(sin) ) < 0 )
	{
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
	struct NetHandle *p = CastNetSocketHandle( handle );

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

		if( read < 0 )
			return EFAILED; //TODO map wsa error 

		toread -= read;

	} while( toread != 0 );

	if( readSize != NULL ) *readSize = (size_t) size;

	return ENOERROR;
}

TINYFUN OsError OsNetWrite( OsHandle handle ,const uint8_t *memory ,size_t size ,int32_t timeoutMs )
{
	struct NetHandle *p = CastNetSocketHandle( handle );

	int wsaError = 0;

	if( p == NULL || memory == NULL || size <= 0 ) return EINVAL;

	wsaError = send( p->_socket ,memory ,(int) size ,0 );

	if( wsaError != 0 )
		return EFAILED; //TODO map wsa error 

	return ENOERROR;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! GUI front end 

    //! link with required sub system (GDI, X11, OpenGL ,... )

//////////////////////////////////////////////////////////////////////////
OsError OsConsoleGetInfo( struct OsConsoleInfo *info ) {
    //! TODO
    return ENOEXEC;
}

void OsConsoleSetColor( int selectColor ,OsColorRef color ) {
    //! TODO
}

//////////////////////////////////////////////////////////////////////////
#define _NATIVE_GUI_

#ifdef _NATIVE_GUI_
 extern struct OsGuiSystemTable *_guiSystemX11;
#else
 static struct OsGuiSystemTable *_guiSystemX11 = NULL;
#endif

#ifdef _OPENGL_GUI_
 extern struct OsGuiSystemTable *_guiSystemOPENGL;
#else
 static struct OsGuiSystemTable *_guiSystemOPENGL = NULL;
#endif

#ifdef _OPENGLES_GUI_
 extern struct OsGuiSystemTable *_guiSystemOPENGLES;
#else
 static struct OsGuiSystemTable *_guiSystemOPENGLES = NULL;
#endif

struct OsGuiSystemTable *_guiSystemDIRECTX = NULL; //! never exist on linux

//! native system is default
static int _guiSystemCurrentId = OS_GUISYSTEMID_DEFAULT;

static struct OsGuiSystemTable *guiGetSystemFromId( int guiSystemId )
{
	switch( guiSystemId )
	{
	case OS_GUISYSTEMID_CURRENT:
		return guiGetSystemFromId(_guiSystemCurrentId);

	case OS_GUISYSTEMID_NATIVE: return _guiSystemX11;
	case OS_GUISYSTEMID_OPENGL: return _guiSystemOPENGL;
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

	if( (p->_magic & 0x0FFFF0000)  != GUISYSTEMHANDLE_MAGIC ) return NULL;

	return p;
}

//////////////////////////////////////////////////////////////////////////
TINYFUN OsError OsGuiWindowCreate( OsHandle *handle ,const char_t *name ,const struct OsGuiWindowProperties *properties ,OsEventFunction eventFunction ,void *userData )
{
	int guiSystemId = OS_GUISYSTEMID_NATIVE; // (properties != NULL) ? properties->guiSystemId : OS_GUISYSTEMID_NATIVE;

	struct OsGuiSystemTable *guiTable = guiGetSystemFromId(guiSystemId);

	if( guiTable == NULL ) return ENOSYS;

	return guiTable->_WindowCreate( handle ,name ,properties ,eventFunction ,userData );
}

TINYFUN void OsGuiWindowRefresh( OsHandle handle , const struct OsRect *updateRect ,int update )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

	p->_guiTable->_WindowRefresh( handle ,updateRect, update );
}

TINYFUN void OsGuiWindowShow( OsHandle handle ,int visible )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

	p->_guiTable->_WindowShow( handle ,visible );
}

TINYFUN void OsGuiMouseSetCursor(OsHandle handle, int cursorID )
{
    struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

    if( p == NULL ) { _ASSERT( 0 ); return; }

    p->_guiTable->_MouseSetCursor( handle ,cursorID  );
}

TINYFUN void OsGLDrawPolygon(OsHandle handle)
{
	assert(0);
}

TINYFUN void OsGuiMouseCapture( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

	p->_guiTable->_MouseCapture( handle );
}

TINYFUN void OsGuiMouseRelease( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

	p->_guiTable->_MouseRelease( handle );
}

TINYFUN void OsGuiGetClientArea( OsHandle handle ,struct OsPoint *size )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

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
    struct OsGuiSystemTable *guiTable = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

    if( guiTable == NULL ) return;

    return guiTable->_FontCalcSize( handle ,text ,size );
}

TINYFUN void OsGuiSetFont( OsGuiContext context ,OsHandle fontHandle )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

    if( guiTable == NULL ) return;

	guiTable->_SetFont( context ,fontHandle );
}

TINYFUN OsError OsGuiImageCreate( OsHandle *handle ,int width ,int height ) //, int GUI )
{
	struct OsGuiSystemTable *guiTable = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT); // GUI);

	if( guiTable == NULL ) return ENOSYS;

	return guiTable->_ImageCreate( handle ,width ,height );
}

TINYFUN void OsGuiImageGetInfo( OsHandle handle ,struct OsGuiImageInfo *info )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return; }

	p->_guiTable->_ImageGetInfo( handle ,info );
}

TINYFUN OsError OsGuiImageWrite( OsHandle handle ,const uint8_t *pixelValues ,size_t size )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return ENOSYS; }

	return p->_guiTable->_ImageWrite( handle ,pixelValues ,size );
}

TINYFUN void OsGuiSetAlphaBlend( OsGuiContext context ,float alpha )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_SetAlphaBlend( context ,alpha );
}

TINYFUN void OsGuiDrawImage( OsGuiContext context ,int left ,int top ,int right ,int bottom ,OsHandle handle ,int imageLeft ,int imageTop ,int imageRight ,int imageBottom )
{
	//struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];
	
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );
	
	if( p == NULL ) { /*_ASSERT( 0 );*/ return; }

	p->_guiTable->_DrawImage( context ,left ,top ,right ,bottom ,handle ,
			imageLeft ,imageTop ,imageRight ,imageBottom );

#ifdef __arm__
	p->_guiTable->_WindowSwapBuffers(context);
#endif //#ifdef __arm__
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

	guiTable->_DrawText( context ,text ,left ,top ,right ,bottom ,align, rect );
}

TINYFUN void OsGuiDrawPolygon( OsGuiContext context ,int npoints ,const struct OsPoint *points )
{
	struct OsGuiSystemTable *guiTable = ((struct OsGuiSystemTable**) context)[0];

	guiTable->_DrawPolygon( context ,npoints, points);
}

TINYFUN int OsGuiResourceGetType( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return 0; }

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

//-- clipboard
TINYFUN OsError OsClipboardGetData( OsHandle handle ,char dataType[32] ,void **data ,int *length )
{
    struct OsGuiSystemTable *gui = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

    if( gui == NULL ) return ENOSYS;

    return gui->_ClipboardGetData( handle ,dataType ,data ,length );
}

TINYFUN OsError OsClipboardSetData( OsHandle handle ,char dataType[32] ,void *data ,int length )
{
    struct OsGuiSystemTable *gui = guiGetSystemFromId(OS_GUISYSTEMID_CURRENT);

    if( gui == NULL ) return ENOSYS;

    return gui->_ClipboardSetData( handle ,dataType ,data ,length );
}

//////////////////////////////////////////////////////////////////////////
enum OsHandleType guiHandleGetType( OsHandle handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return osNotAnHandle; }

	return p->_guiTable->_HandleGetType( handle );
}

static OsError guiHandleDestroy( OsHandle *handle )
{
	struct GuiSystemHandle *p = CastGuiSystemHandle( handle );

	if( p == NULL ) { _ASSERT( 0 ); return EINVAL; }

	return p->_guiTable->_HandleDestroy( handle );
}

//////////////////////////////////////////////////////////////////////////
/* struct AnyHandle
{
	uint32_t _magic;
}; */

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
    case GUISYSTEMHANDLE_MAGIC:
        return guiHandleGetType( handle );
	default:
		return osNotAnHandle;
	}
};

TINYFUN OsError OsHandleDestroy( OsHandle *handle )
{
	struct AnyHandle *p = (struct AnyHandle *) handle;

	if( p == NULL ) return EINVAL;

	switch( p->_magic )
	{
    case PROCESSHANDLE_MAGIC:
	case THREADHANDLE_MAGIC:
	case SEMAPHOREHANDLE_MAGIC:
        return ENOSYS;
	case FILEHANDLE_MAGIC:
        return DeleteFileHandle( handle );
	case CRITICALSECTIONHANDLE_MAGIC:
        return ENOSYS;
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