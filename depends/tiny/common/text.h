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

#ifndef TINY_XAPP_TEXT_H
#define TINY_XAPP_TEXT_H

//////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

    //TODO import function from 'tiny-string.h' here

//////////////////////////////////////////////////////////////////////////
//! Text

typedef bool (*cacceptA)( const char c );
typedef bool (*cacceptW)( const wchar_t c);

//--
inline bool cisdigit( char c ) { return( c>='0' && c<='9' ); }
inline bool cisdigit( wchar_t c ) { return( (c)>=L'0' && (c)<=L'9' ); }
inline bool cishexdigit( char c ) { return( cisdigit(c) || ((c)>='a' && (c)<='f' ) || ((c)>='A' && (c)<='F' ) ); }
inline bool cishexdigit( wchar_t c ) { return( cisdigit(c) || ((c)>=L'a' && (c)<=L'f' ) || ((c)>=L'A' && (c)<=L'F' ) ); }
inline bool cislcase( char c )  { return( (c)>='a' && (c)<='z' ); }
inline bool cislcase( wchar_t c )  { return( (c)>=L'a' && (c)<=L'z' ); }
inline bool cisucase( char c )  { return( (c)>='A' && (c)<='Z' ); }
inline bool cisucase( wchar_t c )  { return( (c)>=L'A' && (c)<=L'Z' ); }
inline bool cisalpha( char c ) { return( cislcase(c) || cisucase(c) ); }
inline bool cisalpha( wchar_t c ) { return( cislcase(c) || cisucase(c) ); }
inline bool cisalphanum( char c ) { return( cisalpha(c) || cisdigit(c) ); }
inline bool cisalphanum( wchar_t c ) { return( cisalpha(c) || cisdigit(c) ); }
inline bool cissign( char c ) { return( c=='+' || c=='-'); }
inline bool cissign( wchar_t c ) { return( c==L'+' || c==L'-'); }
inline bool cisnumsep( char c ) { return( c==',' || c=='.'); }
inline bool cisnumsep( wchar_t c ) { return( c==L',' || c==L'.'); }
inline bool cisnum( char c ) { return( cisdigit(c) || cisnumsep(c) ); }
inline bool cisnum( wchar_t c ) { return( cisdigit(c) || cisnumsep(c) ); }

inline bool cisspace( char c ) { return( (c)==' ' ); }
inline bool cisspace( wchar_t c ) { return( (c)==L' ' ); }
inline bool cistab( char c ) { return( (c)=='\t' ); }
inline bool cistab( wchar_t c ) { return( (c)==L'\t' ); }
inline bool cisnewline( char c ) { return( (c)=='\n' ); }
inline bool cisnewline( wchar_t c ) { return( (c)==L'\n' ); }
inline bool cisreturn( char c ) { return( (c)=='\r' ); }
inline bool cisreturn( wchar_t c ) { return( (c)==L'\r' ); }

inline bool cissep( char c ) { return( cisspace(c) || cistab(c) ); }
inline bool cissep( wchar_t c ) { return( cisspace(c) || cistab(c) ); }
inline bool ciseol( char c ) { return( cisreturn(c) || cisnewline(c) ); }
inline bool ciseol( wchar_t c ) { return( cisreturn(c) || cisnewline(c) ); }
inline bool ciswspace( char c ) { return( cissep(c) || ciseol(c) ); } //! whitesapce
inline bool ciswspace( wchar_t c ) { return( cissep(c) || ciseol(c) ); }

//////////////////////////////////////////////////////////////////////////
inline int cnumvalue( const char c ) { return (c<0x30) ? -1 : (c<0x3A) ? (c-0x30) : -1; }
inline int chexvalue( const char c ) { return (c<0x30) ? -1 : (c<0x3A) ? (c-0x30) : (c<0x41) ? -1 : (c<0x47) ? (c-0x41+10) : (c<0x61) ? -1 : (c<0x67) ? (c-0x61+10) : -1; }
//! return value of hexadecimal digit (0..9,A,B,C,D,E,F -> 0..15)

