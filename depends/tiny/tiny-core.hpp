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

#ifndef TINY_CORE_H
#define TINY_CORE_H

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! tiny os structure wrappers

//////////////////////////////////////////////////////////////////////////////
//! Point

template <typename T>
struct Point_ : OsPoint { //TODO proper typed members
    Point_() { x = y = 0; }
    Point_( T c ) { x = y = c; }
    Point_( T x ,T y ) { this->x=x; this->y=y; }
    Point_( const OsPoint &p ) { x = (T) p.x; y= (T) p.y; }

    Point_ &operator =( T c ) { x = y = c; return *this; }
    Point_ &operator =( const Point_ &p ) { x = p.x; y = p.y; return *this; }

    Point_ &operator +=( T c ) { x += c; y += c; return *this; }
    Point_ &operator -=( T c ) { x -= c; y -= c; return *this; }
    Point_ &operator *=( T c ) { x *= c; y *= c; return *this; }
    Point_ &operator /=( T c ) { x /= c; y /= c; return *this; }

    Point_ &operator +=( const Point_ &p ) { x += p.x; y += p.y; return *this; }
    Point_ &operator -=( const Point_ &p ) { x -= p.x; y -= p.y; return *this; }
    Point_ &operator *=( const Point_ &p ) { x *= p.x; y *= p.y; return *this; }
    Point_ &operator /=( const Point_ &p ) { x /= p.x; y /= p.y; return *this; }

    Point_ operator +( const Point_ &p ) const { return Point_( x + p.x ,y + p.y ); }
    Point_ operator -( const Point_ &p ) const { return Point_( x - p.x ,y - p.y ); }
    Point_ operator *( const Point_ &p ) const { return Point_( (T) (x * p.x) ,(T) (y * p.y) ); }
    Point_ operator /( const Point_ &p ) const { return Point_( (T) (x / p.x) ,(T) (y / p.y) ); }

    Point_ &Abs() { x = abs(x); y = abs(y); return *this; }

    T Max() { return MAX(x,y); }
    T Min() { return MIN(x,y); }
    T Sum() { return x+y; }
};

typedef Point_<int> Point;

//-- format/parse
template <> OsPoint &fromString( OsPoint &p ,const String &s ,size_t &size );
template <> String &toString( const OsPoint &p ,String &s );

template <> inline Point &fromString( Point &p ,const String &s ,size_t &size ) {
    fromString<OsPoint>( p ,s ,size ); return p;
}

template <> inline String &toString( const Point &p ,String &s ) {
    return toString<OsPoint>( p ,s );
}

//-- serialize
template <> inline std::ostream &operator <<( std::ostream &out ,Bits<const OsPoint&> p ) {
    out << tobits(p().x) << tobits(p().y); return out;
}

template <> inline std::istream &operator >>( std::istream &in ,Bits<OsPoint&> pt ) {
    in >> tobits(pt().x) >> tobits(pt().y); return in;
}

template <> inline std::ostream &operator <<( std::ostream &out ,Bits<const Point&> p ) {
    out << tobits(p().x) << tobits(p().y); return out;
}

template <> inline std::istream &operator >>( std::istream &in ,Bits<Point&> pt ) {
    in >> tobits(pt().x) >> tobits(pt().y); return in;
}

//////////////////////////////////////////////////////////////////////////////
//! Rect

inline int getWidth( const OsRect &r ) { return r.right - r.left; }
inline int GetHeight( const OsRect &r ) { return r.bottom - r.top; }

inline bool TestHit( const OsPoint &p ,const OsRect &r  ) {
    return p.x > r.left && p.x < r.right && p.y > r.top && p.y < r.bottom;
}

//--
template <typename T>
struct Rect_ : OsRect {
    Rect_( int c=0 ) { left = top = right = bottom = c; }
    Rect_( int left ,int top ,int right ,int bottom ) { this->left=left; this->top=top; this->right=right; this->bottom=bottom; }
    Rect_( const Point &p ) { left=p.x; top=p.y; right=p.x; bottom=p.y; }
    Rect_( const Point &topLeft ,const Point &bottomRight ) { left=topLeft.x; top=topLeft.y; right=bottomRight.x; bottom=bottomRight.y; }
    Rect_( const OsRect &r ) { left=r.left; top=r.top; right=r.right; bottom=r.bottom; }

