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

#ifndef TINY_BASE_H
#define TINY_BASE_H

//////////////////////////////////////////////////////////////////////////////
#include <cstring>
#include <fstream>
#include <limits>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////
//! @note template definition below in global space to allow in place specialization
//!   from within any namespace

//TODO append '_' to all names below
//TODO capitalize all

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//! Common functions

//! @function Zero
//! @brief sets element and all its members to zero or empty
template <typename T> T &Zero( T &p ) { memset( &p ,0 ,sizeof(T) ); return p; }

//! @function Init
//! @brief sets element and all its members to default value
template <typename T> T &Init( T &p ) { Zero(p); return p; }

template <typename T> T &Copy( T &a ,const T &b ) { memcpy( &a ,&b ,sizeof(T) ); return a; }
template <typename T> size_t Size( const T &a ) { return sizeof(a); }
template <typename T> int Compare( const T &a ,const T &b ) { return memcmp( &a ,&b ,sizeof(T) ); }
template <typename T> bool Equals( const T &a ,const T &b ) { return Compare<T>( &a ,&b ) == 0; }

template<>
inline int Compare( const float &a ,const float &b ) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

template<>
inline int Compare( const double &a ,const double &b ) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

template<>
inline bool Equals( const float &a ,const float &b ) {
    return fabsf( a - b ) < FLT_EPSILON;
}

template<>
inline bool Equals( const double &a ,const double &b ) {
    return fabs( a - b ) < DBL_EPSILON;
}

//////////////////////////////////////////////////////////////////////////////
//! Class helper

//! @brief decorate a class with a private (hidden) structure

template <class T>
struct Private_; //! @note no default, must be defined for each instantiation

template <class T>
class WithPrivate_ {
protected:
    explicit WithPrivate_( Private_<T> *a_private ) : m_private(a_private) {}

    const Private_<T> &priv() const { return *m_private; }

    Private_<T> &priv() { return *m_private; }

    Private_<T> *m_private;
};

/** @exemple

//! in header
class MyClass : private WithPrivate_<MyClass> {
public:
    MyClass();
 ...
};

//! in source
template <>
struct Private_<MyClass> {
    int internal_counter;
};

MyClass::MyClass() : WithPrivate_<MyClass>( new Private_<MyClass>() ) {
    priv().internal_counter = 0;
}

~MyClass() {
    delete m_private;
}
 */

//////////////////////////////////////////////////////////////////////////////
//! Numeric

inline double RoundDecimal( double v ,int decimal ) {
    double r = pow( 10. ,(double) decimal );

    return (double) ((uint64_t) (v * r)) / r;
}

//////////////////////////////////////////////////////////////////////////////
//! String

#define TINY_STRING_UUID    0x0ef344caa2feef6b1

DECLARE_TCLASS(String,TINY_STRING_UUID);

template <> inline String &Zero( String &p ) { p.clear(); return p; }
template <> inline String &Copy( String &a ,const String &b ) { a = b; return a; }
template <> inline size_t Size( const String &a ) { return a.size(); }
template <> inline int Compare( const String &a ,const String &b ) { return a.compare(b); }
template <> inline bool Equals( const String &a ,const String &b ) { return a == b; }

///-- from
template <typename T>
T &fromString( T &p ,const String &s ,size_t &size );
    //! @note no default implementation by design
    //! @note fromString implementation should not set 'p' if an empty string is provided

template <typename T>
T &fromString( T &p ,const String &s ) {
    size_t size; return fromString( p ,s ,size );
}

template <typename T>
T fromString( const String &s ) {
    T value; fromString( value ,s ); return value;
}

//-- function
#define INLINE_FROMSTRING(__class) \
    template <> inline __class &fromString( __class &p ,const String &s ,size_t &size )

#define DEFINE_FROMSTRING(__class) \
    template <> __class &fromString( __class &p ,const String &s ,size_t &size )

//-- class
#define DECLARE_FROMSTRING(__class) \
    __class &fromString( const String &s ) { size_t size; return this->fromString( s ,size ); } \
    __class &fromString( const String &s ,size_t &size )

#define CLASS_FROMSTRING(__class) \
    IMPLEMENT_FROMSTRING(__class) { p.fromString( s ,size ); }

///-- to
template <typename T>
String &toString( const T &p ,String &s ); //! @note no default by design

template <typename T>
String toString( const T &p ) {
    String s; toString( p ,s ); return s;
}

//-- function
#define INLINE_TOSTRING(__class) \
    template <> inline String &toString( const __class &p ,String &s )

#define DEFINE_TOSTRING(__class) \
    template <> String &toString( const __class &p ,String &s )

//-- class
#define DECLARE_TOSTRING(__class) \
    String &toString( String &s ) const

#define CLASS_TOSTRING(__class) \
    IMPLEMENT_TOSTRING(__class) { p.toString( s ); }

///--
#define DEFINE_STRING_API(__class) \
    DEFINE_FROMSTRING(__class); \
    DEFINE_TOSTRING(__class);

#define DECLARE_STRING_API(__class) \
    DECLARE_FROMSTRING(__class); \
    DECLARE_TOSTRING(__class);

//////////////////////////////////////////////////////////////////////////////
//! native support

template <> inline String &fromString( String &p ,const String &s ,size_t &size ) {
    p = s; size = p.size(); return p;
}

template <> inline String &toString( const String &p ,String &s ) {
    return s = p;
}

//--
template <> double &fromString( double &p ,const String &s ,size_t &size );
template <> float &fromString( float &p ,const String &s ,size_t &size );

template <> long &fromString( long &p ,const String &s ,size_t &size );
template <> int &fromString( int &p ,const String &s ,size_t &size );
template <> short &fromString( short &p ,const String &s ,size_t &size );

template <> unsigned long &fromString( unsigned long &p ,const String &s ,size_t &size );
template <> unsigned int &fromString( unsigned int &p ,const String &s ,size_t &size );
template <> unsigned short &fromString( unsigned short &p ,const String &s ,size_t &size );

template <> bool &fromString( bool &p ,const String &s ,size_t &size );

//--
template <> inline String &toString( const long double &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const double &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const float &p ,String &s ) { s = std::to_string(p); return s; }

template <> inline String &toString( const long long &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const long &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const int &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const short &p ,String &s ) { s = std::to_string(p); return s; }

template <> inline String &toString( const unsigned long long &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const unsigned long &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const unsigned int &p ,String &s ) { s = std::to_string(p); return s; }
template <> inline String &toString( const unsigned short &p ,String &s ) { s = std::to_string(p); return s; }

template <> String &toString( const bool &p ,String &s );

///-- extended
struct Percent { float value; };

template <> Percent &fromString( Percent &p ,const String &s ,size_t &size );
template <> String &toString( const Percent &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! Format / Parse

size_t Format( String &s ,const char *format ,size_t maxsize ,... );

template <typename T>
size_t Parse( const char *&s ,T &&accept ,String *capture=NullPtr ,bool captureLimits=true ) {
    size_t i=0 ,head=0 ,skip=0 ,tail=0; if( !s ) return 0;

//-- first
    if( *s && accept(i,s,head) ) {
        i += MAX( head ,1 ) ,s += MAX( head ,1 );
    } else {
        s+=head; if( head && capture ) capture->clear();
        return head;
    }

//-- inner
    while( *s && accept(i,s,skip) ) i+=MAX(skip,1) ,s+=MAX(skip,1);

//-- last
    tail = *s ? skip : 0; //! last skip is tail

    i += tail; s += tail;

//-- capture
    if( capture ) {
        *capture = captureLimits ? String( s-i ,i ) : String( s-i+head ,i-head-tail );
    }

    return i;
}

//////////////////////////////////////////////////////////////////////////////
//! NameType

struct NameType {
    String name;
    String type;
};

template <>
NameType &fromString( NameType &p ,const String &s ,size_t &size );

template <>
std::string &toString( const NameType &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! Pair

struct KeyValue {
    String key;
    String value;
};

//! @brief <key> '=' <value> // ';' ending // quoted text // {} recursive block

template <>
KeyValue &fromString( KeyValue &p ,const String &s ,size_t &size );

template <>
std::string &toString( const KeyValue &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! Params

// struct Params : MapOf<String,String> {}; //TODO ?

typedef MapOf<String,String> Params;

//! @brief a set of KeyValue

template <>
Params &fromString( Params &p ,const String &s ,size_t &size );

template <>
String &toString( const Params &p ,String &s );

bool hasMember( const Params &p ,const char *key );
const String *peekMember( const Params &p ,const char *key );
const char *getMember( const Params &p ,const char *key ,const char *defaultValue="" );
Params &addMembers( Params &p ,const Params &params ,bool addEmpty=true );

template <typename T>
T getMember_( const Params p ,const char *key ,const char *defaultValue="" ) {
    String s = getMember( p ,key ,defaultValue );

    T v; return fromString( v ,s );
}

//////////////////////////////////////////////////////////////////////////////
//! ParamList

    //! @brief ParamList preserve insertion order

struct ParamList : ListOf<KeyValue> {};

template <>
ParamList &fromString( ParamList &p ,const String &s ,size_t &size );

template <>
String &toString( const ParamList &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! StringList

struct StringList : ListOf<String> {};

//! @brief comma separated list //  ';' ending // quoted text // {} recursive block

///-- string
template <>
StringList &fromString( StringList &p ,const String &s ,size_t &size );

template <>
String &toString( const StringList &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
//! Manifest

///-- from
template <typename T ,class TManifest>
T &fromManifest( T &p ,const TManifest &manifest );

#define DEFINE_FROMMANIFEST(__class,__manifest) \
    template <> __class &fromManifest( __class &p ,const __manifest &manifest )

#define INLINE_FROMMANIFEST(__class,__manifest) \
    template <> inline __class &fromManifest( __class &p ,const __manifest &manifest )

#define DECLARE_FROMMANIFEST(__class,__manifest) \
    __class &fromManifest( const __manifest &manifest )

#define CLASS_FROMMANIFEST(__class,__manifest) \
    template <> inline __class &fromManifest( __class &p ,const __manifest &manifest ) { return p.fromManifest( manifest ); }

///-- to
template <typename T ,class TManifest>
TManifest &toManifest( const T &p ,TManifest &manifest );

#define DEFINE_TOMANIFEST(__class,__manifest) \
    template <> __manifest  &toManifest( const __class &p ,__manifest &manifest )

#define INLINE_TOMANIFEST(__class,__manifest) \
    template <> inline __manifest  &toManifest( const __class &p ,__manifest &manifest )

#define DECLARE_TOMANIFEST(__class,__manifest) \
    __manifest toManifest() const { __manifest m; return toManifest( m ); } \
    __manifest &toManifest( __manifest &manifest ) const

#define CLASS_TOMANIFEST(__class,__manifest) \
    template <> inline __manifest &toManifest( const __class &p ,__manifest &manifest ) { return p.toManifest( manifest ); }

///-- both
#define DEFINE_MANIFEST_API(__class,__manifest) \
    DEFINE_FROMMANIFEST(__class,__manifest); \
    DEFINE_TOMANIFEST(__class,__manifest);

#define DECLARE_MANIFEST_API(__class,__manifest) \
    DECLARE_FROMMANIFEST(__class,__manifest); \
    DECLARE_TOMANIFEST(__class,__manifest);

#define CLASS_MANIFEST(__class,__manifest) \
    CLASS_FROMMANIFEST(__class,__manifest) \
    CLASS_TOMANIFEST(__class,__manifest)

///--
    //! @brief using Params as a default medium to resolve string as manifest

template <typename T>
T &fromManifestWithString( T &p ,const char *s ) {
    Params params;

    fromString( params ,String(s) );

    fromManifest( p ,params );

    return p;
}

//////////////////////////////////////////////////////////////////////////////
//! Schema

struct Schema : ListOf<NameType> {
    static Schema &getStatic() { schema.clear(); return schema; }

    static Schema schema; //! may be used for constructing schema from string
};

DEFINE_STRING_API(Schema);

//--
template <class T>
void setMember( T &p ,int m ,const String &s );

#define DEFINE_SETMEMBER(__class) \
    template <> void setMember( __class &p ,int m ,const String &s )
#define INLINE_SETMEMBER(__class) \
    template <> inline void setMember( __class &p ,int m ,const String &s )

//--
template <class T>
String &getMember( const T &p ,int m ,String &s );

#define DEFINE_GETMEMBER(__class) \
    template <> String &getMember( const __class &p ,int m ,String &s )
#define INLINE_GETMEMBER(__class) \
    template <> inline String &getMember( const __class &p ,int m ,String &s )

//--
#define DEFINE_MEMBER_API(__class) \
    DEFINE_SETMEMBER(__class); \
    DEFINE_GETMEMBER(__class);

///--
template <class T>
struct Schema_ {
    static const Schema schema;
};

#define DECLARE_SCHEMA \
    void setMember( int m ,const String &s ); \
    String &getMember( int m ,String &s ) const; \

#define CLASS_SCHEMA(__class) \
    template <> inline void setMember( __class &p ,int m ,const String &s ) { p.setMember(m,s); } \
    template <> inline String &getMember( const __class &p ,int m ,String &s ) { return p.getMember(m,s); }

///--
int getMemberId( const char *name ,const Schema &schema );

Params &listToParams( Params &manifest ,const Schema &schema ,const StringList &values );
StringList &paramsToList( StringList &values ,const Schema &schema ,Params &manifest );

//TODO OUT
template <typename T>
T &fromStringWithSchema( T &p ,const String &s ,const StringList &schema ,size_t size ) {
    _TODO; return p;
}

template <typename T>
String &toStringWithSchema( const T &p ,String &s ,const StringList &schema ) {
    _TODO; return s;
}

///-- String
template <typename T>
T &fromStringWithSchema( T &p ,const String &s ,const Schema &schema ,size_t size ) {
    StringList list;

    fromString( list ,s ,size );

    Params manifest;

    listToParams( manifest ,schema ,list );

    fromManifest( p ,manifest );

    return p;
}

template <typename T>
String &toStringWithSchema( const T &p ,String &s ,const Schema &schema ) {
    Params manifest;

    toManifest( p ,manifest );

    StringList list;

    paramsToList( list ,schema ,manifest );

    toString( list ,s );

    return s;
}

///-- Manifest
template <typename T >
T &fromParamsWithSchema( T &p ,const Params &manifest ) {
    for( auto &it : manifest ) {
        auto &name = it.first;

        int m = getMemberId( name.c_str() ,Schema_<T>::schema );

        if( m < 0 ) continue; //? return false or throw ?

        setMember( p ,m ,it.second );
    }

    return p;
}

template <typename T>
Params &toParamsWithSchema( const T &p ,Params &manifest ) {
    String s;

    int m=0; for( auto &it : Schema_<T>::schema ) {
        getMember( p ,m ,s );

        if( !s.empty() ) //TODO check if we need to make this an option ?
            manifest[it.name] = s;

        ++m;
    }

    return manifest;
}

#define DEFINE_WITHSCHEMA_API(__class) \
    INLINE_FROMSTRING(__class) { return fromStringWithSchema( p ,s ,Schema_<__class>::schema ,size ); } \
    INLINE_TOSTRING(__class) { return toStringWithSchema( p ,s ,Schema_<__class>::schema ); } \
    INLINE_FROMMANIFEST(__class,Params) { return fromParamsWithSchema( p ,manifest ); } \
    INLINE_TOMANIFEST(__class,Params) { return toParamsWithSchema( p ,manifest ); }

//////////////////////////////////////////////////////////////////////////////
//! Convert

template <class TA ,class TB>
TB &Convert_( const TA &from ,TB &to ) {
    return to = (TB) from;
}

template <class TA ,class TB>
TB &Cast_( TA &a ,TB &b ) {
    return static_cast<TB>( a );
}

//////////////////////////////////////////////////////////////////////////////
//! Serialization

template <class TStream ,typename T>
T &Serialize( TStream &p ,const T &a );

template <class TStream ,typename T>
T &Unserialize( TStream &p ,T &a );

///-- binary to serialize wrapper
template <typename T>
struct Bits {
    T operator ()() { return p; }
    T p;
};

template <typename T> Bits<const T&> tobits( const T &p ) { return Bits<const T&>{p}; }
template <typename T> Bits<T&> tobits( T &p ) { return Bits<T&>{p}; }

///-- Default
template <typename T>
OutStream &operator <<( OutStream &out ,Bits<T> p ) {
    return out.write( reinterpret_cast<const char*>( &(p.p) ) ,sizeof(T) );
}

template <typename T>
InStream &operator >>( InStream &in ,Bits<T> p ) {
    return in.read( reinterpret_cast<char*>( &(p.p) ) ,sizeof(T) );
}

///-- String
template <> std::ostream &operator <<( std::ostream &out ,Bits<const String&> p );
template <> std::istream &operator >>( std::istream &in ,Bits<String&> p );

///-- List
//TODO

///-- Map (std)
template< typename TKey ,typename TValue
    ,typename _Compare = std::less<TKey>
    ,typename _Alloc = std::allocator< std::pair<const TKey ,TValue> >
>
std::ostream &operator <<( std::ostream &out ,Bits<const std::map<TKey,TValue> &> p ) {
    size_t size = p().size();

    out << tobits(size);

    for( const auto &it : p() ) {
        out << tobits(it.first) << tobits(it.second);
    }

    return out;
}

template< typename TKey ,typename TValue
    ,typename _Compare = std::less<TKey>
    ,typename _Alloc = std::allocator< std::pair<const TKey ,TValue> >
>
std::istream &operator >>( std::istream &in ,Bits< std::map<TKey,TValue> &> p ) {
    size_t size = 0;

    in >> tobits(size);

    TKey key; TValue value;

    for( int i=0; i<size; ++i ) {
        in >> tobits(key) >> tobits(value);
        p()[key] = value;
    }

    return in;
}

///-- files
    //TODO review this when implementing tiny streams

template <class T>
void Load( const char *filename ,T &m ) {
    std::ifstream in( filename );

    in >> m;
}

template <class T>
void Save( const char *filename ,const T &m ) {
    std::ofstream out( filename ,std::ios::binary );

    out << m;
}

//--
template <class T>
void LoadBinary( const char *filename ,T &m ) {
    std::ifstream in( filename );

    in >> tobits(m);
}

template <class T>
void SaveBinary( const char *filename ,const T &m ) {
    std::ofstream out( filename ,std::ios::binary );

    out << tobits(m);
}

//////////////////////////////////////////////////////////////////////////////
//! List

template <typename T ,typename TKey>
bool addUnique( ListOf<T> &list ,TKey &&key ) {
    for( const auto &it : list ) {
        if( it == key ) return false;
    }

    list.emplace_back( key );

    return true;
}

///-- manifest
template <typename T>
ListOf<T> &fromManifest( ListOf<T> &p ,const StringList &manifest ) {
    T x;

    for( auto &it : manifest ) {
        fromString( x ,it );

        p.emplace_back( x );
    }

    return p;
}

template <typename T>
StringList &toManifest( const ListOf<T> &p ,StringList &manifest ) {
    String s;

    for( auto &it : p ) {
        s.clear();

        toString( it ,s );

        manifest.emplace_back( s );
    }

    return manifest;
}

///-- string
template <typename T>
ListOf<T> &fromString( ListOf<T> &p ,const String &s ,size_t &size ) {
    StringList list;

    fromString( list ,s ,size );

    fromManifest( p ,list );

    return p;
}

template <typename T>
ListOf<T> &fromString( ListOf<T> &p ,const String &s ) {
    size_t size; return fromString( p ,s ,size );
}

template <typename T>
String &toString( const ListOf<T> &p ,String &s ) {
    StringList list;

    toManifest( p ,list );

    toString( list ,s );

    return s;
}

///-- serialization

//////////////////////////////////////////////////////////////////////////////
//! Map

//TODO get serialization from above ... other tools and helpers
//...

template <class TMap ,typename TKey ,typename TValue>
bool findMap( const TMap &map ,const TKey &key ,TValue &value ) {
    const auto &it = map.find( key );

    if( it == map.end() ) return false;

    value = it->second;

    return true;
}

template <class TMap ,typename TKey ,typename TValue>
bool digMap( const TMap &map ,const TValue &value ,TKey &key ) {
    for( const auto &it : map ) {
        if( it.second == value ) {
            key = it.first;
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
//! Arguments

    //TODO elsewhere

//! @brief white space separated list // no ending // quoted text // no block

struct Arguments {
    String &operator [](int i ) { return values[i]; }

    NoDiscard size_t count() const { return values.size(); }

    ListOf<String> values;
};

template <>
Arguments &fromString( Arguments &p ,const String &s ,size_t &size );

template <>
String &toString( const Arguments &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_BASE_H