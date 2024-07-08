#include "tiny.h"

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

//! ACKNOWLEDGEMENT guid make functions inspired from
//! Graeme Hill (2014) <http://graemehill.ca>
//!     https://github.com/graeme-hill/crossguid/blob/master/src/guid.cpp

//////////////////////////////////////////////////////////////////////////////
#include <cstring>

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
#ifndef byte
 typedef unsigned char byte;
#endif

const char hexsep = '-';

inline bool isSpaceChar( char ch ) {
    return ch==' ' || ch=='\t' || ch=='\r' || ch=='\n';
}

//////////////////////////////////////////////////////////////////////////////
//! from Hex text

byte hexToByte( char ch ) {
    // 0-9
    if( ch > 47 && ch < 58 ) return (byte) (ch - 48);

    // a-f
    if( ch > 96 && ch < 103 ) return (byte) (ch - 87);

    // A-F
    if( ch > 64 && ch < 71 ) return (byte) (ch - 55);

    return 0;
}

bool isValidHexChar( char ch ) {
    return ch == '0' || hexToByte(ch) != 0;
}

byte hexPairToByte( const char ch[2] ) {
    return ( hexToByte(ch[0]) << 4 ) + hexToByte(ch[1]);
}

//////////////////////////////////////////////////////////////////////////////
//! to Hex text

char byteToHex( byte bin ,bool lowcase=GUID_DEFAULT_LOWCASE ) {
    // 0-9
    if( bin < 10 ) return (char) (bin + 48);

    // a-f / A-F
    if( bin < 16 ) return (char) ( lowcase ? (bin + 87) : (bin + 55) );

    return '?';
}

char *byteToHexPair( char ch[2] ,byte bin ,bool lowcase=GUID_DEFAULT_LOWCASE ) {
    ch[0] = byteToHex( bin >> 4 ,lowcase );
    ch[1] = byteToHex( bin & 0x0f ,lowcase );

    return ch;
}

char *bytesToHexPairs( int n ,char ch[/* 2*n */] ,byte bin[/* n */] ,bool lowcase=GUID_DEFAULT_LOWCASE ) {
    for( int i=0; i<n; ++i )
        byteToHexPair( ch+2*i ,bin[i] ,lowcase );

    return ch;
}

//////////////////////////////////////////////////////////////////////////////
//! Guid

void Guid::Zero() {
    TINY_NAMESPACE_NAME::Zero( data );
}

void Guid::Make() {
    TINY_NAMESPACE_NAME::Make( data );
}

bool Guid::operator ==( const Guid &other ) const {
    return Compare( data ,other.data ) == 0;
}

bool Guid::operator!=(const Guid &other) const {
    return Compare( data ,other.data ) != 0;
}

bool Guid::operator <( const Guid &other ) const {
    return Compare( data ,other.data ) < 0;
}

Guid &Guid::fromString( const String &s ,size_t &size ) {
    TINY_NAMESPACE_NAME::fromString( data ,s ,size ); return *this;
}

Guid &Guid::fromString( const String &s ) {
    size_t size; fromString( s ,size ); return *this;
}

String &Guid::toString( String &s ) const {
    return TINY_NAMESPACE_NAME::toString( data ,s );
}

