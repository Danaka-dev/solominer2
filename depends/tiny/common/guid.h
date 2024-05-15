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

//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_XAPP_GUID_H
#define TINY_XAPP_GUID_H

//////////////////////////////////////////////////////////////////////////////
#define GUID_DEFAULT_LOWCASE true

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
typedef unsigned char guid_t[16];

//////////////////////////////////////////////////////////////////////////////
struct Guid {
    guid_t data;

//--
    Guid() { Zero(); }
    Guid( const Guid &other ) DEFAULT;
    Guid( Guid &&other ) DEFAULT;

    void Zero();
    void Make();

    Guid &operator =( const Guid &other ) = default;
    Guid &operator =( Guid &&other ) = default;

    bool operator ==( const Guid &other ) const;
    bool operator !=( const Guid &other ) const;

    bool operator < ( const Guid &other ) const;

    Guid &fromString( const String &s ,size_t &size );
    Guid &fromString( const String &s );

    String &toString( String &s ) const;
    String toString() const;
};

//////////////////////////////////////////////////////////////////////////////
template <> guid_t &Zero( guid_t &a );
template <> guid_t &Copy( guid_t &a ,const guid_t &b );
template <> int Compare( const guid_t &a ,const guid_t &b );
template <> inline bool Equals( const guid_t &a ,const guid_t &b ) {
    return Compare(a,b) == 0;
}

guid_t &Make( guid_t &a );

///--
template <>
guid_t &fromString( guid_t &p ,const String &s ,size_t &size );

template <>
String &toString( const guid_t &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_XAPP_GUID_H