    int getWidth() const { return right - left; }
    int getHeight() const { return bottom - top; }
    Point getSize() const { return Point( getWidth() ,getHeight() ); }
    Rect_ getDims() const { return Rect_( Point(0) ,getSize() ); }

    int getCenterH() const { return (left + right) / 2; }
    int getCenterV() const { return (top + bottom) / 2; }
    Point getCenter() const { return Point( getCenterH() ,getCenterV() ); }

    Point getTopLeft() const { return Point( left ,top ); }
    Point getTopRight() const { return Point( right ,top ); }
    Point getBottomLeft() const { return Point( left ,bottom ); }
    Point getBottomRight() const { return Point( right ,bottom ); }

    Point getTopMid() const { return Point( getCenterH() ,top ); }
    Point getRightMid() const { return Point( right ,getCenterV() ); }
    Point getBottomMid() const { return Point( getCenterH() ,bottom ); }
    Point getLeftMid() const { return Point( left ,getCenterV() ); }

    Rect_ &Inflate( const OsRect &r ) { left -= r.left; right += r.right; top -= r.top; bottom += r.bottom; return *this; }
    Rect_ &Inflate( const OsPoint &p ) { left -= p.x; right += p.y; top -= p.x; bottom += p.y; return *this; }
    Rect_ &Inflate( int c ) { left -= c; right += c; top -= c; bottom += c; return *this; }

    Rect_ &Deflate( const OsRect &r ) { left += r.left; right -= r.right; top += r.top; bottom -= r.bottom; return *this; }
    Rect_ &Deflate( const OsPoint &p ) { left += p.x; right -= p.y; top += p.x; bottom -= p.y; return *this; }
    Rect_ &Deflate( int c ) { left += c; right -= c; top += c; bottom -= c; return *this; }

//-- operators
    Rect_ &operator =( const OsRect &r ) { left = r.left; top = r.top; right = r.right; bottom = r.bottom; return *this; }
    Rect_ &operator =( const OsPoint &p ) { left = right = p.x; top = bottom = p.y; return *this; }
    Rect_ &operator =( int c ) { left = top = right = bottom = c; return *this; }

    //--
    Rect_ &operator +=( const OsRect &r ) { left += r.left; top += r.top; right += r.right; bottom += r.bottom; return *this; }
    Rect_ &operator -=( const OsRect &r ) { left -= r.left; top -= r.top; right -= r.right; bottom -= r.bottom; return *this; }

    Rect_ &operator +=( const OsPoint &p ) { left += p.x; top += p.y; right += p.x; bottom += p.y; return *this; }
    Rect_ &operator -=( const OsPoint &p ) { left -= p.x; top -= p.y; right -= p.x; bottom -= p.y; return *this; }

    Rect_ &operator +=( int c ) { left += c; right += c; top += c; bottom += c; return *this; }
    Rect_ &operator -=( int c ) { left -= c; right -= c; top -= c; bottom -= c; return *this; }

    //--
    Rect_ operator +( const OsRect &p ) { return Rect_( left + p.left ,top + p.top ,right + p.right ,bottom + p.bottom ); }
    Rect_ operator -( const OsRect &p ) { return Rect_( left - p.left ,top - p.top ,right - p.right ,bottom - p.bottom ); }

    Rect_ operator +( const OsPoint &p ) { return Rect_( left + p.x ,top + p.y ,right + p.x ,bottom + p.y ); }
    Rect_ operator -( const OsPoint &p ) { return Rect_( left - p.x ,top - p.y ,right - p.x ,bottom - p.y ); }

    Rect_ operator +( int c ) { return Rect_( left+c ,top+c ,right+c ,bottom+c ); }
    Rect_ operator -( int c ) { return Rect_( left-c ,top-c ,right-c ,bottom-c ); }

    //-- intersect
    Rect_ &operator |=( const OsRect &r ) { left = MIN(left,r.left); top = MIN(top,r.top); right = MAX(right,r.right); bottom = MAX(bottom,r.bottom); return *this; }

    bool operator &( const OsPoint &p ) const { return left <= p.x && top <= p.y && right >= p.x && bottom >= p.y; }
    bool operator &( const OsRect &r ) const { return right > r.left && bottom > r.top && left < r.right  && top < r.bottom; }

    bool operator &=( const Rect_ &r ) {
        if( !operator &(r) ) { *this = 0; return false; }

        left = MAX(left,r.left); top = MAX(top,r.top); right = MIN(right,r.right); bottom = MIN(bottom,r.bottom);

        return true;
    }
};

typedef Rect_<int> Rect;

inline Rect sizeAsRect( const OsPoint &size ) {
    return Rect(0,0,size.x,size.y);
}

//--
enum RectPoint {
    topLeft=0 ,topRight=1 ,bottomRight=2 ,bottomLeft=3
    ,topMid=4 ,rightMid=5 ,bottomMid=6 ,leftMid=7
    ,center=8
};

inline Point getRectPoint( const Rect &r ,RectPoint p ) {
    switch( p ) {
        default:
        case topLeft: return r.getTopLeft();
        case topRight: return r.getTopRight();
        case bottomRight: return r.getBottomRight();
        case bottomLeft: return r.getBottomLeft();
        case topMid: return r.getTopMid();
        case rightMid: return r.getRightMid();
        case bottomMid: return r.getBottomMid();
        case leftMid: return r.getLeftMid();
        case center: return r.getCenter();
    }
}

//--
template <> OsRect &fromString( OsRect &p ,const String &s ,size_t &size );
template <> String &toString( const OsRect &p ,String &s );

template <> inline Rect &fromString( Rect &p ,const String &s ,size_t &size ) {
    fromString<OsRect>( p ,s ,size ); return p;
}