String Guid::toString() const {
    String s; toString(s); return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Common

template <>
guid_t &Zero( guid_t &a ) {
    memset( &a ,0 ,sizeof(guid_t) ); return a;
}

template <>
guid_t &Copy( guid_t &a ,const guid_t &b ) {
    memcpy( &a ,&b ,sizeof(guid_t) ); return a;
}

template <>
int Compare( const guid_t &a ,const guid_t &b ) {
    return memcmp( &a ,&b ,sizeof(guid_t) );
}

//--
template <>
guid_t &fromString( guid_t &p ,const String &s ,size_t &size ) {
    const char *ch = s.c_str(); assert(ch);
    byte *pp = p;

    while( isSpaceChar( *ch ) ) ++ch; //! skip initial space(s)

    while( ch[0] ) {
        if( pp - p > 16 ) break;

        if( *ch == hexsep ) {
            ++ch; //! separator
        } else if( isValidHexChar(ch[0]) && isValidHexChar(ch[1]) ) {
            *pp = hexPairToByte( ch ); ++pp; ch += 2; //! good pair
        } else {
            break; //! unknown, fail
        }
    }

    if( (pp - p) != 16 ) {
        TINY_NAMESPACE_NAME::Zero(p); //! wrong size/incomplete , zero
    } else {
        size = ch - s.c_str();
    }

    return p;
}

template <>
String &toString( const guid_t &p ,String &s ) {
    char ch[ 16*2 +4 +1 ]; // 16 double char + 4 sep + 0 term

    bytesToHexPairs( 4 ,ch+0 ,(byte*) p+0 );
    ch[8] = hexsep;
    bytesToHexPairs( 2 ,ch+9 ,(byte*) p+4 );
    ch[13] = hexsep;
    bytesToHexPairs( 2 ,ch+14 ,(byte*) p+6 );
    ch[18] = hexsep;
    bytesToHexPairs( 2 ,ch+19 ,(byte*) p+8 );
    ch[23] = hexsep;
    bytesToHexPairs( 6 ,ch+24 ,(byte*) p+10 );
    ch[36] = 0;

    s = ch; return s;
}

//////////////////////////////////////////////////////////////////////////////
//! Generate

#ifdef PLATFORM_LINUX
#include <uuid/uuid.h>

guid_t &Make( guid_t &a ) {
	static_assert( std::is_same<unsigned char[16],uuid_t>::value ,"incompatible uuid_t type detected" );

	uuid_generate( a );

    return a;
}

#endif

///-- Mac & ios
#ifdef PLATFORM_APPLE
#include <CoreFoundation/CFPUID.h>

Guid newGuid() {
	auto newId = CFPUIDCreate(NULL);
	auto bytes = CFPUIDGetPUIDBytes(newId);
	CFRelease(newId);

	std::array<unsigned char, 16> byteArray =
	{{
		bytes.byte0,
		bytes.byte1,
		bytes.byte2,
		bytes.byte3,
		bytes.byte4,
		bytes.byte5,
		bytes.byte6,
		bytes.byte7,
		bytes.byte8,
		bytes.byte9,
		bytes.byte10,
		bytes.byte11,
		bytes.byte12,
		bytes.byte13,
		bytes.byte14,
		bytes.byte15
	}};
	return Guid{std::move(byteArray)};
}
#endif

///-- Windows
#ifdef PLATFORM_WINDOWS
/*#include <objbase.h>
#include <array>
#include <algorithm>*/

guid_t& Make(guid_t& newId) {

	CoCreateGuid((GUID*) &newId);

	/*std::array<unsigned char, 16> bytes =
	{
		(unsigned char)((newId.Data1 >> 24) & 0xFF),
		(unsigned char)((newId.Data1 >> 16) & 0xFF),
		(unsigned char)((newId.Data1 >> 8) & 0xFF),
		(unsigned char)((newId.Data1) & 0xff),

		(unsigned char)((newId.Data2 >> 8) & 0xFF),
		(unsigned char)((newId.Data2) & 0xff),

		(unsigned char)((newId.Data3 >> 8) & 0xFF),
		(unsigned char)((newId.Data3) & 0xFF),

		(unsigned char)newId.Data4[0],
		(unsigned char)newId.Data4[1],
		(unsigned char)newId.Data4[2],
		(unsigned char)newId.Data4[3],
		(unsigned char)newId.Data4[4],
		(unsigned char)newId.Data4[5],
		(unsigned char)newId.Data4[6],
		(unsigned char)newId.Data4[7]
	};

	return Guid{std::move(bytes)};*/
	return newId;
}
#endif

///-- Android
#ifdef PLATFORM_ANDROID
#include <jni.h>
#include <cassert>

Guid newGuid(JNIEnv *env) {
	assert(env != androidInfo.env || std::this_thread::get_id() == androidInfo.initThreadId);

	jobject javaUuid = env->CallStaticObjectMethod(
		androidInfo.uuidClass, androidInfo.newGuidMethod);
	jlong mostSignificant = env->CallLongMethod(javaUuid,
		androidInfo.mostSignificantBitsMethod);
	jlong leastSignificant = env->CallLongMethod(javaUuid,
		androidInfo.leastSignificantBitsMethod);

	std::array<unsigned char, 16> bytes =
	{
		(unsigned char)((mostSignificant >> 56) & 0xFF),
		(unsigned char)((mostSignificant >> 48) & 0xFF),
		(unsigned char)((mostSignificant >> 40) & 0xFF),
		(unsigned char)((mostSignificant >> 32) & 0xFF),
		(unsigned char)((mostSignificant >> 24) & 0xFF),
		(unsigned char)((mostSignificant >> 16) & 0xFF),
		(unsigned char)((mostSignificant >> 8) & 0xFF),
		(unsigned char)((mostSignificant) & 0xFF),
		(unsigned char)((leastSignificant >> 56) & 0xFF),
		(unsigned char)((leastSignificant >> 48) & 0xFF),
		(unsigned char)((leastSignificant >> 40) & 0xFF),
		(unsigned char)((leastSignificant >> 32) & 0xFF),
		(unsigned char)((leastSignificant >> 24) & 0xFF),
		(unsigned char)((leastSignificant >> 16) & 0xFF),
		(unsigned char)((leastSignificant >> 8) & 0xFF),
		(unsigned char)((leastSignificant) & 0xFF)
	};

	env->DeleteLocalRef(javaUuid);

	return Guid{std::move(bytes)};
}

Guid newGuid() {
	return newGuid(androidInfo.env);
}
#endif

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF