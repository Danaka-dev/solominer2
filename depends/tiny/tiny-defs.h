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

#ifndef TINY_DEFS_H
#define TINY_DEFS_H

//////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <memory>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Type mapping

    /// #define TINY_NO_TYPES //! tiny should not use tiny types
    /// #define TINY_USE_TYPES //! tiny should use tiny types
    /// #define TINY_USE_STD //! tiny should use std types

#ifdef TINY_USE_TYPES
 #define TINY_TYPES_TINY
#else
 #define TINY_TYPES_STD
#endif

//////////////////////////////////////////////////////////////////////////////
//!TODO use proper naming and mapping (vector, list ,tape .. arrray etc)

///-- using tiny types
#ifdef TINY_TYPES_TINY
 #include "tiny-types.h"

///-- templates
 #define ListOf  tiny::array
 #define MapOf   tiny::map

//////////////////////////////////////////////////////////////////////////////
#else ///-- using std types
 #include <cstring>
 #include <sstream>
 #include <string>
 #include <vector>
 #include <map>

///-- templates
 #define PtrOf   std::shared_ptr //TODO  remove
 #define ListOf  std::vector
 #define MapOf   std::map

///-- types
typedef std::string String;
typedef std::stringstream StringStream;
typedef std::iostream Stream;
typedef std::istream InStream;
typedef std::ostream OutStream;

//-- helpers
//TODO tocstr

std::string &toupper( std::string &s );
std::string &tolower( std::string &s );

std::string &ltrim( std::string &s );
std::string &rtrim( std::string &s );

inline void trim( String &s ) {
    ltrim(rtrim(s));
}

//////////////////////////////////////////////////////////////////////////////
#endif

//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_NAMESPACE_NAME
 #define TINY_NAMESPACE_NAME tiny
#endif

#define TINY_NAMESPACE namespace TINY_NAMESPACE_NAME

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Basics

///-- c++ versions

#if __cplusplus < CXX_11
 #define NullPtr    NULL
 #define NoExcept
#else
 #define DEFAULT {}
 #define NullPtr    nullptr
 #define NoExcept   noexcept
#endif

#if __cplusplus < CXX_17
 #define DEFAULT = default;
 #define NoDiscard
#else
 #define NoDiscard [[nodiscard]]
#endif

///-- c++ helper macros
#define SAFEDELETE( __p_ )          {if(__p_!=NullPtr) { delete __p_; __p_=NullPtr; }}
#define SAFEDELETEARRAY( __p_ )     {if(__p_!=NullPtr) { delete [] __p_; __p_=NullPtr; }}
#define SAFERELEASE( __p_ )         {if(__p_!=NullPtr) { __p_->Release(); __p_=NullPtr; }}

#define _TODO        assert(1==0)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//! Common definitions & helpers

#define INT64_LOPART(__x)       (__x & 0x0fffffff)
#define INT64_HIPART(__x)       (__x >> 32)
#define INT64_MAKE(__x,__y)     ((__x << 32) | __y)

#define __thread		__declspec(thread)

// enum AccessMode { modeUnknown=0 ,modeRead=1 ,modeWrite=2 ,modeReadWrite=3 ,modeCreate=4 ,modeNew=8 };
    //! create : if doesn't exist
    //! new : create always

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! API

typedef int32_t iresult_t;

#define IRESULT iresult_t

//--
#define IAPI virtual iresult_t

#if __cplusplus <= CXX_11
 #define API_DECL(__x)  virtual __x
 #define API_IMPL(__x)  virtual __x
 #define IOVERRIDE

#else
 #define API_DECL( __x )  virtual __x
 #define API_IMPL( __x )  __x
 #define IOVERRIDE  override

#endif

#define IAPI_DECL  API_DECL(iresult_t)
#define IAPI_IMPL  API_IMPL(iresult_t)
#define IAPI_DEF   iresult_t

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Results

#define ISUCCESS( __iresult__ ) \
    ((__iresult__) >= IOK)

#define IF_ISUCCESS( __iresult__ ) \
    if ISUCCESS(__iresult__)

#define IF_ISUCCESS_RETURN( __iresult__ ) \
    IF_ISUCCESS(__iresult__) return __iresult__;

///--
#define IFAILED( __iresult__ ) \
    ((__iresult__) < IOK)

#define IF_IFAILED( __iresult__ ) \
    if IFAILED(__iresult__)

#define IF_IFAILED_RETURN( __result__ ) \
    IF_IFAILED(__result__) return __result__;

///--
#define IOK         ((iresult_t)0)

///-- errors --///
// (operation was not carried out)
#define IERROR      ((iresult_t)-1)     //! generic failure result core
#define INOEXEC     ((iresult_t)-2)     //! no function available to carry requested operation
#define IBADARGS    ((iresult_t)-3)     //! bad arguments passed to function
#define IBADENV     ((iresult_t)-4)     //! bad environment found
#define IBADDATA    ((iresult_t)-5)     //! bad data found
#define IREFUSED    ((iresult_t)-6)     //! no permission to carry requested operation
#define IALREADY    ((iresult_t)-7)     //! unique operation already performed
#define IEXIST      ((iresult_t)-8)     //! operation target already exist
#define INOEXIST    ((iresult_t)-9)     //! required operation component(s) missing
#define IPROGRESS   ((iresult_t)-10)    //! required operation already in progress
//...
#define IFATAL      ((iresult_t)-255)   //! fatal error, program should abort

// (operation carried out with error)
#define INODATA     ((iresult_t)-256)     //! no data found when executing the request
//! more error here (<0)
//...

///-- status --///
//! (operation carried out with specific status)
#define IAGAIN      ((iresult_t) 1)     //! operation can be called again
#define ICONTINUE   ((iresult_t) 2)     //! operation should be called again
#define IPARTIAL    ((iresult_t) 3)     //! operation was completed but partially (e.g. some data were missing)
#define INOTHING    ((iresult_t) 4)     //! operation completed, but there was nothing todo
#define IWARNING    ((iresult_t) 255)   //! warning raised (operation was completed)

//! more status here (>0)
//...

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Exception

class Exception {
public:
    Exception( iresult_t id=IERROR ,const char *msg=NullPtr ) :
        m_result(id) ,m_message(msg)
    {}

    iresult_t result() const { return m_result; }

    const char *message() const { return m_message; }

protected:
    iresult_t m_result;
    const char *m_message;
};

//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
#endif //TINY_DEFS_H