template <> inline String &toString( const Rect &p ,String &s ) {
    return toString<OsRect>( p ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! ColorRef

#define TINY_COLORREF_UUID    0x096ad33bd7c33c280

struct ColorRef {
    ColorRef() : color(0) {}
    ColorRef( const OsColorRef &acolor ) : color(acolor) {}

    operator OsColorRef*() const { return (OsColorRef*) this; }
    operator OsColorRef&() const { return * (OsColorRef*) this; }

    bool operator == ( const OsColorRef c ) const { return color == c; }
    bool operator != ( const OsColorRef c ) const { return color != c; }

    OsColorRef color;
};

DECLARE_TCLASS(ColorRef,TINY_COLORREF_UUID);

bool findColorValueByName( const char *name ,ColorRef &color );
bool matchColorByName( const String &s ,ColorRef &color ,size_t *size );
bool findColorNameByValue( ColorRef color ,String &name );

//--
template <> ColorRef &fromString( ColorRef &p ,const String &s ,size_t &size );
template <> String &toString( const ColorRef &p ,String &s );

//--
inline OsColorRef &colorFromString( OsColorRef &p ,const String &s ,size_t &size ) {
    ColorRef color; fromString( color ,s ,size ); p = color.color; return p;
}

inline String &colorToString( const OsColorRef &p ,String &s ) {
    ColorRef color(p); toString( color ,s ); return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Color

    //TODO

/* struct Color : OsColor {
    Color() { r = g = b = a = 0; }
}; */

//////////////////////////////////////////////////////////////////////////////
//! Time

    //TODO proper timeMs / TimeSec  (NB keep TimeDate and such in tiny-types ?)

inline time_t Now() { return time(NullPtr); }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Threading

//////////////////////////////////////////////////////////////////////////
class System {
    //TODO
};

//////////////////////////////////////////////////////////////////////////
class Memory {
public:
    Memory( int nAlign=32 ) : m_flags(OS_MEMORY_DEFAULT) ,m_size(0) ,m_data(NullPtr) { setAlignBits(nAlign); }
    ~Memory() { Free(); }

    const byte *data() const { return m_data; }
    byte *data() { return m_data; }

    size_t size() const { return m_size; }

public:
    bool operator ==( const Memory &mem ) const { return m_data == mem.m_data; }
    bool operator !=( const Memory &mem ) const { return m_data != mem.m_data; }

    bool isNull() const { return m_data == NullPtr; }

public: //--
    void setAlignBits( int n ) { m_flags = (m_flags&0x0ffffff00) | OS_MEMORY_ALIGN(n); }

    bool Alloc( size_t size ) {
        Free();

        if( !(m_data = (byte*) OsMemoryAlloc( size ,m_flags )) ) return false;

        m_size = size;

        return true;
    }

    bool Realloc( size_t size ) {
        void *p;

        if( !(p = OsMemoryRealloc( m_data ,size ,m_flags )) ) return false;

        m_data = (byte*) p;
        m_size = size;

        return true;
    }

    void Free() {
        if( m_data ) { OsMemoryFree( m_data ); m_data = NullPtr; m_size = 0; }
    }

public: //--
    bool Resize( size_t size ) {
        if( size == 0 ) { Free(); return true; }

        return m_data ? Realloc( size ) : Alloc( size );
    }

    //-- content
    bool Set( byte c ,size_t at ,size_t n );

    bool Read( size_t at ,const byte *p ,size_t n ); //! read n bytes from buffer at offset to *p
    bool Write( size_t at ,byte *p ,size_t &n ) const; //! write n bytes to buffer at offset from *p
    int Compare( size_t at ,const byte *p ,size_t n ) const;

    bool Copy( size_t at ,Memory &to ,size_t n ) const;
    bool Zero();

protected:
    int m_flags;
    size_t m_size;
    byte *m_data;
};

//////////////////////////////////////////////////////////////////////////
class Thread {
public: //-- instance
	Thread() : _id(0) ,_hthread( OS_INVALID_HANDLE )    {}
	~Thread() { Destroy(); }

public: //-- creation
	OsError Create( bool suspended=false );
	OsError Destroy() { if( _hthread != OS_INVALID_HANDLE ) return OsHandleDestroy( &_hthread ); return ENOERROR; }

public: //-- interface
	OsError SetPriority( OsThreadPriority priority ) { return OsThreadSetPriority( _hthread ,priority ); }
	OsError Start() { if( _hthread == OS_INVALID_HANDLE ) return Create( false ); else return Resume(); }
	OsError Suspend() { return OsThreadSuspend( _hthread ); }
	OsError Resume() { return OsThreadResume( _hthread ); }
	OsError Terminate( uint32_t exitCode ) { return OsThreadTerminate( _hthread ,exitCode ); }
	OsError Stop( int32_t msTimeout=OS_TIMEOUT_INFINITE ) { return OsThreadStop( _hthread ,msTimeout ); }
	OsError Quit() { return OsThreadQuit( _hthread ); }

public: //--function
	OsError WaitFor( int32_t msTimeout=OS_TIMEOUT_INFINITE ) { if( _hthread == OS_INVALID_HANDLE ) return EINVAL; return OsWaitForObject( _hthread ,msTimeout ); }

public: //-- accessors
	uint32_t GetId( void ) { return _id; }
	OsHandle GetHandle( void ) { return _hthread; }
	uint32_t GetExitCode( void ) { uint32_t exitCode = 0; OsThreadGetExitCode( _hthread ,&exitCode ); return exitCode; }

public: //-- static
	static uint32_t GetCurrentId( void ) { uint32_t id; OsThreadGetSelfId( &id ); return id; }

public: //-- callback
	virtual OsError Main( void ) = 0;

	virtual OsError OnStop( void ) { return EINVAL; }
	virtual void OnTerminate( void ) {}

protected: //-- members
    uint32_t	_id; //! thread id
    OsHandle	_hthread; //! thread handle
};

////////////////////////////////////////////////////////////////////////////////
class Semaphore {
public: //-- instance
	Semaphore() : _hsemaphore( OS_INVALID_HANDLE ) {}
	~Semaphore() { Destroy(); }

public: //-- interface
	OsError Create( uint32_t initialCount=0 ,char_t *name=NULL ) { Destroy(); return OsSemaphoreCreate( &_hsemaphore ,initialCount ,name ); }
	OsError Destroy( void ) { if( _hsemaphore != OS_INVALID_HANDLE ) return OsHandleDestroy( &_hsemaphore ); return ENOERROR; }

	OsError Lock( uint32_t timeoutMs=OS_TIMEOUT_INFINITE ) { return OsSemaphoreLock( _hsemaphore ,timeoutMs ); }
	OsError Unlock( void ) { return OsSemaphoreUnlock( _hsemaphore ); }

protected: //-- members
    OsHandle	_hsemaphore; //! semaphore handle
};

//////////////////////////////////////////////////////////////////////////
class CriticalSection {
public: // -- instance
	CriticalSection() : _hcs( OS_INVALID_HANDLE ) { OsCriticalSectionCreate( &_hcs ); }
	~CriticalSection() { OsHandleDestroy( &_hcs ); }

public: // -- interface
	bool TryEnter( void ) { return OS_SUCCEED( OsCriticalSectionTryEnter( _hcs ) ); }
	void Enter( void ) { OsCriticalSectionEnter( _hcs ); }
	void Leave( void ) { OsCriticalSectionLeave( _hcs ); }

    struct Guard {
        CriticalSection &cs;

        Guard( CriticalSection &a_cs ) : cs(a_cs) { cs.Enter(); }
        ~Guard() { cs.Leave(); }
    };

protected: // -- member
	OsHandle	_hcs;
};

//////////////////////////////////////////////////////////////////////////
class File {
public:
	File() : m_hfile(OS_INVALID_HANDLE) {}
	~File() { Close(); }

    OsHandle &handle() { return m_hfile; }

    bool isOpen() { return m_hfile != OS_INVALID_HANDLE; }

public:
	OsError Create( const char_t *filename ,int access=OS_ACCESS_ALL ,int share=OS_SHARE_ALL ,int create=OS_CREATE_ALWAYS ,struct OsFileAttributes *attributes=NULL ) {
        return OsFileOpen( &m_hfile ,filename ,access ,share ,create ,attributes );
	}

	OsError Open( const char_t *filename ,int access=OS_ACCESS_ALL ,int share=OS_SHARE_ALL ,int create=OS_CREATE_DONT ,struct OsFileAttributes *attributes=NULL ) {
		return OsFileOpen( &m_hfile ,filename ,access ,share ,create ,attributes );
	}

    OsError GetSize( uint64_t &size ) { return OsFileGetSize( m_hfile ,&size ); }
    uint64_t GetSize() { uint64_t size=0; GetSize(size); return size; }

    OsError SetSize( uint64_t size ) { return OsFileSetSize( m_hfile ,size ); }

    OsError Read( uint8_t *memory ,size_t size ) { return OsFileRead( m_hfile ,memory ,size ,NULL ); }
	OsError Read( uint8_t *memory ,size_t size ,size_t &readSize ) { return OsFileRead( m_hfile ,memory ,size ,&readSize ); }
	OsError Write( const uint8_t *memory ,size_t size ) { return OsFileWrite( m_hfile ,memory ,size ,NULL ); }
	OsError Write( const uint8_t *memory ,size_t size ,size_t &writeSize ) { return OsFileWrite( m_hfile ,memory ,size ,&writeSize ); }
    OsError Flush() { return OsFileFlush( m_hfile ); }
	OsError Seek( size_t distance ,int seekpos ) { return OsFileSeek( m_hfile ,distance ,seekpos ); }
    OsError Tell( size_t &distance ) { return OsFileTell( m_hfile ,&distance ); }

	void Close( void ) { OsHandleDestroy( &m_hfile ); }

protected:
	OsHandle m_hfile;
};

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_CORE_H