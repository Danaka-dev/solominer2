#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_COMMON_H
#define SOLOMINER_COMMON_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

#include <cstring>

#include <algorithm>
#include <memory>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <sstream>
#include <string>
#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////////
//! @note in global namespace, we want access from any part of the program

String getOracle( const char *key );

void setOracle( const char *key ,const char *value );
void setOracle( const char *key ,double value );

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//TODO out

inline const char *with_size( const char *name ,size_t &size ) {
    size = strlen(name); return name;
}

template <typename T> //! from
inline std::stringstream &operator >>( std::stringstream &ss ,T &p ) {
    std::stringstream::pos_type pos = ss.tellg();

    String s; size_t size;

    std::getline( ss ,s );
    fromString( p ,s ,size );
    ss.seekg( pos + size );

    return ss;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! Structs

//////////////////////////////////////////////////////////////////////////////
struct UrlConnection {
    String host;
    int port;
};

template <> inline UrlConnection &Zero( UrlConnection &p ) {
    p.host = ""; p.port = 0; return p;
}

template <> inline UrlConnection &Init( UrlConnection &p ) {
    p.host = "127.0.0.1"; p.port = 0; return p;
}

template <> UrlConnection &fromString( UrlConnection &p ,const String &s ,size_t &size );
template <> String &toString( const UrlConnection &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
struct Credential {
    String user;
    String password;
};

template <> inline Credential &Zero( Credential &p ) {
    p.user = ""; p.password = ""; return p;
}

template <> Credential &fromString( Credential &p ,const String &s ,size_t &size );
template <> String &toString( const Credential &p ,String &s );

//////////////////////////////////////////////////////////////////////////////
struct Oracle : Singleton_<Oracle> {
    Oracle() {
        String s ,si;

        // Load( "oracle.dat" ,s ); //TODO this only loads string up to ' '

        std::ifstream in( "oracle.dat" );

        while( in >> si ) {
            //in >> si;
            s += si; s += " ";
        }

        fromString( params ,s );
    }

    ~Oracle() {
        String s;

        toString( params ,s );

        Save( "oracle.dat" ,s );
    }

    Params params;
};

Params &getOracle();

//////////////////////////////////////////////////////////////////////////////
//! string functions

//TODO out

///-- streams
bool getUncommentedLine( std::istream &is ,std::string &s ,char delim='\n' ,char tag='#' );

bool getSectionHeader( const String &s ,String &section );
bool isSectionHeader( std::string &s ,const char *section );

///-- files
bool getConfigSection( const char *filename ,const char *section ,String &content );

//////////////////////////////////////////////////////////////////////////////
} // namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_COMMON_H