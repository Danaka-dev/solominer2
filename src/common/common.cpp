// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "common.h"

#include <fstream>


//////////////////////////////////////////////////////////////////////////////
String getOracle( const char *key ) {
    using namespace  solominer;

    Params &params = getOracle();

    auto it = params.find( key );

    if( it == params.end() ) return {};

    return it->second;
}

void setOracle( const char *key ,const char *value ) {
    using namespace  solominer;

    Params &params = getOracle();

    params[key] = value;
}

void setOracle( const char *key ,double value ) {
    using namespace  solominer;

    String s;

    toString( value ,s );

    setOracle( key ,s.c_str() );
}

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
Params &getOracle() {
    return Oracle::getInstance().params;
}

//////////////////////////////////////////////////////////////////////////////
template <> UrlConnection &fromString( UrlConnection &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str(); size_t n = 0;

    //TODO proper URL parsing

    size = ParseField( str ,':' ,&p.host ,false );
    if( *str ) fromString( p.port ,String(str) ,n ) ,size += n;

    return p;
}

template <> String &toString( const UrlConnection &p ,String &s ) {
    if( p.port > 0 ) {
        Format( s ,"%s:%d" ,p.host.size()+1+8+1 ,p.host.c_str() ,(int) p.port );
    } else {
        s = p.host;
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
template <> Credential &fromString( Credential &p ,const String &s ,size_t &size ) {
    const char *str = s.c_str(); size_t n = 0;

    size = ParseField( str ,':' ,&p.user ,false );
    if( *str ) p.password = str ,size += strlen(str);

    return p;
}

template <> String &toString( const Credential &p ,String &s ) {
    if( !p.password.empty() ) {
        size_t maxsize = p.user.size()+1+p.password.size()+1;
        Format( s ,"%:%s" ,maxsize ,p.user.c_str() ,p.password.c_str() );
    } else {
        s = p.user;
    }

    return s;
}

//////////////////////////////////////////////////////////////////////////////
//! tools (string)

bool getUncommentedLine( std::istream &is ,std::string &s ,char delim ,char tag ) {
    if( !std::getline( is ,s ,delim ))
        return false;

    std::stringstream ss( s );

    std::getline( ss ,s ,tag );

    trim( s );

    return true;
}

bool getSectionHeader( const String &s ,String &section ) {
    if( s.empty() || s[0] != '[' )
        return false;

    std::stringstream ss( s );

    ss.seekg( 1 ,StringStream::cur );
    std::getline( ss ,section ,']' );

    return true;
}

bool isSectionHeader( std::string &s ,const char *section ) {
    if( s.empty() || s[0] != '[' )
        return false;

    std::stringstream ss( s );

    std::string s_section;

    std::getline( ss ,s_section ,'[' );
    std::getline( ss ,s_section ,']' );

    return toupper( s_section ) == section;
}

///--
bool getConfigSection( const char *filename ,const char *section ,String &content ) {
    using namespace std;

    ifstream f( filename );

    if( !f.is_open() )
        return false;

    string s;

    bool result = true;

    while( getUncommentedLine( f ,s ) ) {
        if( s.empty() ) continue;

        if( isSectionHeader( s ,section ) ) {
            StringStream ss;

            while( getUncommentedLine( f ,s ) ) {
                ss << s << '\n';
            }

            content = ss.str();

            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF