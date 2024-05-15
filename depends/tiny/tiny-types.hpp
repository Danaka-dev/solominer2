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
//! tiny implementation of fundamental type extension
//!  > for apps to share complex types (EG [out] string& or [out] map& in an API)

//! NB: this header is fully detachable from the tiny-for-c and can be used on its own

//////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <ctype.h>

#ifdef _MSC_VER
 #pragma warning( disable : 4996 ) //! temp
#endif

//////////////////////////////////////////////////////////////////////////
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

// inline void * __cdelc operator new( size_t size ,void *placement) { return(placement); }

inline void * operator new( size_t size ,void *placement) {
    return(placement);
}

inline void operator delete( void *data ,void *placement ) {
    assert(false);
}

#endif

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////
template <typename T> struct args_ {
	args_( int c=0 ,T *v=NULL ) : _count(c) ,_values(v) {}

	args_ operator()( int a ,int b ) { int ua = MAX(a,0); int c = MAX( MIN(b,_count-1) - ua +1 ,0 ); return args( c ,_values+ua ); }

	args_ &operator =( const args_ &c ) { _count = c._count; _values = c._values; return *this; }

	T operator []( int i ) { return (i < _count ? _values[i] : (T) NULL); }

	bool operator >>( T &v ) { if( _count > 0 ) v = *_values, ++_values ,--_count; else return false; return true; }

	int getCount() { return _count; }

	void Pop( int n=1 ) { _values += n; _count = MAX(0,_count-n); }

	int _count;

	T *_values;
};

typedef struct args_<char_t*> args;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <typename T> struct memory_;

typedef memory_<byte> memory;

//--
template <typename T> struct string_;  //! case sensitive string

typedef string_<char>		stringa;
typedef string_<wchar_t>	stringw;
typedef string_<char_t>		string;

template <typename T> struct STRING_; //! case insensitive string

typedef STRING_<char>		STRINGA;
typedef STRING_<wchar_t>	STRINGW;
typedef STRING_<char_t>		STRING;

// date/time

//--
struct referable;
template <class T> struct reference_; //! 'smart ptr'

struct uuid; //! unique universal identifier
template <typename T> struct component_; //! 'object'

typedef component_<uuid> component;
typedef reference_<component> reference;

//+ data //! ref counted object

//--
struct variant;

template <typename T> struct point_;
template <typename T> struct rect_;

template <typename Ta ,typename Tb> struct pair_;
template <int Tn ,typename T> struct tuple_;

template <typename T> struct enumerator_;

//+ vector //! static sized single dimension serie
//+ kernel //! static sized n dimensional 

//+ buffer //! dynamic sized serie of element

template <typename T> struct array;

template <typename T> struct list;
template <typename T> struct tape;
template <typename T> struct stack;
template <typename T> struct queue;

template <typename Tkey ,typename Tvalue> struct map;

//? NB parallelisable data columns based info struct 

template <typename T> struct table;
template <typename T> struct registry;
template <typename T> struct store;

template <class T ,typename Ti> struct factory;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Memory Management

#define TINY_MEMMINSIZE		64					//! minimum block size in bytes
#define TINY_MEMDEFALIGN	(sizeof(void*))		//! default block size alignment (in bytes)
#define TINY_MEMMAXADJUST	0x01000000			//! maximum block size adjustment (in bytes) //! 16Mb

#ifndef OSALLOC
 #define OSALLOC(size,flag)			malloc(size)
#endif

#ifndef OSREALLOC
 #define OSREALLOC(mem,size,flag)	realloc(mem,size)
#endif

#ifndef OSFREE
 #define OSFREE(mem)				free(mem)
#endif

template <typename T> inline T *memalloc( T *p ,size_t size ,int flags=0 )
{
	if( p == NULL ) { if( size == 0 ) return NULL; else return (T*) OSALLOC( size ,flags ); }

	if( size == 0 ) { OSFREE( p ); return NULL; }

	return (T*) OSREALLOC( (void*) p ,size ,0 );
}

// NB -> buffer like array but without constructor / destructor (no support for objects)
template <typename T> struct memory_ // ! foundation for string, buffer and array
{
	memory_() : _items(NULL) {}
	memory_( const T *items ,size_t count ) : _items(NULL) { CopyFrom( items ,count ); }
	memory_( const T *items ,size_t a ,size_t b ) : _items(NULL) { CopyFrom( items+a ,(b>a) ? (b-a) : 0 ); }
	//! memory_( const memory_<T> &copy ) //! NB memory object copy is memberwise, IE doesn't copy content
	~memory_() { Free(); }

	T *_items;

	operator const T *() const { return _items; }

	memory_<T> operator()( int a ,int b ){ return memory_( _items ,(size_t) MAX(a,0) ,(size_t) MAX(b,0) ); }

	const T *Ptr() const { return _items; }
	T *Ptr() { return _items; }

	bool IsNull() const { return _items == NULL; }

	//! NB cannot // bool CopyFrom( const memory_<T> &copy ); //! (we don't know the size of the buffer)

	bool CopyFrom( const T *items ,size_t count )
	{
		if( !Alloc( count ) ) return false;

		copy( _items ,items ,count );

		return true;
	}

	bool CopyTo( T *items ,size_t count ) const
	{
		if( items == NULL ) return false; //! NB target must be large enough for count items

		copy( items ,_items ,count );

		return true;
	}

		//! TODO handle count == 0 for these functions
		//! TODO use macro alloc function
	bool Alloc( size_t count ) { T *p = (T*) malloc( sizeof(T)*(MAX(count,0)) ); if(p) Free() ,_items = p; return p != NULL; }
	bool Adjust( size_t count ) { if( _items == NULL ) return Alloc(count); T *p = (T*) realloc(_items,sizeof(T)*(MAX(count,0))); if(p) _items=p; return p != NULL; }
	void Free( void ) { if( _items ) free(_items) ,_items = NULL; }

	static size_t size_align( size_t count ,size_t align=0 ) //! 'size' and 'align' in bytes
	{
		size_t a = (align==0) ? sizeof(T) : align;

		size_t c = count * sizeof(T);

		size_t r = c % a;

		return c + (r ? (align - r) : 0);
	}

	static size_t size_adjust( size_t count ,size_t align=0 ) //! adjust size to the next power of 2, respect MEMMINSIZE and MEMMAXADJUST
	{ 
		size_t allocSize = MAX( size_align( count ,align ) ,TINY_MEMMINSIZE );

		//! limit allocation adjustment
		if( allocSize > TINY_MEMMAXADJUST )
			return size_align( count ,TINY_MEMMAXADJUST );

		//! find next power of 2
		size_t pow2size = 1;

		while( allocSize > pow2size ) pow2size *= 2;

		return pow2size;
	}

//-- static tool function
	static T *copy( T *dst ,const T *src ,size_t count ) { if( dst && src && count ) memcpy( dst ,src ,count*sizeof(T) ); return dst; }

	/*
	static bool move( const T *s ); // memmove
	static bool set( const T *s ); // ...
	static bool cmp( const T *s );
	static bool icmp( const T *s );
	static bool find( const T *s ); //! memchr
	*/
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef bool (*cacceptA)( const char c );
typedef bool (*cacceptW)( const wchar_t c);
typedef bool (*caccept)( const char_t c);

//////////////////////////////////////////////////////////////////////////
inline bool cisdigit( const char c ) { return( c>='0' && c<='9' ); }
inline bool cisdigit( const wchar_t c ) { return( (c)>=L'0' && (c)<=L'9' ); }
inline bool cishexdigit( const char c ) { return( cisdigit(c) || ((c)>='a' && (c)<='f' ) || ((c)>='A' && (c)<='F' ) ); }
inline bool cishexdigit( const wchar_t c ) { return( cisdigit(c) || ((c)>=L'a' && (c)<=L'f' ) || ((c)>=L'A' && (c)<=L'F' ) ); }
inline bool cislcase( const char c )  { return( (c)>='a' && (c)<='z' ); }
inline bool cislcase( const wchar_t c )  { return( (c)>=L'a' && (c)<=L'z' ); }
inline bool cisucase( const char c )  { return( (c)>='A' && (c)<='Z' ); }
inline bool cisucase( const wchar_t c )  { return( (c)>=L'A' && (c)<=L'Z' ); }
inline bool cisalpha( const char c ) { return( cislcase(c) || cisucase(c) ); }
inline bool cisalpha( const wchar_t c ) { return( cislcase(c) || cisucase(c) ); }
inline bool cisalphanum( const char c ) { return( cisalpha(c) || cisdigit(c) ); }
inline bool cisalphanum( const wchar_t c ) { return( cisalpha(c) || cisdigit(c) ); }
inline bool cissign( const char c ) { return( c=='+' || c=='-'); }
inline bool cissign( const wchar_t c ) { return( c==L'+' || c==L'-'); }
inline bool cisnumsep( const char c ) { return( c==',' || c=='.'); }
inline bool cisnumsep( const wchar_t c ) { return( c==L',' || c==L'.'); }
inline bool cisnum( const char c ) { return( cisdigit(c) || cisnumsep(c) ); }
inline bool cisnum( const wchar_t c ) { return( cisdigit(c) || cisnumsep(c) ); }

inline bool cisspace( const char c ) { return( (c)==' ' ); }
inline bool cisspace( const wchar_t c ) { return( (c)==L' ' ); }
inline bool cistab( const char c ) { return( (c)=='\t' ); }
inline bool cistab( const wchar_t c ) { return( (c)==L'\t' ); }
inline bool cisnewline( const char c ) { return( (c)=='\n' ); }
inline bool cisnewline( const wchar_t c ) { return( (c)==L'\n' ); }
inline bool cisreturn( const char c ) { return( (c)=='\r' ); }
inline bool cisreturn( const wchar_t c ) { return( (c)==L'\r' ); }

inline bool cissep( const char c ) { return( cisspace(c) || cistab(c) ); }
inline bool cissep( const wchar_t c ) { return( cisspace(c) || cistab(c) ); }
inline bool ciseol( const char c ) { return( cisreturn(c) || cisnewline(c) ); }
inline bool ciseol( const wchar_t c ) { return( cisreturn(c) || cisnewline(c) ); }
inline bool ciswspace( const char c ) { return( cissep(c) || ciseol(c) ); } //! whitesapce
inline bool ciswspace( const wchar_t c ) { return( cissep(c) || ciseol(c) ); }

//////////////////////////////////////////////////////////////////////////
inline int cnumvalue( const char c ) { return (c<0x30) ? -1 : (c<0x3A) ? (c-0x30) : -1; }
inline int chexvalue( const char c ) { return (c<0x30) ? -1 : (c<0x3A) ? (c-0x30) : (c<0x41) ? -1 : (c<0x47) ? (c-0x41+10) : (c<0x61) ? -1 : (c<0x67) ? (c-0x61+10) : -1; }
//! return value of hexadecimal digit (0..9,A,B,C,D,E,F -> 0..15)
// inline int cnumvalue( const char c ,int base ) { return base <= 16 &&  (base==10) ? cnumvalue(c) : (base==16) ? chexvalue(c<0x30) : -1; }

inline char ctoupper( const char c ) { return toupper(c); }
inline wchar_t ctoupper( const wchar_t c ) { return towupper(c); }
inline char ctolower( const char c ) { return tolower(c); }
inline wchar_t ctolower( const wchar_t c ) { return towlower(c); }

//////////////////////////////////////////////////////////////////////////
inline bool cistypesep( const char_t c ) { return cisspace(c) || c == ':'; }
inline bool cislistsep( const char_t c ) { return cisspace(c) || c == ','; }
inline bool cisitemsep( const char_t c ) { return cisspace(c) || c == ';'; }
inline bool cisname( const char_t c ) { return cisalpha(c) || c == '_'; }

//////////////////////////////////////////////////////////////////////////
template <typename T> struct string_
{
//-- instance
	string_() : _string(NULL) { Reserve(0); }
	string_( const T *s ) { _string = dup( s ); }
	string_( const T *s ,int len ) : _string(NULL) { CopyFrom( s ,len ); }
	string_( const string_ &copy ) { _string = dup( (const T*) copy ); }

	~string_() { SAFEFREE(_string); }

	template <typename Tn> string_( const Tn &v ) : _string(NULL) { operator =( v ); }

	T *_string;

//-- operators
	operator const T *() const { return _string; }
	operator int() const { return To<int>(); }

	string_ operator()( int a ,int b ){ return SubString(a,b); }

	T &operator []( int i ) { return At(i); }

	//! NB: NOT using template <typename Tn> string_ &operator =( Tn v );
	//! operators provide implicit type cast to native types
	//!	function use explicit type cast and access partial specialization conversion
	//! exception is operator <<() which use full explicit type range

	string_ &operator =( wchar_t v );

	string_ &operator =( char v );
	string_ &operator =( short v );
	string_ &operator =( int v );
	string_ &operator =( long v );
	string_ &operator =( long long v );

	string_ &operator =( unsigned char v );
	string_ &operator =( unsigned short v );
	string_ &operator =( unsigned int v );
	string_ &operator =( unsigned long v );
	string_ &operator =( unsigned long long v );

	string_ &operator =( float v );
	string_ &operator =( double v );

	string_ &operator =( const char *s );
	string_ &operator =( const wchar_t *s );

	string_ &operator =( const string_ &s ) { return operator =( s.Ptr() ); }

	//! compare
	bool operator ==( const T *s ) const { return Compare( s ); }
	bool operator !=( const T *s ) const { return !operator ==( s ); }

	//! operation
	string_ &operator +=( const T c ) { Append( c ); return *this; }
	string_ &operator +=( const T *s ) { Append( s ); return *this; }
	string_ &operator +=( const string_ &s ) { Append( (const T*) s ); return *this; }

	template <typename Tn> string_ &operator +=( const Tn &v ) { string_ s(v); Append( (const T*) s ); return *this; }

	string_ operator +( const T c ) const { string_ r( *this ); return r += c; }
	string_ operator +( const T *s ) const { string_ r( *this ); return r += s; }
	string_ operator +( const string_ &s ) const { string_ r( *this ); return r += (const T*) s; }

	template <typename Tn> string_ operator +( const Tn &v ) const { string_ r( *this ); return r += v; }

	string_ &operator *=( int n ) { Repeat( n ); return *this; }
	string_ operator *( int n ) const { string_ r( *this ); return r *= n; }

	//! NB: this operator is using full From<>() cast range, unlike other operator that are limited to native type cast
	template <typename Tn> string_ &operator <<( const Tn &v ) { Append( v ); return *this; }

	//? template <typename Tn> string_ &operator >>( Tn &v ) { }

//-- getter
	inline const T *Ptr() const { return _string; }
	inline T *Ptr() { return _string; }

	inline T *Duplicate() const { return dup( _string ); }

	inline int Length() const { return len(_string); }
	inline int Size() const { return (Length()+1) * sizeof(T); }
	inline bool IsEmpty() const { return _string==NULL || _string[0]==0; }

	inline string_ SubString( int a ,int b ) const { if( _string == NULL ) return string_(); else return string_( _string+a ,MAX(0,b-a) ); }

	inline T &At( int i ) { assert( _string != NULL && i < Length() ); return _string[i]; }

//-- operator function (NB available for inheritance)
	template <typename Tn> string_ &Set( const Tn &v ) { return operator =(v); }
	template <typename Tn> string_ &Add( const Tn &v ) { return operator +=(v); }
	template <typename Tn> string_ &Mul( const Tn &v ) { return operator *=(v); }

//-- parse/generate (NB open for partial specialization ... see also FromString<T> and ToString<T>)
	template <typename Tn> bool From( const Tn &v );
	template <typename Tn> bool To( Tn &v ) const;

	template <typename Tn> Tn To( void ) const { Tn v; To( v ); return v; }

//-- compare
	bool Match( const T *s ) const { return match( _string ,s ) == 0; }

	bool Compare( const T *s ) const { return cmp( _string ,s ) == 0; }
	bool Compare( const T *s ,int length ) const { return ncmp( _string ,s ,length ) == 0; }

	int Find( const T *s )  { return find( _string ,s ); }
	int Find( const T *s ,int from=0 ) { int i; if( _string==NULL || (i=find(_string+from,s))<0 ) return -1; ; return from+i; }

//-- manipulate
	bool Repeat( int n ) { int l=len(_string); if( !Reserve(l*n) ) return *this; int i=0; if( l*n ) while( i++ < n ) if(i) cpy( _string+(i-1)*l ,_string ); _string[i*l]=0; return *this; }

	bool Append( T c ) { int l=len(_string); if( !Adjust(l+1) ) return false; _string[l]=c; _string[l+1]=0; return true; }
	bool Append( const T *s ) { int l=len(s); if( l<=0 ) return true; if( !Augment( l ) ) return false; cat( _string ,s ); return true; }

	template <typename Tn> bool Append( const Tn &v ) { string_ s; s.From<Tn>( v ); return Append( (const T *) s ); }

	bool Insert( int at ,T c );
	bool Insert( int at ,const T *s );

	template <typename Tn> bool Insert( int at ,const Tn &v ) { string_ s; s.From<Tn>( v ); return Insert( at ,(const T *) s ); }

	void Erase( int at ,int length );

	void Fill( T c ) { set( _string ,c ); }
	void Reverse( void ) { rev( _string ); }

	void TrimLeft( void );
	void TrimRight( void );
	void Trim( void ) { TrimLeft(); TrimRight(); }

	void UpperCase( void ) { toupper( _string ); }
	void LowerCase( void ) { tolower( _string ); }

	void Capitalize( void );

//-- generate/parse
	int Format( const T *format ,int maxLength ,... )
	{
		va_list varg;

		va_start( varg ,maxLength );

		if( !Reserve( maxLength ) ) return -1;

		int len = vsnprintf( _string ,maxLength ,format ,varg );

		if( len < 0 ) _string[0] = 0;

		va_end( varg );

		return len;
	}

	// int Scan( const T *format ,int maxLength ,... ) //! TODO inverse of Format

	typedef bool (*caccept)( const T c);

	int Token( const T *s ,int at ,caccept faccept ,bool islimit=false ) //! rename to GetToken
	{
		int j=at;

		if( s ) while( s[j] && faccept( s[j] ) != islimit ) ++j;

		if( j==at ) { Clear(); return j; }

		if( CopyFrom( s+at ,j-at ) == false ) return -1;

		return j;
	}

	bool GetField( const T *s ,int &at ,caccept flimit ,bool allowDefault=false ,bool requireSep=true )
	{
		int i = Token( s ,at ,flimit ,true ); if( i < 0 ) return false; //! get field value

		if( i == at && !allowDefault ) return false;

		int j = Token( s ,i ,flimit ,false ); //! skip delimiter

		if( j == i && requireSep ) return false;

		if( j == at ) return false; //! such condition always false: may happen if allowDefault and not requireSep

		at = j; return true;
	}

	template <typename Tn> bool GetField( Tn &value ,const T *s ,int &at ,caccept flimit ,bool allowDefault=false ,bool requireSep=true )
	{
		if( !GetField( s ,at ,flimit ,allowDefault ,requireSep ) ) return false;

		value = To<Tn>();

		return true;
	}

	bool CopyFrom( const T *s ) { SAFEFREE( _string ); _string = dup(s); return _string != NULL; }
	bool CopyFrom( const T *s ,int length );

	int CopyTo( T *s ,int maxLength ) const;

//-- memory
	bool Alloc( int length ) { SAFEFREE(_string); _string = (T*) malloc( sizeof(T)*(MAX(length,0)+1) ); if(_string) _string[0] = 0; return _string != NULL; }
	bool Adjust( int length ) { if(_string==NULL) return Alloc(length); T *p = (T*) realloc(_string,sizeof(T)*(MAX(length,0)+1)); if(p) _string=p; return p != NULL; }
	bool Augment( int nmore ) { int l=len(_string); return Adjust(l+nmore); }
	bool Reserve( int length ) { int l=Length(); if( l==0 || l < length ) return Adjust( length ); else return true; }
	void Free( void ) { SAFEFREE( _string ); }

	void Clear( void ) { if( _string != NULL ) _string[0] = 0; }

//-- static
	static int len( const char *s ) { return s ? (int) strlen(s) : 0; }
	static int len( const wchar_t *s ) { return s ? (int) wcslen(s) : 0; }

	static char *dup( const char *s ) { return s ? strdup(s) : strdup(""); }
	static wchar_t *dup( const wchar_t *s ) { return s ? wcsdup(s) : wcsdup(L""); }

	// static char *set( const char *s ,char c ) { return s ? strset(s,c) : NULL; }
	// static wchar_t *set( const wchar_t *s ,wchar_t c ) { return s ? wcsset(s,c) : NULL; }

	static char *cpy( char *dest ,const char *source ) { if( dest && source ) strcpy( dest ,source ); return dest; }
	static wchar_t *cpy( wchar_t *dest ,const wchar_t *source ) { if( dest && source ) wcscpy( dest ,source ); return dest; }

	static char *ncpy( char *dest ,const char *source ,size_t n ) { if( dest && source && n ) strncpy( dest ,source ,n ); return dest; }
	static wchar_t *ncpy( wchar_t *dest ,const wchar_t *source ,size_t n ) { if( dest && source &&  n ) wcsncpy( dest ,source ,n ); return dest; }

	static char *cat( char *dest ,const char *source ) { if( dest && source ) strcat( dest ,source ); return dest; }
	static wchar_t *cat( wchar_t *dest ,const wchar_t *source ) { if( dest && source ) wcscat( dest ,source ); return dest; }

	// static char *rev( char *s ) { return s ? strrev(s) : NULL; }
	// static wchar_t *rev( wchar_t *s ) { return s ? wcsrev(s) : NULL; }

	static int cmp( const char *a ,const char *b ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : strcmp( a ,p ); }
	static int cmp( const wchar_t *a ,const wchar_t *b ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcscmp( a ,p ); }

	static int ncmp( const char *a ,const char *b ,int n ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : strncmp( a ,p ,n ); }
	static int ncmp( const wchar_t *a ,const wchar_t *b ,int n ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcsncmp( a ,p ,n ); }

	static int match( const char *a ,const char *b ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : strncmp( a ,p ,len(p) ); }
	static int match( const wchar_t *a ,const wchar_t *b ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcsncmp( a ,p ,len(p) ); }

	static int find( const char *a ,const char *b ) { if( a==NULL || b==NULL ) return -1; const char *p = strstr( a ,b ); return p ? (int) (p-a) : -1; }
	static int find( const wchar_t *a ,const wchar_t *b ) { if( a==NULL || b==NULL ) return -1; const wchar_t *p = wcsstr( a ,b ); return p ? (int) (p-a) : -1; }

	static int find( const char *a ,char c ) { if( a==NULL ) return -1; const char *p = strchr( a ,c ); return p ? (int) (p-a) : -1; }
	static int find( const wchar_t *a ,wchar_t c ) { if( a==NULL ) return -1; const wchar_t *p = wcschr( a ,c ); return p ? (int) (p-a) : -1; }

	static int rfind( const char *a ,const char *b ); //! TODO
	static int rfind( const wchar_t *a ,const wchar_t *b ); //! TODO

	static int rfind( const char *a ,char c ) { if( a==NULL ) return -1; const char *p = strrchr( a ,c ); return p ? (int) (p-a) : -1; }
	static int rfind( const wchar_t *a ,wchar_t c ) { if( a==NULL ) return -1; const wchar_t *p = wcsrchr( a ,c ); return p ? (int) (p-a) : -1; }

	/*
	static char *toupper( char *s ) { return s ? strupr(s) : 0; }
	static wchar_t *toupper( wchar_t *s ) { return s ? _wcsupr(s) : 0; }

	static char *tolower( char *s ) { return s ? strlwr(s) : 0; }
	static wchar_t *tolower( wchar_t *s ) { return s ? _wcslwr(s) : 0; }
	*/
	
	static int toi( const char *s ) { return s ? atoi(s) : 0; }
	// static int toi( const wchar_t *s ) { return s ? _wtoi(s) : 0; }

	static long tol( const char *s ) { return s ? atol(s) : 0; }
	// static long tol( const wchar_t *s ) { return s ? wcstol(s) : 0; }

	static double tof( const char *s ) { return s ? atof(s) : 0.; }
	// static double tof( const wchar_t *s ) { return s ? wcstod(s) : 0.; }
};

//////////////////////////////////////////////////////////////////////////
//-- manipulate

template <typename T> bool string_<T>::Insert( int at ,T c )
{ 
	int l=len(_string); at = MIN(at,l);

	if( !Adjust(l+1) ) return false; 

	if( l > 0 ) memmove( _string+at+1 ,_string+at ,(l-at+1)*sizeof(T) ); 

	_string[at] = c;

	return true;
}

template <typename T> bool string_<T>::Insert( int at ,const T *s )
{ 
	int l1=len(_string) ,l2=len(s); at = MIN(at,l1);

	if( l2 <= 0 ) return true;

	if( !Adjust(l1+l2) ) return false;

	if( l1 > 0 ) memmove( _string+at+l2 ,_string+at ,(l1-at+1)*sizeof(T) );

	memcpy( _string+at ,s ,l2 );

	return true;
}

template <typename T> void string_<T>::Erase( int at ,int length )
{ 
	int l=len(_string); 

	if( at<l )
	{ 
		if( at+length<l) 
			memmove(_string+at,_string+at+length ,l-at-length ) ,_string[l-length]=0; 
		else 
			_string[at]=0;

	}
}

template <typename T> void string_<T>::TrimLeft( void )
{ 
	T *p=_string; int l=len(_string);
	
	if( l<=0 ) return;
	
	int i=0; while( p[i] && cisspace(p[i]) ) ++i;
	
	if( i==0 ) return;

	if( i<l ) 
		memmove( _string ,_string+i ,len(_string)-i ); 

	else 
		_string[0] = 0;
}

template <typename T> void string_<T>::TrimRight( void )
{
	T *p=_string; int l=len(_string);

	if( l<=0 ) return;

	int i=l-1; while( i>=0 && cisspace(p[i]) ) --i;

	_string[i+1] = 0;
}

template <typename T> void string_<T>::Capitalize( void )
{ 
	T *p=_string; int l=len(_string);

	if( l<=0 ) return;

	int i=0; bool sp=true; while( p[i] )
	{
		if( cisspace(p[i]) ) 
			sp = true;

		else if( sp ) 
			p[i] = ctoupper( p[i] ) ,sp=false;

		++i;
	}
}

template <typename T> bool string_<T>::CopyFrom( const T *s ,int length )
{ 
	if( length < 0 ) return CopyFrom( s );

	if( !Reserve(length) ) return false;

	if( length>0 ) memcpy( _string ,s ,length*sizeof(T) ); 

	_string[length] = 0;

	return true;
}

template <typename T> int string_<T>::CopyTo( T *s ,int maxLength ) const
{
	if( s == NULL ) return 0;

	int l = MIN( len(_string) ,maxLength );

	if( l > 0 ) ncpy( s ,_string ,l ); 

	return l;
}

//////////////////////////////////////////////////////////////////////////
//! assign

//-- <char>
template <> template <> inline bool string_<char>::From<wchar_t>( const wchar_t &v ) { return Format( "%C" ,3 ,(wchar_t) v ) > 0; }

template <> template <> inline bool string_<char>::From<char>( const char &v ) { if( !Reserve(1) ) return false; _string[0]=v; _string[1]=0; return true; }
template <> template <> inline bool string_<char>::From<short>( const short &v ) { return Format( "%d" ,7 ,(int) v ) > 0; }
template <> template <> inline bool string_<char>::From<int>( const int &v ) { return Format( "%d" ,15 ,(int) v ) > 0; }
template <> template <> inline bool string_<char>::From<long>( const long &v ) { return Format( "%ld" ,15 ,(long) v ) > 0; }
template <> template <> inline bool string_<char>::From<long long>( const long long &v ) { return Format( "%" I64_FMT "d" ,31 ,(int64_t) v ) > 0; }

template <> template <> inline bool string_<char>::From<unsigned char>( const unsigned char &v ) { if( !Reserve(1) ) return false; _string[0]=(char)v; _string[1]=0; return true; }
template <> template <> inline bool string_<char>::From<unsigned short>( const unsigned short &v ) { return Format( "%u" ,7 ,(int) v ) > 0; }
template <> template <> inline bool string_<char>::From<unsigned int>( const unsigned int &v ) { return Format( "%u" ,15 ,(int) v ) > 0; }
template <> template <> inline bool string_<char>::From<unsigned long>( const unsigned long &v ) { return Format( "%lu" ,15 ,(long) v ) > 0; }
template <> template <> inline bool string_<char>::From<unsigned long long>( const unsigned long long &v ) { return Format( "%" I64_FMT "u" ,31 ,(int64_t) v ) > 0; }

template <> template <> inline bool string_<char>::From<float>( const float &v ) { return Format( "%f" ,31 ,(double) v ) > 0; }
template <> template <> inline bool string_<char>::From<double>( const double &v ) { return Format( "%f" ,31 ,(double) v ) > 0; }

template <> template <> inline bool string_<char>::From<const char*>( char const * const &v ) { SAFEFREE( _string ); _string = dup(v); return _string != NULL; }
// template <> template <> inline bool string_<char>::From<const wchar_t*>( wchar_t const * const &v ) { SAFEFREE( _string ); ... convert }

/* NB convert
template <> template <> inline string_<wchar_t> string_<char>::To( void ) const
{ 
string_<wchar_t> ws; int l = len(_string); if( ws.Reserve(l) ) swprintf( ws._string ,l ,L"%hs" ,(const char*) _string ); return ws;
}

template <> template <> inline string_<char> string_<wchar_t>::To( void ) const
{ 
string_<char> s; int l = len(_string); if( s.Reserve(l) ) sprintf_s( s._string ,l ,"%hs" ,(const wchar_t*) _string ); return s;
}
*/

//-- operators (NB defined here, after From<> specialization have been declared
template <> inline string_<char> &string_<char>::operator =( wchar_t v ) { From<wchar_t>( v ); return *this; }

template <> inline string_<char> &string_<char>::operator =( char v ) { From<char>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( short v ) { From<short>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( int v ) { From<int>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( long v ) { From<long>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( long long v ) { From<long long>( v ); return *this; }

template <> inline string_<char> &string_<char>::operator =( unsigned char v ) { From<char>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( unsigned short v ) { From<unsigned short>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( unsigned int v ) { From<unsigned int>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( unsigned long v ) { From<unsigned long>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( unsigned long long v ) { From<unsigned long long>( v ); return *this; }

template <> inline string_<char> &string_<char>::operator =( float v ) { From<float>( v ); return *this; }
template <> inline string_<char> &string_<char>::operator =( double v ) { From<double>( v ); return *this; }

template <> inline string_<char> &string_<char>::operator =( const char *s ) { From<const char*>( s ); return *this; }
template <> inline string_<char> &string_<char>::operator =( const wchar_t *s ) { From<const wchar_t*>( s ); return *this; }

//-- <wchar_t>
//TODO

//////////////////////////////////////////////////////////////////////////
//! cast

//-- <char>
template <> template <> inline bool string_<char>::To<char>( char &v ) const { v = (char) (IsEmpty() ? 0 : _string[0]); return true; }
template <> template <> inline bool string_<char>::To<wchar_t>( wchar_t &v ) const { v = (wchar_t) (IsEmpty() ? 0 : _string[0]); return true; }
template <> template <> inline bool string_<char>::To<short>( short &v ) const { v = (short) toi( _string ); return true; }
template <> template <> inline bool string_<char>::To<int>( int &v ) const { v = (int) toi( _string ); return true; }
template <> template <> inline bool string_<char>::To<long>( long &v ) const { v = (long) tol( _string ); return true; }
template <> template <> inline bool string_<char>::To<long long>( long long &v ) const { v = (long long) tol( _string ); return true; }

template <> template <> inline bool string_<char>::To<unsigned char>( unsigned char &v ) const { v = (unsigned char) (IsEmpty() ? 0 : _string[0]); return true; }
template <> template <> inline bool string_<char>::To<unsigned short>( unsigned short &v ) const { v = (unsigned short) toi( _string ); return true; }
template <> template <> inline bool string_<char>::To<unsigned int>( unsigned int &v ) const { v = (unsigned int) toi( _string ); return true; }
template <> template <> inline bool string_<char>::To<unsigned long>( unsigned long &v ) const { v = (unsigned long) tol( _string ); return true; }
template <> template <> inline bool string_<char>::To<unsigned long long>( unsigned long long &v ) const { v = (unsigned long long) tol( _string ); return true; }

template <> template <> inline bool string_<char>::To<float>( float &v ) const { v = (float) tof( _string ); return true; }
template <> template <> inline bool string_<char>::To<double>( double &v ) const { v = (double) tof( _string ); return true; }

template <> template <> inline bool string_<char>::To<string_<char> >( string_<char> &s ) const { s = _string; return true; }
template <> template <> inline bool string_<char>::To<string_<wchar_t> >( string_<wchar_t> &s ) const { s = _string; return true; }

//-- <wchar_t>
//TODO

//////////////////////////////////////////////////////////////////////////
//! global operators

template <typename A ,typename T> A operator +( const A a ,const string_<T> &b ) { return a + b.template To<A>(); }
template <typename A ,typename T> A operator -( const A a ,const string_<T> &b ) { return a - b.template To<A>(); }
template <typename A ,typename T> A operator *( const A a ,const string_<T> &b ) { return a * b.template To<A>(); }
template <typename A ,typename T> A operator /( const A a ,const string_<T> &b ) { return a / b.template To<A>(); }
	//! NB: comment block above to remove right hand side (RHS) string arithmetic

/* template <typename T ,typename Tvalue> string_<T> ToString_( const Tvalue &v ) { string_<T> s; s.From<Tvalue>( v ); return s; }

template <typename Tn> string ToString( const Tn &v ) { return ToString_<char_t,Tn>( v ); }
template <typename Tn> stringa ToStringA( const Tn &v ) { return ToString_<char,Tn>( v ); }
template <typename Tn> stringw ToStringW( const Tn &v ) { return ToString_<wchar_t,Tn>( v ); }
*/

// TODO FromString

//////////////////////////////////////////////////////////////////////////
template <typename T> struct STRING_ : string_<T>
{
//-- instance
	STRING_() : string_<T>() {}
	STRING_( const T *s ) : string_<T>(s) {}
	STRING_( const T *s ,int len ) : string_<T>(s,len) {}
	STRING_( const STRING_<T> &copy ) : string_<T>(NULL) { this->_string = dup( (const T*) copy ); }

	~STRING_() { SAFEFREE( this->_string ); }

	template <typename Tn> STRING_( const Tn &v ) : string_<T>() { Set( v ); }

//-- operators
	operator const T *() const { return this->_string; }
	operator int() const { return this->template To<int>(); }

	STRING_ operator()( int a ,int b ){ return this->SubString(a,b); }

	T &operator []( int i ) { return this->At(i); }

	STRING_ &operator =( wchar_t v ) { return this->Set(v); }

	STRING_ &operator =( char v ) { this->Set(v); return *this; }
	STRING_ &operator =( short v ) { this->Set(v); return *this; }
	STRING_ &operator =( int v ) { this->Set(v); return *this; }
	STRING_ &operator =( long v ) { this->Set(v); return *this; }
	STRING_ &operator =( long long v ) { this->Set(v); return *this; }

	STRING_ &operator =( unsigned char v ) { this->Set( v ); return *this; }
	STRING_ &operator =( unsigned short v ) { this->Set( v ); return *this; }
	STRING_ &operator =( unsigned int v ) { this->Set( v ); return *this; }
	STRING_ &operator =( unsigned long v ) { this->Set( v ); return *this; }
	STRING_ &operator =( unsigned long long v ) { this->Set( v ); return *this; }

	STRING_ &operator =( float v ) { this->Set( v ); return *this; }
	STRING_ &operator =( double v ) { this->Set( v ); return *this; }

	STRING_ &operator =( const char *s ) { this->Set( s ); return *this; }
	STRING_ &operator =( const wchar_t *s ) { this->Set( s ); return *this; }

	STRING_ &operator =( const string_<T> &s ) { return operator =( s.Ptr() ); }
	// STRING_ &operator =( const STRING_ &s ) { return operator =( s.Ptr() ); }

	bool operator ==( const T *s ) const { return Compare( s ); }
	bool operator !=( const T *s ) const { return !operator ==( s ); }

	STRING_ &operator +=( const T c ) { Append( c ); return *this; }
	STRING_ &operator +=( const T *s ) { Append( s ); return *this; }
	STRING_ &operator +=( const string_<T> &s ) { Append( (const T*) s ); return *this; }

	template <typename Tn> STRING_ &operator +=( const Tn &v ) { string_<T> s(v); Append( (const T*) s ); return *this; }

	STRING_ operator +( const T c ) const { string_ r( *this ); return r += c; }
	STRING_ operator +( const T *s ) const { string_ r( *this ); return r += s; }
	STRING_ operator +( const string_<T> &s ) const { string_ r( *this ); return r += (const T*) s; }

	template <typename Tn> STRING_ operator +( const Tn &v ) const { string_<T> r( *this ); return r += v; }

	STRING_ &operator *=( int n ) { this->Repeat( n ); return *this; }
	STRING_ operator *( int n ) const { string_ r( *this ); return r *= n; }

	template <typename Tn> STRING_ &operator <<( const Tn &v ) { Append( v ); return *this; }

//-- function
	bool Match( const T *s ) const { return match( this->_string ,s ); }

	bool Compare( const T *s ) const { return cmp( this->_string ,s ) == 0; }
	bool Compare( const T *s ,int length ) const { return ncmp( this->_string ,s ,length ) == 0; }
	// bool Compare( const T *s ,int i ,int length ) const { return ncmp( (_string!=NULL)?(_string+i):NULL ,s ,length ); }

	static int cmp( const char *a ,const char *b ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : stricmp( a ,p ); }
	static int cmp( const wchar_t *a ,const wchar_t *b ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcsicmp( a ,p ); }

	static int ncmp( const char *a ,const char *b ,int n ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : strnicmp( a ,p ,n ); }
	static int ncmp( const wchar_t *a ,const wchar_t *b ,int n ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcsnicmp( a ,p ,n ); }

	static int match( const char *a ,const char *b ) { const char *p = (b!=NULL)?b:""; return (a==NULL||*a==0) ? (0-*p) : strnicmp( a ,p ,string_<T>::len(p) ); }
	static int match( const wchar_t *a ,const wchar_t *b ) { const wchar_t *p = (b!=NULL)?b:L""; return (a==NULL||*a==0) ? (0-*p) : wcsnicmp( a ,p ,string_<T>::len(p) ); }

	//TODO
	// find function that ignores case
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// date / time 

// TODO

//////////////////////////////////////////////////////////////////////////
//! smart ptr

//-- simple smart ptr
template <class T> struct refptr_
{
	//-- instance
	refptr_( int nil = 0 ) : _instance( NULL ) {}
	refptr_( T *instance ) : _instance( NULL ) { Assign( instance ); }
	refptr_( T &instance ) : _instance( NULL ) { Assign( &instance ); }
	refptr_( refptr_ &instance ) : _instance( NULL ) { Assign( instance ); }

	~refptr_() { Assign( NULL ); }

	T *_instance;

	volatile unsigned long _refCount;

	unsigned long AddRef( void ) { return ++_refCount;  }
	unsigned long Release( void ) { if(_refCount && --_refCount == 0) SAFEDELETE( _instance ); return _refCount;  }

	//-- operator
	operator T*() const { return _instance; }
	T* operator ->() const { return _instance; }

	refptr_ &operator =( T *instance ) { Assign( instance ); return *this; }
	refptr_ &operator =( refptr_ &instance ) { Assign( instance ); return *this; }

	bool operator ==( const T *instance ) const { return Compare( instance ); }
	bool operator !=( const T *instance ) const { return !Compare( instance ); }

	//-- function
	void Assign( T *instance ) { if(instance == _instance) return; if(_instance != NULL) { Release(); _instance = NULL; } if(instance != NULL) { _instance = instance; AddRef(); } }
	void Assign( refptr_ &instance ) { Assign( instance._instance ); }
	bool Compare( const T *instance ) const { return _instance == instance; }
	bool IsNull( void ) const { return _instance == NULL; }
};

//-- composite smart ptr
struct referable
{
	virtual unsigned long AddRef( void ) = 0;
	virtual unsigned long Release( void ) = 0;
};

/*
struct refcounted : referable
{
	virtual unsigned long AddRef( void ) { ASYNCH_INCREMENT( _refs ); }
	virtual unsigned long Release( void ) { if( ASYNCH_DECREMENT( _refs ) == 0 ) delete this; }

	ASYNCH_VARIABLE _refs;
};
*/

template <class T> struct reference_ //! 'T' derive from Referable or similar interface (EG IUnknown)
{
	//-- instance
	reference_( int nil=0 ) : _instance(NULL) {}
	reference_( T *instance ) : _instance(NULL) { Assign( instance ); }
	reference_( T &instance ) : _instance(NULL) { Assign( &instance ); }
	reference_( reference_ &instance ) : _instance(NULL) { Assign( instance ); }

	~reference_() { Assign( NULL ); }
		
	T *_instance;

	//-- operator
	operator T*() const { return _instance; }
	T* operator ->() const { return _instance; }

	reference_ &operator =( T *instance ) { Assign( instance ); return *this; }
	reference_ &operator =( reference_ &instance ) { Assign( instance ); return *this; }

	bool operator ==( const T *instance ) const { return Compare( instance ); }
	bool operator !=( const T *instance ) const { return !Compare( instance ); }

	//...

	//-- function
	void Assign( T *instance ) { if( instance == _instance ) return; if( _instance != NULL ) { _instance->Release(); _instance = NULL; } if( instance != NULL ) (_instance = instance)->AddRef(); }
	void Assign( reference_ &instance ) { Assign( instance._instance ); }
	bool Compare( const T *instance ) const { return _instance == instance; }
	bool IsNull( void ) const { return _instance == NULL; }
};

struct uuid
{
	unsigned long  data1;
	unsigned short data2;
	unsigned short data3;
	unsigned char  data4[8];
};

//TODO to/from string

template <typename T> struct component_ : referable
{
	static const T _uid;

	virtual ~component_() {}

	virtual T GetClassID() const { return _uid; }

	//TODO
	/*
	virtual unsigned long AddRef( void ) = 0;
	virtual unsigned long Release( void ) = 0;
	*/

	virtual OsError GetInterface( const T &uid ,void **ppv )
	{ 
		if( uid == _uid )
		{ 
			if( ppv ) *ppv = (void*) ((component_*) this); else return EEXIST; return ENOERROR;
		}

		return ENODATA;
	}

	template <class Ti> Ti *GetInterfaceT()
	{
		void *pv = NULL; 

		if( OS_FAILED(GetInterface( Ti::_uid ,&pv )) ) return NULL;

		return (Ti*) pv;
	}
};

//////////////////////////////////////////////////////////////////////////

// TODO data_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef TINY_VARIANT_OBJECT	
 #define TINY_VARIANT_OBJECT		referable
 //! should derive from referable or some interface that expose 'AddRef' and 'Release'

 #define TINY_VARIANT_CAST(x)		((int) (void*) (x))
 //EG _value._object->To<T>()

 #define TINY_VARIANT_CASTPTR(x)	(x)
 //EG _value._object->Ptr<T>()
 //EG _value._object->Cast<T>()

 #define TINY_VARIANT_OP(o,b)		(o)
 //EG return o.Call( "add" ,b );

#endif

struct variant
{
	enum types
	{
		typeBYTE=0 ,typeWORD ,typeDWORD ,typeQWORD
		,typeCharA ,typeCharW
		,typeShort ,typeInt32 ,typeInt64
		,typeFloat ,typeDouble
		,typeBool
		,typePointer
		,typeStringA ,typeStringW
		,typeObject
	};

	//NB array<variant> ~= struct
	//?TODO variant hold array

	// template <typename T> types typeoftype();

	static size_t sizeoftype( types type ) { switch(type) { 
		case typeBYTE: case typeCharA: return 1; 
		case typeWORD: case typeShort: return 2; 
		case typeDWORD: case typeInt32: case typeFloat: case typeBool: return 4; 
		case typeQWORD: case typeInt64: case typeDouble: return 8;
		default: return sizeof(void*);
	} } 

	size_t sizeoftype() const { return sizeoftype( this->_type ); }

	variant() : _type(typeInt32) { _value._p = NULL; }
	variant( enum types type ) : _type(type) { _value._p = NULL; }
	variant( const variant &v ) : _type(typeInt32) { *this = v; } 
	
	/* 
	...
	explicit variant( const void *p ) : _type(typeInt) { *this = p; }
	explicit variant( stringA &s ) : _type(typeInt) { *this = s; }
	explicit variant( stringW &s ) : _type(typeInt) { *this = s; }
	explicit variant( referable &r ) : _type(typeInt) { *this = r; }
	*/

	/*
	template <typename T> explicit T() { return To<T>(); }
	*/

	~variant() { ReleaseValue(); }

	types Type( void ) const { return _type; }

	bool isFloatingPoint() const { return _type == typeFloat || _type == typeDouble; }

	// template <typename T> A &Cast( B &b ) { return (A) b; }

	//-- arithmetic
/*	struct Add { template <typename B> struct Functor { 
		template <typename A> static variant fun( const A &a ,const B &b ) { return variant() = (a + b); } 
		static variant fun( TINY_VARIANT_OBJECT &a ,const B &b ) { return variant() = TINY_VARIANT_OP(a,b); }
	}; };

	struct Sub { template <typename B> struct Functor { 
		template <typename A> static variant fun( const A &a ,const B &b ) { return variant() = a - b; }
		static variant fun( stringa &a ,const B &b ) { return variant() = a; } 
		static variant fun( stringw &a ,const B &b ) { return variant() = a; } 
		static variant fun( TINY_VARIANT_OBJECT &a ,const B &b ) { return variant() = TINY_VARIANT_OP(a,b); }
		}; 	
	/* template <> struct Functor<stringa> { 
		template <typename A> static variant fun( const A &a ,const stringa &b ) { return variant() = a; } 
	};*/ /*};

	struct Mul { template <typename B> struct Functor { 
		template <typename A> static variant fun( const A &a ,const B &b ) { return variant() = a * b; } 
		static variant fun( stringa &a ,const B &b ) { return variant() = a * (int) b; } 
		static variant fun( stringw &a ,const B &b ) { return variant() = a * (int) b; } 
		static variant fun( TINY_VARIANT_OBJECT &a ,const B &b ) { return variant() = TINY_VARIANT_OP(a,b); }
	}; };

	struct Div { template <typename B> struct Functor { 
		template <typename A> static variant fun( const A &a ,const B &b ) { return variant() = a / b; } 
		static variant fun( stringa &a ,const B &b ) { return variant() = a; } 
		static variant fun( stringw &a ,const B &b ) { return variant() = a; } 
		static variant fun( TINY_VARIANT_OBJECT &a ,const B &b ) { return variant() = TINY_VARIANT_OP(a,b); }
	}; };

	template <typename B ,typename F> variant Op( const B &b ) const { switch( _type ) {
		case typeBYTE : return F::fun( _value._byte ,b );
		case typeWORD : return F::fun( _value._word ,b );
		case typeDWORD : return F::fun( _value._dword ,b );
		case typeQWORD : return F::fun( _value._qword ,b );
		case typeCharA : return F::fun( _value._chara ,b );
		case typeCharW : return F::fun( _value._charw ,b );
		case typeInt32 : return F::fun( _value._int_32 ,b );
		case typeInt64 : return F::fun( _value._int_64 ,b );
		case typeFloat : return F::fun( _value._float ,b );
		case typeDouble : return F::fun( _value._double ,b );
		case typeBool : return variant() = _value._bool; // return F::num<bool>( _value._bool ,b );
		case typePointer : return variant() = _value._p; //! no arithmetic
 		case typeStringA : assert(_value._stringa); return F::fun( *_value._stringa ,b );
 		case typeStringW : assert(_value._stringw); return F::fun( *_value._stringw ,b );
		case typeObject : assert(_value._object); return F::fun( *_value._object ,b );
		default: return variant();
	}}

	template <typename Tn> variant operator +( const Tn &v ) const { return Op<Tn,Add::Functor<Tn> >( v ); }
	template <typename Tn> variant operator -( const Tn &v ) const { return Op<Tn,Sub::Functor<Tn> >( v ); }
	template <typename Tn> variant operator *( const Tn &v ) const { return Op<Tn,Mul::Functor<Tn> >( v ); }
	template <typename Tn> variant operator /( const Tn &v ) const { return Op<Tn,Div::Functor<Tn> >( v ); }

	template <typename F> variant Op( const variant &v ) const { switch( v._type ) {
		case typeBYTE : return Op<byte,typename F::template Functor<byte> >( v._value._byte );
		case typeWORD : return Op<word,typename F::template Functor<word> >( v._value._word );
		case typeDWORD : return Op<dword,typename F::template Functor<dword> >( v._value._dword );
		case typeQWORD : return Op<qword,typename F::template Functor<qword> >( v._value._qword );
		case typeCharA : return Op<char,typename F::template Functor<char> >( v._value._chara );
		case typeCharW : return Op<wchar_t,typename F::template Functor<wchar_t> >( v._value._charw );
		case typeInt32 : return Op<int32_t,typename F::template Functor<int32_t> >( v._value._int_32 );
		case typeInt64 : return Op<int64_t,typename F::template Functor<int64_t> >( v._value._int_64 );
		case typeFloat : return Op<float,typename F::template Functor<float> >( v._value._float );
		case typeDouble : return Op<double,typename F::template Functor<double> >( v._value._double );
		case typeBool : return variant() = _value._bool; //  return Op<bool,F::Functor<bool> >( v._value._bool );
		case typePointer : return variant() = _value._p; //! no arithmetic
		case typeStringA : assert(_value._stringa); return Op<stringa,typename F::template Functor<stringa> >( *v._value._stringa );
		case typeStringW : assert(_value._stringw); return Op<stringw,typename F::template Functor<stringw> >( *v._value._stringw );
		case typeObject : return *this; //! can only be left hand side
		default: return variant();
	}}

	variant operator +( const variant &v ) const { return Op<Add>( v ); }
	variant operator -( const variant &v ) const { return Op<Sub>( v ); }
	variant operator *( const variant &v ) const { return Op<Mul>( v ); }
	variant operator /( const variant &v ) const { return Op<Div>( v ); }
*/
	//-- get
	template <class Tn> Tn To( void ) const { switch( _type ) {
		case typeBYTE : return (Tn) (_value._byte);
		case typeWORD : return (Tn) (_value._word);
		case typeDWORD : return (Tn) (_value._dword);
		case typeCharA : return (Tn) (_value._chara);
		case typeCharW : return (Tn) (_value._charw);
		case typeInt32 : return (Tn) (_value._int_32);
		case typeInt64 : return (Tn) (_value._int_64);
		case typeFloat : return (Tn) (_value._float);
		case typeDouble : return (Tn) (_value._double);
		case typeBool : return (Tn) (_value._bool);
		case typePointer : return * (Tn*) (_value._p);
		case typeStringA : assert(_value._stringa); return _value._stringa->To<Tn>();
		case typeStringW : assert(_value._stringw); return _value._stringw->To<Tn>();
		case typeObject : assert(_value._object); return (Tn) TINY_VARIANT_CAST(_value._object);
		default: return (Tn) 0;
	}}

	template <class T> T *ToPtr( void ) const { switch( _type ) {
		case typeBYTE : return (T*) &(_value._byte);
		case typeWORD : return (T*) &(_value._word);
		case typeDWORD : return (T*) &(_value._dword);
		case typeCharA : return (T*) &(_value._chara);
		case typeCharW : return (T*) &(_value._charw);
		case typeInt32 : return (T*) &(_value._int_32);
		case typeInt64 : return (T*) &(_value._int_64);
		case typeFloat : return (T*) &(_value._float);
		case typeDouble : return (T*) &(_value._double);
		case typeBool : return (T*) &(_value._bool);
		case typePointer : return (T*) (_value._p);
		case typeStringA : assert(_value._stringa); return (T*) _value._stringa->Ptr();
		case typeStringW : assert(_value._stringw); return (T*) _value._stringw->Ptr();
		case typeObject : assert(_value._object); return (T*) TINY_VARIANT_CASTPTR(_value._object);
		default: return (T*) NULL;
	}}

	template <class T> T &ToRef( void ) const { T *p = ToPtr<T>(); assert(p!=NULL); return *p; }

	template <class T> T Convert( void ) { T v = To<T>(); *this = v; return v; }

	//-- set
	variant &operator =( const variant &v )
	{
		ReleaseValue();

		switch( v._type ) {
		case typeStringA : _value._stringa = new stringa( *(v._value._stringa) ); break;
		case typeStringW : _value._stringw = new stringw( *(v._value._stringw) ); break; 
		case typeObject : _value._object = v._value._object; if( _value._object ) _value._object->AddRef(); break;
		default: memcpy( &_value ,&(v._value) ,sizeoftype(v._type) ); break;
		}

		_type = v._type;

		return( *this );
	}

	variant &operator =( byte v ) { ReleaseValue(); _type=typeBYTE; _value._byte = v; return( *this ); }
	variant &operator =( word v ) { ReleaseValue(); _type=typeWORD; _value._word = v; return( *this ); }
	variant &operator =( dword v ) { ReleaseValue(); _type=typeDWORD; _value._dword = v; return( *this ); }
    
	variant &operator =( qword v ) { ReleaseValue(); _type=typeQWORD; _value._qword = v; return( *this ); }

	variant &operator =( char v ) { ReleaseValue(); _type=typeCharA; _value._chara = v; return( *this ); }
	variant &operator =( wchar_t v ) { ReleaseValue(); _type=typeCharW; _value._charw = v; return( *this ); }
	variant &operator =( short v ) { ReleaseValue(); _type=typeShort; _value._short = v; return( *this ); }
	variant &operator =( int32_t v ) { ReleaseValue(); _type=typeInt32; _value._int_32 = v; return( *this ); }
	variant &operator =( int64_t v ) { ReleaseValue(); _type=typeInt64; _value._int_64 = v; return( *this ); }
	variant &operator =( float v ) { ReleaseValue(); _type=typeFloat; _value._float = v; return( *this ); }
	variant &operator =( double v ) { ReleaseValue(); _type=typeDouble; _value._double = v; return( *this ); }

	variant &operator =( bool v ) { ReleaseValue(); _type=typeBool; _value._bool = v; return( *this ); }
	variant &operator =( const void *p ) { ReleaseValue(); _type=typePointer; _value._p = p; return( *this ); } //NB agnostic of pointer type

	variant &operator =( const char *p ) { ReleaseValue(); _type=typeStringA; _value._stringa = new stringa( p ); return( *this ); } 
	variant &operator =( const wchar_t *p ) { ReleaseValue(); _type=typeStringW; _value._stringw = new stringw( p ); return( *this ); } 

	variant &operator =( const stringa &s ) { ReleaseValue(); _type=typeStringA; _value._stringa = new stringa( s ); return( *this ); }
	variant &operator =( const stringw &s ) { ReleaseValue(); _type=typeStringW; _value._stringw = new stringw( s ); return( *this ); }

	variant &operator =( const referable &o ) { ReleaseValue(); _type=typeObject; if( (_value._object = (referable*) &o) != NULL ) _value._object->AddRef(); return( *this ); }
	
	//-- members
//	ULONG AddRefValue( void ) { if( _type==typeObject && _value._object != NULL ) return _value._object->AddRef(); else return( 1 ); }
	inline void ReleaseValue( void ) { switch( _type ) { 
		case typeStringA : SAFEDELETE( _value._stringa ); return;
		case typeStringW : SAFEDELETE( _value._stringw ); return;
		case typeObject : SAFERELEASE( _value._object ); return;
        default: return;
	} }

	types	_type;

	union
	{
		byte		_byte;
		word		_word;
		dword		_dword;
		qword		_qword;

		char		_chara;
		wchar_t		_charw;
		short		_short;
		int32_t		_int_32;
		int64_t		_int_64;

		float		_float;
		double		_double;

		bool		_bool;

		const void *_p;
		//! variant are agnostic of pointer type and will convert as requested (To/ToPtr) without type checking or conversion,
		//! const void because no arithmetic allowed

		stringa		*_stringa;
		stringw		*_stringw;		
		
		TINY_VARIANT_OBJECT	*_object;
		//! NB cannot ref<...> in union

	} _value;
};

//////////////////////////////////////////////////////////////////////////
template <typename T> struct point_ // 2 ,3
{
	point_() : _x(0) ,_y(0) {}
	point_( T c ) : _x(c) ,_y(c) {}
	point_( T x ,T y ) : _x(x) ,_y(y) {}
	point_( const OsPoint &p ) : _x(p.x) ,_y(p.y) {}

	operator OsPoint*() const { return (OsPoint*) this; }
	operator OsPoint&() const { return * (OsPoint*) this; }

	point_ &operator =( int v ) { _x = _y = (T) v; return *this; }

	template <typename Tn> point_ &operator =( const point_<Tn> &p ) { _x = p._x; _y = p._y; return *this; }

	template <typename Tn> point_ &operator +=( Tn c ) { _x += c; _y += c; return *this; }
	template <typename Tn> point_ &operator -=( Tn c ) { _x -= c; _y -= c; return *this; }
    template <typename Tn> point_ &operator *=( Tn c ) { _x *= c; _y *= c; return *this; }
    template <typename Tn> point_ &operator /=( Tn c ) { _x /= c; _y /= c; return *this; }

	template <typename Tn> point_ &operator +=( point_<Tn> p ) { _x += p._x; _y += p._y; return *this; }
	template <typename Tn> point_ &operator -=( const point_<Tn> &p ) { _x -= p._x; _y -= p._y; return *this; }

	template <typename Tn> point_ operator +( const point_<Tn> &p ) const { return point_<T>( _x + p._x ,_y + p._y ); }
	template <typename Tn> point_ operator -( const point_<Tn> &p ) const { return point_<T>( _x - p._x ,_y - p._y ); }

	template <typename Tn> point_ operator *( const point_<Tn> &p ) const { return point_<T>( (T) (_x * p._x) ,(T) (_y * p._y) ); }
	template <typename Tn> point_ operator /( const point_<Tn> &p ) const { return point_<T>( (T) (_x / p._x) ,(T) (_y / p._y) ); }

	point_ &Abs() { _x = abs(_x); _y = abs(_y); return *this; }

	T Max() { return MAX(_x,_y); }
	T Min() { return MIN(_x,_y); }

	T _x ,_y; //TODO except underscore for point and rect to ease switch between those and OsRect / OsPoint
};

typedef point_<int> point;

template <> template <> inline bool string::From<point>( const point &p )
{ 
	Format( "%d,%d" ,32 ,(int) p._x ,(int) p._y ); return true;
}

template <> template <> inline bool string::To<point>( point &p ) const
{ 
	int i=0 ,len=Length(); string s;

	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._x = s.To<int>();
	i = s.Token( _string ,i ,cislistsep ); if( i<=0 ) return false; 
	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._y = s.To<int>();

	return true;
}

typedef point_<float> pointf;

/* TODO for OsPoint
template <> inline rect::operator OsRect*() const { return (OsRect*) this; }
template <> inline rect::operator OsRect&() const { return * (OsRect*) this; }
*/

//////////////////////////////////////////////////////////////////////////
template <typename T> struct rect_
{
	rect_() : _left(0) ,_top(0) ,_right(0) ,_bottom(0) {}
	rect_( T v ) : _left(v) ,_top(v) ,_right(v) ,_bottom(v) {}
	rect_( T left ,T top ,T right ,T bottom ) : _left(left) ,_top(top) ,_right(right) ,_bottom(bottom) {}
	rect_( const point_<T> &topLeft ,const point_<T> &bottomRight ) : _left(topLeft._x) ,_top(topLeft._y) ,_right(bottomRight._x) ,_bottom(bottomRight._y) {}
	rect_( const OsRect &r ) : _left(r.left) ,_top(r.top) ,_right(r.right) ,_bottom(r.bottom) {}

	operator OsRect*() const;
	operator OsRect&() const;

	point_<T> GetTopLeft( void ) const { return point_<T>( _left ,_top ); }
	point_<T> GetTopRight( void ) const { return point_<T>( _right ,_top ); }
	point_<T> GetBottomLeft( void ) const { return point_<T>( _left ,_bottom ); }
	point_<T> GetBottomRight( void ) const { return point_<T>( _right ,_bottom ); }
	point_<T> GetCenter( void ) const { return point_<T>( (_left + _right) / 2 ,(_top + _bottom) / 2 ); }
	point_<T> GetDimensions( void ) const { return point_<T>( GetWidth() ,GetHeight() ); }

	point_<T> GetSize( void ) const { return point_<T>( _right-_left ,_bottom-_top ); }

	int GetWidth( void ) const { return _right - _left; }
	int GetHeight( void ) const { return _bottom - _top; }

	int GetHorizontalCenter( void ) const { return (_left + _right) / 2; }
	int GetVerticalCenter( void ) const { return (_top + _bottom) / 2; }

	template <typename Tn> rect_ &Offset( const point_<Tn> &p ) { _left += p._x; _right += p._x; _top += p._y; _bottom += p._y; return *this; }
	template <typename Tn> rect_ &OffsetInv( const point_<Tn> &p ) { _left -= p._x; _right -= p._x; _top -= p._y; _bottom -= p._y; return *this; }

	template <typename Tn> rect_ &Inflate( const rect_<Tn> &r ) { _left -= r._left; _right += r._right; _top -= r._top; _bottom += r._bottom; return *this; }
	template <typename Tn> rect_ &Deflate( const rect_<Tn> &r ) { _left += r._left; _right -= r._right; _top += r._top; _bottom -= r._bottom; return *this; }

	rect_ &operator =( const rect_ &r ) { _left = r._left; _top = r._top; _right = r._right; _bottom = r._bottom; return *this; }

	template <typename Tn> rect_ &operator +=( const point_<Tn> &p ) { return Offset( p ); }
	template <typename Tn> rect_ &operator -=( const point_<Tn> &p ) { return OffsetInv( p ); }

	template <typename Tn> rect_ &operator +=( const rect_<Tn> &r ) { _left = MIN(_left,r._left); _top = MIN(_top,r._top); _right = MAX(_right,r._right); _bottom = MAX(_bottom,r._bottom); return *this; }

	template <typename Tn> bool operator &( const point_<Tn> &p ) const { return _left <= p._x && _top <= p._y && _right >= p._x && _bottom >= p._y; }

	template <typename Tn> bool operator &( const rect_<Tn> &r ) const { return _right > r._left && _bottom > r._top && _left < r._right  && _top < r._bottom; }

	bool operator &=( const rect_ &r )
	{ 
		if( !operator &(r) ) { *this = 0; return false; }

		_left = MAX(_left,r._left); _top = MAX(_top,r._top); _right = MIN(_right,r._right); _bottom = MIN(_bottom,r._bottom); 
		
		return true;
	}

	T _left ,_top ,_right ,_bottom;
};

typedef rect_<int> rect;

template <> template <> inline bool string::From<rect>( const rect &r )
{ 
	Format( "%d,%d,%d,%d" ,64 ,(int) r._left ,(int) r._top ,(int) r._right ,(int) r._bottom ); return true;
}

template <> template <> inline bool string::To<rect>( rect &p ) const
{ 
	int i=0 ,len=Length(); string s;

	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._left = s.To<int>();
	i = s.Token( _string ,i ,cislistsep ); if( i<=0 ) return false; 
	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._top = s.To<int>();
	i = s.Token( _string ,i ,cislistsep ); if( i<=0 ) return false; 
	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._right = s.To<int>();
	i = s.Token( _string ,i ,cislistsep ); if( i<=0 ) return false; 
	i = s.Token( _string ,i ,cisdigit ); if( i<=0 ) return false; p._bottom = s.To<int>();

	return true;
}

typedef rect_<float>	rectf;

template <> inline rect::operator OsRect*() const { return (OsRect*) this; }
template <> inline rect::operator OsRect&() const { return * (OsRect*) this; }

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//! composite

template <typename Ta ,typename Tb> struct pair_
{
/*
	pair_() {}

	template <typename TTa> pair_( TTa a ) : _a(a) {}
	template <typename TTa ,typename TTb> pair_( TTa a ,TTb b ) : _a(a) ,_b(b) {}
*/

	bool operator ==( const pair_ &p ) { return _a == p._a && _b == p._b; }
	bool operator !=( const pair_ &p ) { return _a != p._a || _b != p._b; }

	Ta _a; Tb _b;
};

template <typename Ta ,typename Tb ,typename Tc> struct triplet_
{
/*
	triplet_() {}

	template <typename TTa> triplet_( TTa a ) : _a(a) {}
	template <typename TTa ,typename TTb> triplet_( TTa a ,TTb b ) : _a(a) ,_b(b) {}
	template <typename TTa ,typename TTb ,typename TTc> triplet_( TTa a ,TTb b ,TTc c ) : _a(a) ,_b(b) ,_c(c) {}
*/

	bool operator ==( const triplet_ &t ) { return _a == t._a && _b == t._b && _c == t._c; }
	bool operator !=( const triplet_ &t ) { return _a != t._a || _b != t._b && _c != t._c; }

	triplet_ operator =( const triplet_ &t ) { _a = t._a; _b = t._b; _c = t._c; return *this; } 

	Ta _a; Tb _b; Tc _c;
};

template <int Tn ,typename T> struct tuple_
{
	// template <typename Tc> Tc Cast();

	bool operator ==( const tuple_ &t ) { for( int i=0; i<Tn; ++i ) if( _values[i] != t._values[i] ) return false; return true; }
	bool operator !=( const tuple_ &t ) { return this->operator !=( t ); }

	tuple_ operator =( const tuple_ &t ) { for( int i=0; i<Tn; ++i ) _values[i] = t._values[i]; return *this; } 

	T &operator[]( int i ) { _ASSERT( i < Tn ); return _values[i]; }

	T _values[Tn];
};

typedef tuple_<2,int>	tuple2i;

// template <> template <> inline pair<2,int> &tuple_<2,int>::Cast() { return (pair<T,T>*) this; }

// template <class T> template <2,T> inline pair<int,int> *tuple_<2,int>::Cast() { return (pair<int,int>*) this; }

//////////////////////////////////////////////////////////////////////////
/*
#define TINYC_ENUMERATOR_OPERATORS(_T_) \
	operator _T_ *() const { return Get(); } \
	_T_* operator ->() const { return Get(); } \
	_T_* operator ++( int ) { _T_ *p = Get(); if( p!=NULL) Next(); return p; } 

	//! NB iterator are private to type
	//!  > both implement 'auto i = begin(); while( (p = i++) != NULL ) {}' iteration/enumeration style
	//!  > only iterator implement 'for( auto i=begin();i!=end();++i ) {}' iteration style

template <typename T> struct enumerator_old
{
	virtual T *Get( void ) = 0;
	virtual T *Next( void ) = 0;

	TINYC_ENUMERATOR_OPERATORS(T);
};

template <typename T> struct enumerator_
{
	struct fun
	{
		virtual T *Get( void ) = 0;
		virtual bool Next( void ) = 0;
		// virtual T *Next( void ) = 0;
	
	} *_f;

	enumerator_( fun *f ) : _f(f) {}
	~enumerator_() { delete _f; }

	operator T *() const { return _f->Get(); } 
	T* operator ->() const { return _f->Get(); }
	bool operator ++( int ) { return _f->Next(); } 
};
*/

#ifdef PLATFORM_32BIT
 typedef uint32_t enumerator;
#else
 typedef uint64_t enumerator;
#endif

template <typename T> struct enumerable
{
	virtual T *each( enumerator &i ) const = 0;
	//! NB set i to 0 for first item 

	// virtual bool each( T *item ) const = 0;

	//! EG while( (item = myEnumerable.each(i)) != NULL ) { item->
};

/* EG
void Iterate( void )
{
	for( auto i = mycollection.first(); i!=mycollection.end(); ++i ) { i-> ... }

	for( auto i = mycollection.last(); i!=mycollection.begin(); --i ) { i-> ... }
}

void Enumerate( void )
{
	auto i = mycollection.begin();
	//OR
	auto i = mycollection.each();

	while( i++ ) { i-> ... }
}
*/

/* template <class A ,class B> struct iterator_iterator : iterator<A> //! iterator type "A" over iterator of type "B"
{
	iterator_iterator( iterator<B> *baseIterator ) : _baseIterator(baseIterator) {}

	virtual bool Next( void ) { return _baseIterator->Next(); }

	iterator<B> *_baseIterator;
}; */

//////////////////////////////////////////////////////////////////////////
//! Containers

/*
inline size_t AlignSize( size_t size ,size_t align=TINY_MEMDEFALIGN ) //! both arg in bytes
{
	assert( align != 0 );

	size_t r = size % align;

	return size + ((r != 0) ? (align - size % align) : 0);
}
*/

//////////////////////////////////////////////////////////////////////////

//!TODO a buffer that act as array without calling constructor or deestructor of items

//////////////////////////////////////////////////////////////////////////
//! sizable array

//NB no constructor or destructor will be called for items 
//  (but array's item memory is initialized (0) at allocation)
//NB deleting an item will not preserve item's order
//  (eg use list<> if item order must be preserved)

#define ARRAY_ITEMNEW(i)		{ new ( (void*) &(_array[(i)]) ) T; }
#define ARRAY_ITEMDELETE(i)		{ m_memory[(i)].~T(); }

template <typename T>
struct array : enumerable<T> {
	array() : _array(NULL) ,_count(0) ,_size(0) {}
	array( int size ) : _array(NULL) ,_size(0) ,_count(0) { Reserve(size); }
	array( const T *manifest ,int count ) : _array(NULL) ,_size(0) ,_count(0) { CopyFrom( manifest ,count ); }
	array( const array &copy ) { this->CopyFrom(copy); }

	~array() { SAFEFREE(_array); _size = _count = 0; }
	
//-- iterator
	template <typename Ti ,typename Tc> struct iterator__
	{
		iterator__( Tc *p ,int i ) : _p(p) ,_i(i) {}

		bool operator ==( const iterator__ &i ) const { return _p == i._p && _i == i._i; }
		bool operator !=( const iterator__ &i ) const { return !operator ==( i ); }

		void operator ++() { ++_i; }
		void operator --() { --_i; }

		bool operator ++( int ) { ++_i; return _i < _p->GetCount(); }

		operator Ti *() const { return & (*_p)[_i]; }
		Ti* operator ->() const { return & (*_p)[_i]; }

		Tc *_p; 

		int _i;
	};

	typedef iterator__< T ,array<T> > iterator;
	typedef iterator__< const T ,const array<T> > const_iterator;

	iterator begin() { return iterator(this,-1); }
	iterator first() { return iterator(this,0); }
	iterator last() { return iterator(this,_count-1); }
	iterator end() { return iterator(this,_count); }

	const_iterator begin() const { return const_iterator(this,-1); }
	const_iterator first() const { return const_iterator(this,0); }
	const_iterator last() const { return const_iterator(this,_count-1); }
	const_iterator end() const { return const_iterator(this,_count); }

//-- enumerator
	// virtual bool each( enumerator i ,T *value ) const { if( i < _ count ) { if( value ) value = &_array[i++]; return true; } else return false; }

	virtual T *each( enumerator &i ) const { return (i < (enumerator) _count) ? &(_array[i++]) : NULL; }

	/*
	template <typename Ti ,typename Tc> struct enumerator_fun : enumerator_<Ti>::fun
	{
		enumerator_fun( Tc *p ,int i ) : _p(p) ,_i(i) {}

		virtual Ti *Get( void ) { return & (*_p)[_i]; }
		virtual bool Next( void )  { _i++; return _i < _p->GetCount(); }

		// virtual Ti *Next( void )  { return ++_i; } // { if( _i >= _p->GetCount() ) return NULL; T *p = return & (*_p)[_i]; ++_i; }

		Tc *_p; 

		int _i;
	};

	typedef enumerator_< T > enumerator;

	typedef enumerator_< const T > const_enumerator;

	enumerator each() { return enumerator( new enumerator_fun<T ,array<T> >(this,-1) ); }
	const_enumerator each() const { return new const_enumerator( new enumerator_fun<const T ,const array<T> >(this,-1) ); }
	*/

//-- accessors
	int getSize() const { return _size; }
	int GetCount() const { return _count; }
	bool IsEmpty() const { return _count == 0; }

	const T *Ptr( int i=0 ) const { assert( i<_count ); return _array + i; }
	T *Ptr( int i = 0 ) { assert( i<_count ); return _array + i; }

	const T *EndPtr() const { return _array + _count; }
	T *EndPtr() { return _array + _count; }

	void Set( const T &item ) { for( int i=0; i<_count; ++i ) _array[i] == item; }
	bool Set( const T &item ,int n ) { if( !Reserve(n) ) return false; _count = n; for( int i=0; i<_count; ++i ) _array[i] = item; return true; }

	// bool CopyFrom( const array<T> &copy ) { int n = copy.GetCount(); if( !Reserve(n) ) return false; memcpy( _array ,copy._array ,n*sizeof(T) ); _count = n; return true; }
	
	bool CopyFrom( const T *manifest ,int n ) { if(!Reserve( n )) return false; for( int i=0; i<n; ++i ) _array[i] = manifest[i]; _count = n; return true; }
	bool CopyFrom( const array<T> &copy ) { int n = copy.GetCount(); if(!Reserve( n )) return false; for( int i=0; i<n; ++i ) _array[i] = copy._array[i]; _count = n; return true; }

//-- operators
	operator const T*() const { return _array; }
	operator T*() { return _array; }

	T &operator[]( int i ) const { assert( i<_count ); return _array[i]; }

	array &operator =( const array &copy ) { CopyFrom( copy ); return *this; }

	array &operator +=( const T &item ) { Append( item ); return *this; }

//-- operation to the front of the array
	// T *Insert( const T &item );
	// int Withdraw( int count=1 );

//-- operation to the back of the array
	T *Append( void ) { if( !Reserve(_count+1) ) return NULL; ARRAY_ITEMNEW(_count); return &(_array[_count++]); }

	T *Append( const T &item ) { if( !Reserve(_count+1) ) return NULL; ARRAY_ITEMNEW(_count); _array[_count] = item; return &(_array[_count++]); }
	
	int Grow( int count=1 ) { if(!Reserve( _count + count )) _count += count;  return _count; }
	int Truncate( int count=1 ) { int n = MAX( _count-count ,0 ); _count = n; return n; } //TODO call ARRAY_ITEMDELETE
	//TODO should be truncateTo

//-- lookup
	int Find( const T &item ) { int i=0; while( i<_count ) if( _array[i] == item ) return i; else ++i; return -1; }

	T *AppendUnique( const T &item ) { int i= Find(item); if( i >= 0 ) return &(_array[i]); else return Append(item); }

//-- random access
	bool Move( int i ,int to )
	{
		if( _count==0 || i>=_count ) return false; if( i==to ) return true;
		
		byte r[sizeof(T)];

		memcpy( &r ,&_array[i] ,sizeof(T) );

		if( i<to )
			memmove( &_array[i] ,&_array[i+1] ,(to-i)*sizeof(T) );
		else
			memmove( &_array[to+1] ,&_array[to] ,(i-to)*sizeof(T) );

		memcpy( &_array[to] ,&r ,sizeof(T) );

		return true;
	}

	//  void Swap( int i ,int to )

	void Remove( int i ) { if( i >= _count || _count == 0 ) return; ARRAY_ITEMDELETE(i); if( i < _count-1 ) memcpy( &_array[i] ,&_array[_count-1] ,sizeof(T) ); --_count; }
	bool Remove( const T &item ) { int i,j=0; while( (i=Find( item )) >= 0 ) Remove(i) ,++j; return j > 0; }
	void Clear( void ) { while( _count > 0 ) { --_count; ARRAY_ITEMDELETE(_count); } }

	void Trash( void ) { _count = 0; } //! CAREFUL ! no destructor will be called

//-- allocation
	bool Reserve( int size ) //! make sure allocated buffer can hold 'size' element
	{ 
		if( size <= _size ) return true;

		size_t allocSize = memory_<T>::size_adjust( size ,TINY_MEMDEFALIGN );

		T *p = (T*) (this->_array ? realloc( this->_array ,allocSize ) : malloc( allocSize ));

		if( p == NULL ) return false;

		memset( (void*) (p+_size) ,0 ,allocSize - _size*sizeof(T) );

        this->_array = p;  _size = size; return true;
	}

	bool ReserveMore( int count = 1 ) { return Reserve( _count + count ); }

	bool Adjust( int size ) //! set array size to 'size' element
	{ 
		if( !Reserve(size) ) return false;

		_TODO; return false;
	}

	void Compact( void ) { Adjust( this->_count ); }
	void Free( void ) { _TODO; }

	void Commit( int n ) { _count = MIN( n ,this->_size ); }

	//-- members
	T *_array; //! actual buffer for the items

	int _count; //! count of element in array
	int _size; //! allocated size of array (in count of element)
};

//-- Array operators
template <typename T> void ArraySet( T *a ,int n ,T v ) { for( int i=0; i<n; ++i ) a[i] = v; }

template <typename T> void ArrayAdd( T *a ,int n ,T v ) { for( int i=0; i<n; ++i ) a[i] += v; }
template <typename T> void ArraySub( T *a ,int n ,T v ) { for( int i=0; i<n; ++i ) a[i] -= v; }
template <typename T> void ArrayMul( T *a ,int n ,T v ) { for( int i=0; i<n; ++i ) a[i] *= v; }
template <typename T> void ArrayDiv( T *a ,int n ,T v ) { for( int i=0; i<n; ++i ) a[i] /= v; }

template <typename T> void ArrayCopy( T *a ,int n ,T *b ) { for( int i=0; i<n; ++i ) a[i] = b[i]; }

template <typename T> void ArrayAdd( T *a ,int n ,T *b ) { for( int i=0; i<n; ++i ) a[i] += b[i]; }
template <typename T> void ArraySub( T *a ,int n ,T *b ) { for( int i=0; i<n; ++i ) a[i] -= b[i]; }
template <typename T> void ArrayMul( T *a ,int n ,T *b ) { for( int i=0; i<n; ++i ) a[i] *= b[i]; }
template <typename T> void ArrayDiv( T *a ,int n ,T *b ) { for( int i=0; i<n; ++i ) a[i] /= b[i]; }

template <typename T> void ArrayPow2( T *a ,int n ) { for( int i=0; i<n; ++i ) a[i]*=a[i]; }

template <typename T> T ArraySum( T *a ,int n ) { T r = 0; for( int i=0; i<n; ++i ) r += a[i]; return r; }

//////////////////////////////////////////////////////////////////////////
template <typename T> struct list : enumerable<T> //! singled linked list
{
	struct entry
	{ 	
		entry( T item ,entry *next ) : _item(item) ,_next(next) {}

		T _item; entry *_next;
	};

	list() : _head(NULL) {}

	~list() { Clear(); }

//-- iterator
	struct iterator // : ::iterator<T>
	{
		iterator( entry *head ) : _entry(head) {}

		// operator bool() { return _entry != NULL; }

		// operator ++

		// operator *(); operator ->();

		virtual T Get( void ) { return _entry->_item; }

		virtual bool Next( void ) { return _entry !=NULL && (_entry = _entry->_next) != NULL; }

		entry *_entry;
	};

//-- enumerator
	virtual T *each( enumerator &i ) const
	{ 
		entry *p = (i==0) ? _head : ((entry*) i)->_next;

		i = (enumerator) p; return p->_item;
	}

	bool Add( T item ) { _head = new entry( item ,_head ); return _head != NULL; }

	// Remove 

	void Clear( void ) { entry *p; while( (p = _head) != NULL ) { _head = _head->_next; delete p; } }

	// ::iterator<T> *First( void ) { return new iterator( _head ); }

	entry *_head;
};

//////////////////////////////////////////////////////////////////////////
template <typename T> struct tape //! double linked list
{
	struct entry
	{ 	
		entry( T item ,entry *prev ,entry *next ) : _item(item) ,_prev(prev) ,_next(next) {}

		T _item; entry *_prev ,*_next;
	};

    entry *_head ,*_tail;

///--
	tape() : _head(NULL)
    {}

	~tape() {
        Clear();
    }

///--
	const T *GetFirst() const {
        return _head ? _head->_item : NULL;
    }

	const T *GetLast() const {
        return _head ? _head->_item : NULL;
    }

///--
	bool Prepend( T item ) {
        entry *p = new entry( item ,NULL ,_head );

        if( p==NULL ) return false; if( _head ) _head->_prev = p; _head = p; return true;
    }

	bool Append( T item ) {
        _head = new entry( item ,_head ); return _head != NULL;
    }

	bool Remove( T item ) {
        assert(false); //TODO

        return false;
    }

	void Clear( void ) {
        entry *p; while( (p = _head) != NULL ) { _head = _head->_next; delete p; }
    }

///--
	bool InsertBefore( T item );
	bool InsertAfter( T item );
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <typename T> struct stack : array<T> //! LIFO
{
	T *Push( const T &item ) { return array<T>::Append( item ); }
	T *Peek( void ) const { if( array<T>::GetCount() == 0 ) return NULL; return *array<T>::last(); }

	bool Peek( T &item ) const { if( array<T>::GetCount() == 0 ) return false; item = *array<T>::last(); return true; }
	bool Pop( T &item ) { if( array<T>::GetCount() == 0 ) return false; item = *array<T>::last(); array<T>::Truncate(); return true; }
};

//////////////////////////////////////////////////////////////////////////
template <class T> struct queue : array<T> //! FIFO
{
	T *Push( const T &item ) { return array<T>::Append( item ); }
	T *Peek( void ) const { if( array<T>::GetCount() == 0 ) return NULL; return *array<T>::first(); }

	bool Pull( T &item ) { if( array<T>::GetCount() == 0 ) return false; item = *array<T>::first(); array<T>::Withdraw(); return true; }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <typename Tkey ,typename Tvalue> struct map : array<pair_<Tkey,Tvalue> > {
	typedef pair_<Tkey,Tvalue> pair; //? entry

//-- instance
	map() {}

	map( const map &copy ) {
        CopyFrom( copy );
    }

	map( const pair *a ,... ) {
		va_list va;

		Append( *a );

		va_start(va,a);
		{
			const pair *n;
			
			while( (n = va_arg(va,pair*)) != NULL )
				Append( *n );
		}
		va_end(va);
	}

//-- lookup
	Tvalue &operator []( const Tkey &key ) {
        return *Add( key );
    }

	template <class K>
	Tvalue &operator []( K key ) {
        return *Add( (Tkey) key );
    }

	array<pair> &operator =( const array<pair> &copy ) {
        this->CopyFrom( copy ); return *this;
    }

	array<pair> &AsArray() { return *this; }

	pair &At( int i ) const {
        return this->_array[i];
    }

	Tvalue *Add( const Tkey &key ) {
        Tvalue *p = Find( key ); return p ? p : Put( key );
    }

	Tvalue *Add( const Tkey &key ,const Tvalue &value ) {
        Tvalue *p = Add( key ); if( p ) *p = value; return p;
    }

    //! IE AddUniquePair (ensure both key and value are unique)
    //! if value doesn't exist -> new key is added or existing key is replaced
    //! if value exist -> old key to this value is replaced with new key
        //! if new key existed (to some other value) it is replaced (removed) as well
	Tvalue *AddUnique( const Tkey &key ,const Tvalue &value ) {
		pair *p = Dig( value ); 
		
		if( p != NULL ) 
		{
			if( p->_a == key ) return &p->_b; //! already in with same key/value

			Remove( p->_a );
		}

		return Add( key ,value );
	}

	Tvalue *Find( const Tkey &key ) const {
        for( int i=0; i<array<pair>::GetCount(); ++i ) if( array<pair>::_array[i]._a == key ) return &(array<pair>::_array[i]._b); return NULL;
    }

	bool Remove( const Tkey &key ) {
        for( int i=0; i<array<pair>::GetCount(); ++i ) if( array<pair>::_array[i]._a == key ) { array<pair>::Remove(i); return true; } return false;
    }

	void Clear( void ) {
        array<pair>::Clear();
    }

//-- private (don't use for normal operation)
	Tvalue *Put( const Tkey &key ) { pair *p = array<pair>::Append(); if( p ) { p->_a=key; return &(p->_b); } return NULL; }

	// Tvalue *Put( const Tkey &key ,const Tvalue &value ) { pair *p = array<pair>::Append( pair(key,value) ); if( p ) return &(p->_b); return NULL; }

	pair *Dig( const Tvalue &value ) const {
        for( int i=0; i<array<pair>::GetCount(); ++i ) {
            if( this->_array[i]._b == value )
                return &(this->_array[i]);
        }

        return NULL;
    }
	//! find entry matching value (first matching entry is returned)
};

//////////////////////////////////////////////////////////////////////////
//! static map can be used with static initializer

/* E.G.

staticmap<int,STRING>::entry _myManifest[] = 
{
	{ 16 ,_T("Left") }
	,{ 32 ,_T("Right") }
	//...
};

staticmap<int,STRING> _myMap( _myManifest ,ARRAYSIZEOF(_myManifest) );

*/

//?? how to use map or staticmap alike (pass a staticmap for reading where map is expected)...

template <typename Ta ,typename Tb> struct staticmap
{
	typedef pair_<Ta,Tb> entry;

	const entry *_manifest;

	int _entries;

	template <typename Ti ,typename Tc> struct iterator__
	{
		iterator__( Tc *p ,int i ) : _p(p) ,_i(i) {}

		bool operator ==( const iterator__ &i ) const { return _p == i._p && _i == i._i; }
		bool operator !=( const iterator__ &i ) const { return !operator ==( i ); }

		void operator ++() { ++_i; }
		void operator --() { --_i; }

		bool operator ++( int ) { ++_i; return _i < _p->GetCount(); }

		operator Ti *() const { return & (*_p)[_i]; }
		Ti* operator ->() const { return & (*_p)[_i]; }

		Tc *_p; 

		int _i;
	};

	staticmap( entry *manifest ,int entries ) : _manifest(manifest) ,_entries(entries) {}

	const entry *GetManifest() const { return _manifest; }

	int GetCount() const { return _entries; }

	const Ta *FindLHS( const Tb& b ) const
	{
		int i=0; if( _manifest ) while( i < _entries ) if( _manifest[i]._b == b ) return &_manifest[i]._a; else ++i;

		return NULL;
	}

	const Tb *FindRHS( const Ta &a ) const
	{
		int i=0; if( _manifest ) while( i < _entries ) if( _manifest[i]._a == a ) return &_manifest[i]._b; else ++i;

		return NULL;
	}

	//! NB Get function return default [0] entry if key was not found
	const Ta &GetLHS( const Tb& b ) const
	{
		int i=0; if( _manifest ) while( i < _entries ) if( _manifest[i]._b == b ) return _manifest[i]._a; else ++i;

		return _manifest[0]._a;
	}

	const Tb &GetRHS( const Ta &a ) const
	{
		int i=0; if( _manifest ) while( i < _entries ) if( _manifest[i]._a == a ) return _manifest[i]._b; else ++i;

		return _manifest[0]._b;
	}
};

//////////////////////////////////////////////////////////////////////////

//!!! TODO Enums increment auto 'i' (less error prone and coherent with 'each')

//! an int (sparse) indexed map

template <typename T> struct table : map<int,T> {
    typedef map<int,T> super_t;

    using map<int,T>::Find;

    map<int,T> &AsMap() { return *this; }
	const map<int,T> &AsMap() const { return *this; }


	T &operator []( int id ) { return map<int,T>::operator[]( id ); }

	T *Get( int id ) {
        return this->Add(id);
    }

	bool Set( int id ,const T &item );
	bool Enum( int i ,T **item ,int &id ) const;
	bool Find( int id ,T **item ) const;
	bool Dig( const T &item ,int &id );
	bool Remove( int id );

	//-- by value
	bool Enum( int i ,T *item ,int &id ) const { return Enum( i ,&item ,id ); }
	bool Find( int id ,T *item ) const { return Find( id ,&item ); }

	// void Digest( const map<STRING,T> &m ,const map<STRING,int> &nameMap )
		// condense m in a table providing name mapping

	// template <typename Ti> void Digest( const map<Ti,T> &m ,const map<Ti,int> &index );
	// void Expand( map<STRING,T> &m ,const map<STRING,int> &nameMap )

	// from map.Convert( ... );
};

//--
template <typename T> bool table<T>::Set( int id ,const T &item )
{
	T *p = this->Add( id );

	if( p == NULL ) return false;

	*p = item;

	return true;
}

template <typename T> bool table<T>::Enum( int i ,T **item ,int &id ) const {
	if( i >= this->_map.GetCount() ) return false;

	const typename map<int,T*>::pair &p = this->_map.m_memory[i];

	if( item ) *item = p._b;

	id = p._a;

	return true;
}

template <typename T> bool table<T>::Find( int id ,T **item ) const {
	T *find = this->_map.Find( id );

	if( find == NULL ) return false;

	if( item ) *item = find;

	return true;
}

template <typename T>
bool table<T>::Dig( const T &item ,int &id ) {
	typename map<STRING,T*>::pair *p = this->_map.Dig( item );

	if( p == NULL ) return false;

	id = p->_a;

	return true;
}

template <typename T> bool table<T>::Remove( int id ) {
	return this->_map.Remove( id );
}

//////////////////////////////////////////////////////////////////////////
//! a name (case insensitive) indexed map

template <typename T>
struct registry : map<STRING,T> {
	map<STRING,T> &AsMap() { return *this; }
	const map<STRING,T> &AsMap() const { return *this; }

	using map<STRING,T>::Find;

	T &operator []( const char_t *name ) {
        return map<STRING,T>::operator[]( name );
    }

	// int GetCount() const { return _map.GetCount(); }

	/*
	bool Register( const char_t *name ,T *item );
	bool Enum( int index ,T **item ,string &name );
	bool Find( const char_t *name ,T **item );
	bool Dig( T *item ,string &name );
	void Revoke( const char_t *name );
	*/

	T *Get( const char_t *name ) {
        return map<STRING ,T>::Add( STRING(name) );
    }

	bool Register( const char_t *name ,const T &item );
	bool Enum( int i ,T **item ,string &name ) const;
	bool Find( const char_t *name ,T **item ) const;
	bool Dig( const T &item ,string &name );
	bool Revoke( const char_t *name );

	//-- by value
	bool Enum( int i ,T *item ,string &name ) const {
        T *p=NULL; if( Enum( i ,&p ,name ) ) { *item = *p; return true; } return false;
    }

    bool Find( const char_t *name ,T *item ) const {
        T *p=NULL; if( Find( name ,&p ) ) { *item = *p; return true; } return false;
    }

	//-- duct tape for table
	bool Set( const char_t *name ,const T &item ) {
        return Register( name ,item );
    }

	bool Remove( const char_t *name ) {
        return Revoke( name );
    }

	// map<STRING,T*> _map;
};

template <typename T> bool registry<T>::Register( const char_t *name ,const T &item ) {
	if( name == NULL ) return false;

	T *p = this->Add( name );

	if( p == NULL ) return false;

	*p = item;

	return true;
}

template <typename T> bool registry<T>::Enum( int i ,T **item ,string &name ) const {
	if( i >= this->GetCount() ) return false;

	typename map<STRING,T>::pair &p = map<STRING,T>::_array[i];

	if( item ) *item = &(p._b);

	name = p._a;

	return true;
}

template <typename T> bool registry<T>::Find( const char_t *name ,T **item ) const {
	if( name == NULL ) return false;

	T *find = map<STRING,T>::Find( name );

	if( find == NULL ) return false;

	if( item ) *item = find;

	return true;
}

template <typename T>
bool registry<T>::Dig( const T &item ,string &name ) {
	typename map<STRING,T*>::pair *p = map<STRING,T>::Dig( item );

	if( p == NULL  ) return false;

	name = p->_a;

	return true;
}

template <typename T>
bool registry<T>::Revoke( const char_t *name ) {
	return map<STRING,T>::Remove( name );
}

typedef registry<string> StringRegistry;

/*
template <typename T> bool registry<T>::Register( const char_t *name ,T *item )
{
	_map[name] = item;

	return true;
}

template <typename T> bool registry<T>::Enum( int index ,T **item ,string &name )
{
	if( item == NULL || index >= _map.GetCount() ) return false;

	map<STRING,T*>::pair &p = _map._array[index];

	*item = p._b;
	name = p._a;

	return true;
}

template <typename T> bool registry<T>::Find( const char_t *name ,T **item )
{
	if( name == NULL || item == NULL ) return false;

	T **find = _map.Find( name );

	if( find == NULL ) return false;

	*item = *find;

	return true;
}

template <typename T> bool registry<T>::Dig( T *item ,string &name )
{
	map<STRING,T*>::pair *pair = _map.Dig( item );

	if( pair == NULL ) return false;

	name = pair->_a;

	return true;
}

template <typename T> void registry<T>::Revoke( const char_t *name )
{
	_map.Remove( name );
}
*/

//////////////////////////////////////////////////////////////////////////
//! store

//! IE an int (dense) + name (optional) indexed map 

template <typename T> struct store
{
	//! Store item ids are a mean to index item faster than with name,
	//!    IE id set must be dense (from  0..n)

	int GetCount() const { return _items.GetCount(); }

	bool FindName( int id ,string &name ) const { map<STRING,int>::pair *p = _names.Dig( id ); if( p ) name = p->_a; else return false; return true; }

	T *SetItem( int id ,const T &value ); //! will create item if doesn't exist only if id is equal to the next available id
	T *SetItem( const char *name ,const T &value ); //! will create item if doesn't exist

	//! NB adding property by name will set Item ID incrementally from 0..n
	T *AddItem( const char_t *name );
	T *AddItem( const char_t *name ,const T &value );

	//! FindItem return NULL if not found
	const T *FindItem( int id ) const;
	const T *FindItem( const char_t *name ) const;

	T *FindItem( int id );
	T *FindItem( const char_t *name );

	//! GetItem will add the item if it doesn't exist (will throw if id is not dense)
	const T &GetItem( int id ) const; //! throw (will not add item if doesn't exist)
	const T &GetItem( const char_t *name ) const; //! throw (will not add item if doesn't exist)

	T &GetItem( int id ); //! throw (will add item following SetItem( int id ) rule )
	T &GetItem( const char_t *name ); //! throw (will add resource if doesn't exist)

	T GetItemCopy( int id ) const; //! return 0 if item doesn't exist (only use if T = 0 is defined)
	T GetItemCopy( const char_t *name ) const;

	array<T> _items;

	map<STRING,int> _names;
};

template <typename T> T *store<T>::AddItem( const char_t *name )
{
	int *id = _names.Find( name );

	if( id ) return &_items[*id];

	T *item = _items.Append();

	if( item == NULL ) return NULL;

	id = _names.Add( STRING(name) );

	if( id ) *id = _items.GetCount() -1;

	return item;
}

template <typename T> T *store<T>::AddItem( const char_t *name ,const T &value )
{
	T *item = AddItem( name );

	if( item ) *item = value;

	return item;
}

template <typename T> const T *store<T>::FindItem( int id ) const
{
	if( id >= _items.GetCount() ) return NULL;

	return &( _items[id] );
}

template <typename T> const T *store<T>::FindItem( const char_t *name ) const
{
	int *id = _names.Find( name );

	if( id == NULL ) return NULL;;

	return &( _items[ *id ] );
}

template <typename T> T *store<T>::FindItem( int id )
{
	if( id >= _items.GetCount() ) return NULL;

	return &( _items[id] );
}

template <typename T> T *store<T>::FindItem( const char_t *name )
{
	int *id = _names.Find( name );

	if( id == NULL ) return NULL;;

	return &( _items[ *id ] );
}

template <typename T> const T &store<T>::GetItem( int id ) const
{
	if( id >= _items.GetCount() ) throw;

	return _items[id];
}

template <typename T> const T &store<T>::GetItem( const char_t *name ) const
{
	int *id = _names.Find( STRING(name) );

	if( id == NULL ) return GetItem( 0 );

	return GetItem( *id );
}

template <typename T> T &store<T>::GetItem( int id )
{
	if( id >= _items.GetCount() ) throw;

	return _items[id];
}

//////////////////////////////////////////////////////////////////////////
template <class T ,typename Ti> struct factory
{
	typedef T *(*FactorFunctionPtr)( void );

	template <class Tn> static T *FactorFunction( void ) { return new Tn; }

	struct FactorManifest { Ti _classId; FactorFunctionPtr _factorFunction; };

	//-- instance
	factory() {}
	factory( const FactorManifest *manifest ) { RegisterClassManifest( manifest ); }

	~factory() { _registry.Clear(); }

	//-- interface
	T *Factor( const Ti &classId ) { FactorFunctionPtr *p = _registry.Find( classId ); if( p == NULL ) return NULL; else return (*p)(); }

	//-- registration
	bool RegisterClassFactory( const Ti &classId ,FactorFunctionPtr function ) { _registry[classId] = function; return true; }

	bool RegisterClassManifest( const FactorManifest *manifest )
	{ 
		if( manifest == NULL ) return true;

		int i=0; while( manifest[i]._factorFunction )
		{
			RegisterClassFactory( manifest[i]._classId ,manifest[i]._factorFunction );

			++i;
		}

		return true;
	}

	void RevokeClassFactory( const Ti &classId ) { _registry.Remove( classId ); }

	//-- member
	map<Ti,FactorFunctionPtr> _registry;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
}; //TINY_NAMESPACE

///////////////////////////////////////////////////////////////////////////////
//EOF