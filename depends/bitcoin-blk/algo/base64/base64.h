#pragma once

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2023-2024 The Solominer developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BASE64_H
#define BITCOIN_BASE64_H

//////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

/**
 * Encode a byte sequence as a base64-encoded string.
 */

std::string EncodeBase64( const unsigned char *pch ,size_t len );

std::string EncodeBase64( const std::vector<unsigned char> &vch );

std::string EncodeBase64( const std::string& str );

//////////////////////////////////////////////////////////////////////////////

/**
 * Decode a base64-encoded string into a byte vector
 */

std::vector<unsigned char> DecodeBase64( const char *p ,bool *pfInvalid=nullptr );

std::string DecodeBase64( const std::string &str );

//////////////////////////////////////////////////////////////////////////////

/**
 * Encode HTTP basic access authentication header field from Base64(username:password)
 * @param auth  [out] result basic access field
 * @param user  [in] username
 * @param pass  [in] password
 */

void makeBasicAuth( std::string &auth ,const std::string &user ,const std::string &pass );

//////////////////////////////////////////////////////////////////////////////
#endif // BITCOIN_BASE64_H