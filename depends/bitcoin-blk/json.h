//! Copyright (c) 2023-2024 Danaka developers
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_BLK_JSON
#define BITCOIN_BLK_JSON

//////////////////////////////////////////////////////////////////////////////
//! @note simply adapted this header file to use json library other than rapidjson

#include <rapidjson/document.h>

//////////////////////////////////////////////////////////////////////////////
namespace bitcoin_blk {

//////////////////////////////////////////////////////////////////////////////
namespace json {

//////////////////////////////////////////////////////////////////////////////
using namespace rapidjson;

static bool isEmpty( const Value &value ) {
    return !value.IsObject() || value.ObjectEmpty();
}

///--
template <typename T>
bool isType_( const Value &i );

template <> inline bool isType_<bool>( const Value &i ) { return i.IsBool(); }
template <> inline bool isType_<int>( const Value &i ) { return i.IsInt(); }
template <> inline bool isType_<unsigned int>( const Value &i ) { return i.IsUint(); }
template <> inline bool isType_<int64_t>( const Value &i ) { return i.IsInt64(); }
template <> inline bool isType_<uint64_t>( const Value &i ) { return i.IsUint64(); }
template <> inline bool isType_<double>( const Value &i ) { return i.IsDouble(); }
template <> inline bool isType_<const char*>( const Value &i ) { return i.IsString(); }

///--
template <typename T>
T getType_( const Value &value );

template <> inline bool getType_<bool>( const Value &i ) { return i.GetBool(); }
template <> inline int getType_<int>( const Value &i ) { return i.GetInt(); }
template <> inline unsigned int getType_<unsigned int>( const Value &i ) { return i.GetUint(); }
template <> inline int64_t getType_<int64_t>( const Value &i ) { return i.GetInt64(); }
template <> inline uint64_t getType_<uint64_t>( const Value &i ) { return i.GetUint64(); }
template <> inline double getType_<double>( const Value &i ) { return i.GetDouble(); }
template <> inline const char *getType_<const char*>( const Value &i ) { return i.GetString(); }

///--
template <typename T>
T getValue_( const Value &value ,const char *key ,T defaultValue ) {
    if( isEmpty(value) )
        return defaultValue;

    auto i = value.FindMember(key);

    if( i == value.MemberEnd() )
        return defaultValue;

    return isType_<T>(i->value) ? getType_<T>(i->value) : defaultValue;
}

inline bool getBool( const Value &value ,const char *key ,bool defaultValue=false ) {
    return getValue_<bool>( value ,key ,defaultValue );
}

static int getInt( const Value &value ,const char *key ,int defaultValue=0 ) {
    return getValue_<int>( value ,key ,defaultValue );
}

inline unsigned int getUint( const Value &value ,const char *key ,unsigned int defaultValue=0 ) {
    return getValue_<unsigned int>( value ,key ,defaultValue );
}

inline int64_t getInt64( const Value &value ,const char *key ,int64_t defaultValue=0 ) {
    return getValue_<int64_t>( value ,key ,defaultValue );
}

inline uint64_t getUint64( const Value &value ,const char *key ,uint64_t defaultValue=0 ) {
    return getValue_<uint64_t>( value ,key ,defaultValue );
}

inline double getDouble( const Value &value ,const char *key ,double defaultValue=0 ) {
    return getValue_<double>( value ,key ,defaultValue );
}

inline const char *getString( const Value &value ,const char *key ,const char *defaultValue="" ) {
    return getValue_<const char*>( value ,key ,defaultValue );
}

///--
static const rapidjson::Value nullValue;

inline const rapidjson::Value &getArray( const rapidjson::Value &value ,const char *key ) {
    if( isEmpty(value) )
        return nullValue;

    auto i = value.FindMember(key);

    if( i == value.MemberEnd() )
        return nullValue;

    return i->value.IsArray() ? i->value : nullValue;
}

inline const rapidjson::Value &getObject( const rapidjson::Value &value ,const char *key ) {
    if( isEmpty(value) )
        return nullValue;

    auto i = value.FindMember(key);

    if( i == value.MemberEnd() )
        return nullValue;

    return i->value.IsObject() ? i->value : nullValue;
}

//////////////////////////////////////////////////////////////////////////////
} //namespace json

//////////////////////////////////////////////////////////////////////////////
} //namespace bitcoin_blk

//////////////////////////////////////////////////////////////////////////////
#endif // BITCOIN_BLK_JSON