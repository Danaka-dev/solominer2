// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2023-2024 The Solominer developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "base64.h"

#include <cstring>

//////////////////////////////////////////////////////////////////////////////
//! Encode

template<int frombits, int tobits, bool pad, typename O, typename I>
bool ConvertBits( const O &outfn ,I it ,I end ) {
    size_t acc = 0;
    size_t bits = 0;

    constexpr size_t maxv = (1 << tobits) - 1;
    constexpr size_t max_acc = (1 << (frombits + tobits - 1)) - 1;

    while (it != end) {
        acc = ((acc << frombits) | *it) & max_acc;
        bits += frombits;
        while (bits >= tobits) {
            bits -= tobits;
            outfn((acc >> bits) & maxv);
        }
        ++it;
    }

    if (pad) {
        if (bits) outfn((acc << (tobits - bits)) & maxv);
    } else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
        return false;
    }

    return true;
}

std::string EncodeBase64( const unsigned char *pch ,size_t len ) {
    static const char *pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string str;
    str.reserve(((len + 2) / 3) * 4);
    ConvertBits<8, 6, true>([&](int v) { str += pbase64[v]; }, pch, pch + len);
    while (str.size() % 4) str += '=';
    return str;
}

std::string EncodeBase64( const std::vector<unsigned char> &vch ){
    return EncodeBase64( vch.data() ,vch.size() );
}

std::string EncodeBase64( const std::string &str ) {
    return EncodeBase64( (const unsigned char*) str.c_str() ,str.size() );
}

//////////////////////////////////////////////////////////////////////////////
//! Decode

std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid)
{
    static const int decode64_table[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
        -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };

    const char* e = p;
    std::vector<unsigned char> val;
    val.reserve(strlen(p));

    while (*p != 0) {
        int x = decode64_table[(unsigned char)*p];
        if (x == -1) break;
        val.push_back(x);
        ++p;
    }

    std::vector<unsigned char> ret;
    ret.reserve((val.size() * 3) / 4);
    bool valid = ConvertBits<6, 8, false>([&](unsigned char c) { ret.push_back(c); }, val.begin(), val.end());

    const char* q = p;
    while (valid && *p != 0) {
        if (*p != '=') {
            valid = false;
            break;
        }
        ++p;
    }
    valid = valid && (p - e) % 4 == 0 && p - q < 4;
    if (pfInvalid) *pfInvalid = !valid;

    return ret;
}

std::string DecodeBase64( const std::string& str )
{
    std::vector<unsigned char> vchRet = DecodeBase64( str.c_str() );
    return std::string( (const char*) vchRet.data() ,vchRet.size() );
}

//////////////////////////////////////////////////////////////////////////////
//! Basic Auth

void makeBasicAuth( std::string &auth ,const std::string &user ,const std::string &pass ) {
    std::string userColonPass;

    userColonPass = user + ":" + pass;

    auth = std::string("Basic ") + EncodeBase64(userColonPass); // .c_str();
}

//////////////////////////////////////////////////////////////////////////////
//EOF