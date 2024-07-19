/*
 * CryptH - Simple symmetric encryption/decryption
 *
 * Copyright 2012-2024 M. Ganapati <m.ganapati@proton.me>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//////////////////////////////////////////////////////////////////////////////
#include "crypth.h"

// c++
#include <cstring>
#include <cassert>

//////////////////////////////////////////////////////////////////////////////
#include <algo/base58/base58.h>
#include <algo/sha/sha256.h>

//////////////////////////////////////////////////////////////////////////////
#define MIN(a,b) (((a)<(b))?(a):(b))

#define CRYPTH_BLOCK_SIZE 32
#define CRYPTH_ENCODE_SIZE 44 // base64 | base58 encode size for 32

//////////////////////////////////////////////////////////////////////////////
typedef unsigned char Byte;
typedef std::vector<Byte> Hex;
typedef std::string String;

//////////////////////////////////////////////////////////////////////////////
void XOR( Byte *out ,const Byte *data ,const Byte *mask ,size_t size ) {
    for( size_t i=0; i<size; ++i ) {
        out[i] = data[i] ^ mask[i];
    }
}

//TODO don't expose mask

bool CryptH( const char *password ,const char *text ,String &s ) {
    // x0=SHA256(password),x1=SHA256(x0),....
    // xor ToBin(text)

    assert( CRYPTH_BLOCK_SIZE == SHA256_OUTPUT_SIZE );

    const int N = CRYPTH_BLOCK_SIZE;

    //-- password
    String passh = CRYPTH_SALT;
    passh += password; passh += CRYPTH_PEPPER;

    Hex hex;
    const size_t np = passh.size();
    hex.resize( np );
    memcpy( hex.data() ,passh.c_str() ,np );

    //-- block
    unsigned char output[N];
    unsigned char block[N];
    unsigned char cypher[N];

    int n = (int) strlen(text)+1;

    s.clear();

    for( int i=0; i<n; i+=N ) {
        SHA256( output ,hex.data() ,hex.size() ); //! make mask

        memset( block ,0 ,N );
        memcpy( block ,text+i ,MIN(n-i,N) ); //! 0 padded block

        XOR( cypher ,block ,output ,N ); //! apply

        s += EncodeBase58( cypher ,cypher+N ); //! encode

        hex.resize( N );
        memcpy( hex.data() ,output ,N ); //! next mask base
    }

    return true;
}

bool DecryptH( const char *password ,const char *text ,String &s ) {
    const int N = CRYPTH_BLOCK_SIZE;
    const int N1 = CRYPTH_ENCODE_SIZE;

    //-- password
    String passh = CRYPTH_SALT;
    passh += password; passh += CRYPTH_PEPPER;

    Hex hex;
    const size_t np = passh.size();
    hex.resize( np );
    memcpy( hex.data() ,passh.c_str() ,np );

    //-- block
    unsigned char output[N];
    unsigned char cypher[N];
    char block[N1+1];
    Hex hb;

    int n = (int) strlen(text);

    s.clear();

    for( int i=0; i<n; i+=N1 ) {
        SHA256( output ,hex.data() ,hex.size() ); //! make mask

        memset( block ,0 ,N1+1 );
        memcpy( block ,text+i ,N1 );

        if( !DecodeBase58( block ,hb ) ) return false; //! decode

        XOR( cypher ,hb.data() ,output ,N ); //! apply

        s += String( (const char*) cypher ,MIN(strlen((const char*) cypher),N) );

        hex.resize( N );
        memcpy( hex.data() ,output ,N ); //! next mask base
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//EOF