//////////////////////////////////////////////////////////////////////////////
int strmatch( const char *a ,const char *b );
int strimatch( const char *a ,const char *b );

inline bool Match( const char *s ,const char *b ) { return strmatch( s ,b ) == 0; }
inline bool iMatch( const char *s ,const char *b ) { return strimatch( s ,b ) == 0; }

inline bool Match( const String &s ,const char *b ) { return Match( s.c_str() ,b ); }
inline bool iMatch( const String &s ,const char *b ) { return iMatch( s.c_str() ,b ); }

//////////////////////////////////////////////////////////////////////////////
//! Skip //MAYBE name this TestSymbol

#define OPTIONAL || true

inline bool SkipSymbol( const char *&s ,char c ) {
    if( *s != c ) return false; ++s; return true;
}

bool SkipToken( const char *&s ,const char *b );

//TODO + Hex encodign

//////////////////////////////////////////////////////////////////////////////
//! Parse

size_t ParseHeader( const char_t *&s ,const char_t *open ,const char_t *close ,String *header=NullPtr ,bool captureLimits=true );
size_t ParseHeader( const char_t *&s ,char_t open ,char_t close ,String *header=NullPtr ,bool captureLimits=true );

size_t ParseField( const char_t *&s ,cacceptA accept ,char_t limit ,String *field=NullPtr ,bool captureLimits=true );
size_t ParseField( const char_t *&s ,const char_t *limit ,String *field=NullPtr ,bool captureLimits=true );
size_t ParseField( const char_t *&s ,char_t limit ,String *field=NullPtr ,bool captureLimits=true );

size_t ParseLine( const char_t *&s ,String *line=NullPtr ,bool captureLimits=false );
size_t ParseToken( const char_t *&s ,String *token=NullPtr );

//! @note quoted
//TODO

//! @note recursive
size_t ParseBlock( const char_t *&s ,const char_t *open ,const char_t *close ,String *block=NullPtr ,bool captureLimits=true );
size_t ParseBlock( const char_t *&s ,char_t open ,char_t close ,String *block=NullPtr ,bool captureLimits=true );

//////////////////////////////////////////////////////////////////////////////
//! Comment and spaces

struct CommentDef {
    const char *line ,*open ,*close;
};

static const CommentDef ccode_like_comment = { _T("//") ,_T("/*") ,_T("*/") };
static const CommentDef cmake_like_comment = { _T("#") ,_T("#[[") ,_T("]]") };

const CommentDef &setDefaultCommentStyle( CommentDef &def );
const CommentDef &getDefaultCommentStyle();

size_t NoComment( const char_t *&s ,const CommentDef &def=getDefaultCommentStyle() );
size_t NoSpace( const char_t *&s ); //! @note any white space, including eol...
size_t NoCommentNorSpace( const char_t *&s ,const CommentDef &def=getDefaultCommentStyle() );

void Uncomment( const char_t *s ,String &text ,const CommentDef &def=getDefaultCommentStyle() );
void Unformat( const char_t *s ,String &text );
    //! @note removing formating -> space to single space, multiple line break to one line break

//! @note in-place
void Uncomment( String &s ,const CommentDef &def=getDefaultCommentStyle() );
//? Unquote here ?
void Unformat( String &s );

//////////////////////////////////////////////////////////////////////////
//! Replacement

inline String &Emblock( String &s ) {
    String block = "{"; block += s; block += "}"; return s = block;
}

inline String &Enquote( String &s ) {
    _TODO; //TODO replace all " in s to \"
    String block = "\""; block += s; block += "\""; return s = block;
}

//--
void unquoteText();

//--
void replaceChar( String &s ,char a ,char b );

//! @note replacing variables from vars in string s to string r
void replaceTextVariables( const char *s ,const Params &vars ,String &r );

//////////////////////////////////////////////////////////////////////////
//! Print

void prettyPrintBlock( const char *s ,String &r );

//////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////
#endif //TINY_XAPP_TEXT_H