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

//////////////////////////////////////////////////////////////////////////////
TINY_NAMESPACE {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Text

//! @params 'a' : source text to be matched upon. Null value returns false
//! @params 'b' : text to match at source. Null value assimilated to "" and always true
int strmatch( const char *a ,const char *b ) {
    int r = 0; if( !a ) return -1;

    if( b ) while( *b && (r = (int) (*a - *b)) == 0 ) {
        ++a; ++b;
    }

    return r;
}

int strimatch( const char *a ,const char *b ) {
    int r = 0; if( !a ) return -1;

    if( b ) while( *b && (r = (int) (tolower(*a) - tolower(*b))) == 0 ) {
        ++a; ++b;
    }

    return r;
}

//////////////////////////////////////////////////////////////////////////////
//! Parsing

size_t ParseHeader( const char_t *&s ,const char_t *open ,const char_t *close ,String *header ,bool captureLimits ) {
    return Parse( s ,[open,close]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( i==0 ) {
                if( strmatch(s,open)!=0 ) return false;
                skip = strlen(open); return true;
            }

            if( strmatch(s,close)!=0 ) return true;

            skip = strlen(close); return false;
        }
        ,header ,captureLimits
    );
}

size_t ParseHeader( const char_t *&s ,char_t open ,char_t close ,String *header ,bool captureLimits ) {
    return Parse( s ,[open,close]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( i==0 ) {
                if( *s != open ) return false;
                skip = 1; return true;
            }
            if( *s != close ) return true;
            skip = 1; return false;
        }
        ,header ,captureLimits
    );
}

size_t ParseField( const char_t *&s ,cacceptA accept ,char_t limit ,String *field ,bool captureLimits ) {
    return Parse( s ,[accept,limit]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( accept(*s) ) return true;
            skip = 1; return false;
        }
        ,field ,captureLimits
    );
}

size_t ParseField( const char_t *&s ,const char_t *limit ,String *field ,bool captureLimits ) {
    return Parse( s ,[limit]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( strmatch( s ,limit ) != 0 ) return true;
            skip = strlen(limit); return false;
        }
        ,field ,captureLimits
    );
}

size_t ParseField( const char_t *&s ,char_t limit ,String *field ,bool captureLimits ) {
    return Parse( s ,[limit]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( *s != limit ) return true;
            skip = 1; return false;
        }
        ,field ,captureLimits
    );
}

size_t ParseLine( const char_t *&s ,String *line ,bool captureLimits ) {
    return Parse( s ,[]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( !ciseol(*s) ) return true;
            skip = 1 + (cisreturn(s[0]) && cisnewline(s[1]) ? 1 : 0); return false;
        }
        ,line ,captureLimits
    );
}

size_t ParseToken( const char_t *&s ,String *token ) {
    return Parse( s ,[]( size_t i ,const char *s ,size_t &skip ) -> bool {
            return ( (i == 0) ? cisalpha(*s) : cisalphanum(*s)) || *s == '_';
        }
        ,token ,false
    );
}

//! @note recursive
size_t ParseBlock( const char_t *&s ,const char_t *open ,const char_t *close ,String *block ,bool captureLimits ) {
    int n = 1;

    return Parse( s ,[&n,open,close]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( i == 0 ) {
                if( strcmp( s ,open ) != 0 ) return false;
                skip = strlen(open); return true;
            }
            if( strmatch(s,open) == 0 ) { ++n; return true; }
            if( strmatch(s,close) == 0 ) { skip=strlen(close); return --n != 0; }

            return true;
        }
        ,block ,captureLimits
    );
}

size_t ParseBlock( const char_t *&s ,char_t open ,char_t close ,String *block ,bool captureLimits ) {
    int n = 1;

    return Parse( s ,[&n,open,close]( size_t i ,const char *s ,size_t &skip ) -> bool {
            if( i == 0 ) {
                if( *s != open ) return false;
                skip=1; return true;
            }
            if( *s == open ) { ++n; return true; }
            if( *s == close ) { skip=1; return --n != 0; }

            return true;
        }
        ,block ,captureLimits
    );
}

//////////////////////////////////////////////////////////////////////////////
//! Comment and spaces

static const CommentDef *g_commentDef = &ccode_like_comment;

