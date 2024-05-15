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

#ifndef TINY_MEMORY_H
#define TINY_MEMORY_H

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////
#define TINY_MEMMINSIZE        64                    //! minimum block size in bytes
#define TINY_MEMDEFALIGN    (sizeof(void*))        //! default block size alignment (in bytes)
#define TINY_MEMMAXADJUST    0x01000000            //! maximum block size adjustment (in bytes) //! 16Mb

#ifndef OSALLOC
#define OSALLOC(size,flag)			malloc(size)
#endif

#ifndef OSREALLOC
#define OSREALLOC(mem,size,flag)	realloc(mem,size)
#endif

#ifndef OSFREE
#define OSFREE(mem)				free(mem)
#endif

/**
 * @param count of item to align
 * @param align bytes to align size to
 * @return aligned size
 */

template<typename T>
size_t size_align_( size_t count ,size_t align = 0 ) {
    size_t a = (align == 0) ? MAX( TINY_MEMDEFALIGN ,sizeof(T)) : align;

    size_t c = count * sizeof( T );

    size_t r = c % a;

    return c + (r ? (align - r) : 0);
}

inline size_t size_align( size_t size ,size_t align = 0 ) {
    return size_align_<byte>( size ,align );
}

/**
 * @brief adjust size to the next power of 2, respect MEMMINSIZE and MEMMAXADJUST
 * @param count of item to adjust
 * @param align bytes to align size to
 * @return adjusted and aligned size
 */

template<typename T>
size_t size_adjust_( size_t count ,size_t align = 0 ) {
    size_t allocSize = MAX( size_align_<T>( count ,align ) ,TINY_MEMMINSIZE );

    //! limit allocation adjustment
    if( allocSize > TINY_MEMMAXADJUST )
        return size_align_<T>( count ,TINY_MEMMAXADJUST );

    //! find next power of 2
    size_t pow2size = 1;

    while( allocSize > pow2size )
        pow2size *= 2;

    return pow2size;
}

inline size_t size_adjust( size_t size ,size_t align = 0 ) {
    return size_adjust_<byte>( size ,align );
}

/**
 * Single allocation function for OSALLOC macros
 * @tparam T type of item to be in the requested buffer
 * @param p pointer to previously allocated memory (NullPtr if none)
 * @param count of item to alloc, 0 to free the memory
 * @param flags to be passed to OSALLOC macro (alignment, zeroing...)
 * @return allocated, re-allocated memory, or NullPtr if free requested
 */
template<typename T>
T *MemAlloc_( T *p ,size_t count ,int flags = 0 ) {
    if( !p ) {
        if( count == 0 ) return NullPtr;

        return (T *) OSALLOC( count * sizeof( T ) ,flags );
    }

    if( count == 0 ) {
        OSFREE( p );
        return NullPtr;
    }

    return (T *) OSREALLOC((void *) p ,count * sizeof( T ) ,0 );
}

inline byte *MemAlloc( byte *p ,size_t count ,int flags = 0 ) {
    return MemAlloc_<byte>( p ,count ,flags );
}

//////////////////////////////////////////////////////////////////////////
class MemoryException : Exception {
public:
    MemoryException( iresult_t id = IERROR ,const char *msg = NullPtr ) :
            Exception( id ,msg ) {}
};

//////////////////////////////////////////////////////////////////////////
//! Memory

    //! @memory is a basic memory object, use as a base or tool for complete class object
    //! @note no constructor / destructor call for item(s) in memory

template<typename T>
struct Memory_ {
    T *items;

//--
    Memory_() : items(NullPtr) {}

    Memory_( size_t count ) : items(NullPtr) {
        Alloc(count);
    }

    Memory_( const T *items ,size_t count ) : items(NullPtr) {
        CopyFrom( items ,count );
    }

    Memory_( const T *pa ,const T *pb ) : items(NullPtr) {
        CopyFrom( pa ,(pb > pa) ? (pb - pa) : 0 );
        //TODO reverse if pointer inverted ?
    }

