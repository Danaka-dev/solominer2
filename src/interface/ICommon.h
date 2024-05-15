#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_ICOMMON_H
#define SOLOMINER_ICOMMON_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! AssetId

/**
 * @brief an asset identity (fiat, stock, coin, token ...)
 * "<value_id> : <market/chain>"
 * value_id = ticker
 * market/chain = market or chain id
 * @exemple
 * AMZN:nasdaq
 * USDT:polygon
 */

struct AssetId {
    String name;
    String space;
};

template<>
String &fromManifest( String &p ,const AssetId &s );

//+ to manifest

//////////////////////////////////////////////////////////////////////////////
struct AmountValue {
    double amount; //TODO ~BigInt/BigFloat value, avoid conversion issues
    String value; //! @note can be an AssetId
};

inline bool hasAmount( const AmountValue &amount ) {
    return amount.amount > 0. && !amount.value.empty();
}

//--
template <> inline AmountValue &Zero( AmountValue &p ) {
    p = { 0. ,"" }; return p;
}

template <> inline bool Equals( const AmountValue &a ,const AmountValue &b ) {
    return Equals(a.amount,b.amount) && a.value == b.value;
}

//--
INLINE_FROMSTRING(AmountValue) {
    size_t sz;

    fromString( p.amount ,s ,sz ); size += sz;

    const char *str = s.c_str() + sz;
    const char *s0 = str;

    ParseToken( str ,&p.value );

    size += str - s0;

    return p;
}

INLINE_TOSTRING(AmountValue) {
    Format( s ,"%lf%s" ,64 ,(double) p.amount ,(const char*) p.value.c_str() );

    return s;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_ICOMMON_H