const CommentDef &setDefaultCommentStyle( CommentDef &def ) {
    return * (g_commentDef = &def);
}

const CommentDef &getDefaultCommentStyle() {
    return *g_commentDef;
}

//--
size_t NoComment( const char_t *&s ,const CommentDef &def ) {
    const char_t *s0 = s;

    while(
        ParseHeader( s ,def.open ,def.close )
        || ( strmatch( s ,def.line ) == 0 && ParseLine( s ) )
    ) {}

    return s - s0;
}

size_t NoSpace( const char_t *&s ) {
    return Parse( s ,[]( size_t i ,const char *s ,size_t &skip ) -> bool {
            return ciswspace(*s);
        }
    );
}

size_t NoCommentNorSpace( const char_t *&s ,const CommentDef &def ) {
    const char_t *s0 = s;

    NoSpace(s);

    while( NoComment(s,def) && NoSpace(s) ) {}

    return s - s0;
}

//--
void Uncomment( const char_t *s ,String &text ,const CommentDef &def ) {
    text.resize( strlen(s) );

    const char *s1 = s; //! read
    char *s2 = (char *) text.c_str(); //! write

    while( *s1 ) {
        NoComment( s1 ,def ); *s2 = *s1; ++s1; ++s2;
    }
}

void Unformat( const char_t *s ,String &text ) {
    text.resize( strlen(s) );

    const char *s1 = s; //! read
    char *s2 = (char *) text.c_str(); //! write

    while( *s1 ) {
        NoSpace( s1 ); *s2 = *s1; ++s1; ++s2;
    }
}

//--
void Uncomment( String &s ,const CommentDef &def ) {
    const char *str = s.c_str();
    const char *s1 = str; //! read
    char *s2 = (char *) str; //! write

    if( s1 ) while( *s1 ) {
        NoComment( s1 ,def );

        *s2 = *s1; ++s1; ++s2;
    }

    s.resize( s2 - str );
}

void Unformat( String &s ) {
    const char *str = s.c_str();
    const char *s1 = str; //! read
    char *s2 = (char *) str; //! write

    while( *s1 ) {
        NoSpace( s1 );

        *s2 = *s1; ++s1; ++s2;
    }

    s.resize( s2 - str );
}

//////////////////////////////////////////////////////////////////////////////
//! Replacement

void replaceChar( String &s ,char a ,char b ) {
    for( auto &it : s ) {
        if( it == a ) it = b;
    }
}

void replaceTextVariables( const char *s ,const Params &vars ,String &r ) {
    const char *s0 = s ,*s1 = s0;

    String token;

    while( (s1 = strchr( s0 ,'$' )) != NullPtr ) {
        r += String( s0 ,s1 ); s0 = s1;

        ++s1;

        if( ParseHeader( s1 ,'{' ,'}' ,&token ,false ) || ParseToken( s1 ,&token ) ) {
            const auto &it = vars.find( token );

            if( it != vars.end() )
                r += it->second;
        }

        if( s1 == s0+1 ) { //! nothing found
            r += '$';
        }

        s0 = s1;
    }

    //-- trailer
    if( *s0 ) {
        r += s0;
    }
}

//////////////////////////////////////////////////////////////////////////////
//! Print

void AddIndent( String &s ,int n ) {
    for( int i=0; i<n; ++i ) s += "\t";
}

void prettyPrintBlock( const char *s ,String &r ) {
    r.clear();

    if( s && *s ) {} else return;

    const char *p = s;
    char s1[2] = {"x"};

    int indent = 0;
    bool begin=true; //! no space at begin of line

    while( *p ) {
        switch( *p ) {
            case '=': if( !begin && *(p-1) != ' ' ) r += ' '; r += "="; if( *(p+1) != ' ' ) r += " "; break;
            case '{': ++indent; r += "{\n";  begin=true; break;
            case '}': --indent; begin=true; r += "}\n"; if( indent==0) r += "\n"; break;
            case ';': r += ";\n"; begin=true; break;
            case ' ': if( !begin ) r += " ";

            case '\t': case '\r': case '\n': break;
            default:
                if( begin ) AddIndent(r,indent);
                s1[0] = *p; r += s1; begin=false;
                break;
        }

        ++p;
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
} //TINY_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//EOF