    Memory_( const T *items ,size_t a ,size_t b ) : items(NullPtr) {
        CopyFrom( items + a ,(b > a) ? (b - a) : 0 );
    }

    //! Memory_( const Memory_<T> &copy )
        //! NB memory object copy is memberwise, IE doesn't copy content
        //TODO this is probably wrong (double dealloc)

    ~Memory_() {
        Free();
    }

    operator const T *() const { return items; }

    Memory_<T> operator()( int a ,int b ) {
        return Memory_( items ,(size_t) MAX( a ,0 ) ,(size_t) MAX( b ,0 ));
    }

    T &operator []( int i ) { return items[i]; }

    const T *ptr() const { return items; }
    T *ptr() { return items; }

    bool IsNull() const {
        return items == NullPtr;
    }

    //! NB cannot // bool CopyFrom( const Memory_<T> &copy ); //! (we don't know the size of the buffer)

//-- allocation
    bool AllocBytes( size_t size ) {
        T *p = MemAlloc_<byte>( items ,size );

        if( !p && size ) return false;

        items = p;

        return true;
    }

    bool Alloc( size_t count ) {
        T *p = MemAlloc_<T>( items ,count );

        if( !p && count ) return false;

        items = p;

        return true;
    }

    void Free( void ) {
        Alloc( 0 );
    }

    void Detach() {
        items = NullPtr;
    }

//-- blitting
    bool CopyFrom( const T *src ,size_t count ) {
        if( !Alloc( count )) return false;

        copy( items ,src ,count );

        return true;
    }

    bool CopyTo( T *dst ,size_t count ) const {
        if( !dst ) return count == 0;

        copy( dst ,items ,count );

        return true;
    }

//-- static operation
    static T *copy( T *dst ,const T *src ,size_t count ) {
        if( dst && src && count )
            memcpy( dst ,src ,count * sizeof( T ));

        return dst;
    }

    /*
    static bool move( const T *s ); // memmove
    static bool set( const T *s ); // ...
    static bool cmp( const T *s );
    static bool icmp( const T *s );
    static bool find( const T *s ); //! memchr
    */
};

//////////////////////////////////////////////////////////////////////////
//! Buffer

#define ARRAY_ITEMNEW(i)		{ new ( (void*) &(m_memory[(i)]) ) T; }
#define ARRAY_ITEMDELETE(i)		{ m_memory[(i)].~T(); }

template <typename T>
class Buffer_ {
public:
    Buffer_() : m_size(0) ,m_count(0) {}
    Buffer_( size_t count ) : m_memory(count) ,m_size(count*sizeof(T)) ,m_count(count) {}
    Buffer_( const T &c ,size_t count ) : m_memory(count) ,m_size(count*sizeof(T)) ,m_count(count) { Set(c); }
    Buffer_( const T *items ,int count ) : m_memory(items,count) ,m_size(count*sizeof(T)) ,m_count(count) {}
    Buffer_( const Buffer_ &copy ) { this->CopyFrom(copy); }

    ~Buffer_() { m_memory.Free(); }

//-- iterator
    typedef T* iterator_t;

    iterator_t begin() { return m_memory.ptr(); }
    iterator_t end() { return m_memory.ptr() + m_count; }
    iterator_t rbegin() { return m_memory.ptr() + m_count -1; }
    iterator_t rend() { return m_memory.ptr() -1; }

    const iterator_t begin() const { return m_memory.ptr(); }
    const iterator_t end() const { return m_memory.ptr() + m_count; }
    const iterator_t rbegin() const { return m_memory.ptr() + m_count -1; }
    const iterator_t rend() const { return m_memory.ptr() -1; }

//-- accessors
    size_t size() const { return m_size; }
    size_t count() const { return m_count; }
    bool empty() const { return m_count == 0; }

    const T *ptr( int i=0 ) const { return m_memory.ptr() + i; }
    T *ptr( int i=0 ) { return m_memory.ptr() + i; }

    void Set( const T &item ) { for( int i=0; i < m_count; ++i ) m_memory[i] = item; }
    bool Set( const T &item ,size_t n ) { if( !Reserve(n) ) return false; m_count = n; for( int i=0; i < m_count; ++i ) m_memory[i] = item; return true; }

    bool copyFrom( const T *items ,size_t n ) {
        if( !Reserve(n) ) return false; for( int i=0; i<n; ++i ) m_memory[i] = items[i]; m_count = n; return true;
    }

    bool CopyFrom( const Buffer_<T> &copy ) {
        int n = copy.GetCount(); if(!Reserve( n )) return false; for( int i=0; i<n; ++i ) m_memory[i] = copy.m_memory[i]; m_count = n; return true;
    }

//-- operators
    operator const T*() const { return m_memory.Ptr(); }
    operator T*() { return m_memory.Ptr(); }

    T &operator[]( int i ) const { assert( i < m_count ); return m_memory[i]; }

    Buffer_ &operator =( const Buffer_ &copy ) { CopyFrom( copy ); return *this; }
    Buffer_ &operator +=( const T &item ) { Append( item ); return *this; }

//-- content
    // T *Insert( const T &item );
    T *Append( void ) { if( !Reserve( m_count + 1) ) return NULL; ARRAY_ITEMNEW(m_count); return &(m_memory[m_count++]); }
    T *Append( const T &item ) { if( !Reserve( m_count + 1) ) return NULL; ARRAY_ITEMNEW(m_count); m_memory[m_count] = item; return &(m_memory[m_count++]); }

    bool Move( int i ,int to ) {
        if( m_count == 0 || i >= m_count ) return false; if( i == to ) return true;

        byte r[sizeof(T)];

        memcpy( &r ,&m_memory[i] ,sizeof(T) );

        if( i<to )
            memmove( &m_memory[i] ,&m_memory[i + 1] ,(to - i) * sizeof(T) );
        else
            memmove( &m_memory[to + 1] ,&m_memory[to] ,(i - to) * sizeof(T) );

        memcpy( &m_memory[to] ,&r ,sizeof(T) );

        return true;
    }

    //  void Swap( int i ,int to )
        //TODO NO ... support for object in Collection_

    void Remove( int i ) { if( i >= m_count || m_count == 0 ) return; ARRAY_ITEMDELETE(i); if( i < m_count - 1 ) memcpy( &m_memory[i] ,&m_memory[m_count - 1] ,sizeof(T) ); --m_count; }
    bool Remove( const T &item ) { int i,j=0; while( (i=Find( item )) >= 0 ) Remove(i) ,++j; return j > 0; }
    void Clear( void ) { while( m_count > 0 ) { --m_count; ARRAY_ITEMDELETE(m_count); } }

    void Trash( void ) { m_count = 0; } //! CAREFUL ! no destructor will be called

//-- allocation

    //! make sure allocated buffer can hold 'size' element
    bool Reserve( size_t count ) {
        if( count*sizeof(T) <= m_size ) return true;

        size_t size = size_adjust_<T>( count ,TINY_MEMDEFALIGN );

        if( !m_memory.AllocBytes( size ) ) return false;

        memset( (void*) (m_memory.items + m_size) ,0 ,size - m_size * sizeof(T) );

        m_size = size; return true;
    }

    //! @brief reserve count more element
    bool ReserveMore( size_t count = 1 ) {
        return Reserve( m_count + count );
    }

    //! @brief resize to count of element
    bool Resize( size_t count ) {
        if( !Reserve(count) ) return false;

        m_count = count; return true;
    }

    void Compact( void ) {
        Resize( m_count );
    }

    void Free( void ) {
        m_memory.Free(); m_size = m_count = 0;
    }

protected: //-- members
    Memory_<T> m_memory; //! actual buffer for the items
    size_t m_size; //! allocated size of array (in byte)

    size_t m_count; //! count of element in array
};

//--
typedef Buffer_<byte> Bytes;
typedef Buffer_<byte> Buffer;

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_MEMORY_H