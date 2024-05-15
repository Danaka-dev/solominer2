#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef SOLOMINER_SERVICE_H
#define SOLOMINER_SERVICE_H

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>
#include <common/base.h>

#include <interface/IService.h>

#include <fstream>
#include <ctime>

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
template <class T>
inline bool get_cache_field( std::istream &is ,T &field ) {
    std::string s;

    if( !std::getline( is ,s ,'\x0' ) )
        return false;

    field = s;

    return true;
}

template <class Ta ,class Tb>
class CServiceCache_ {
protected:
    struct Entry {
        Tb result;
        time_t timestamp; //+ validity
    };

    MapOf<Ta,Entry> m_cache; //! url -> entry

    time_t m_validity; //TODO per request

protected:
    String m_persist;
    bool m_cacheHit; //! last request hit cache
    bool m_cacheDirty; //! cache changed since last save (ie cache FILE is dirty)

public:
    CServiceCache_( time_t validity=1000 ,const char *cachefile="" ) :
        m_validity(validity) ,m_persist(cachefile) ,m_cacheHit(false) ,m_cacheDirty(false)
    {}

    // Tidy+Save on destructor

    bool cacheHit() { return m_cacheHit; }
    bool cacheDirty() { return m_cacheDirty; }

public:
    void Load( const char *cachefile="" ) {
        if( cachefile[0] ) m_persist = cachefile;
        if( m_persist.empty() ) return;

        std::ifstream fs;

        fs.open( m_persist ); // ,std::ios::in );

        Ta key; Entry value;

        char cspace;

        if( fs.is_open() ) while( !fs.eof() ){
            bool r =
                get_cache_field( fs ,key )
                && fs >> value.timestamp && fs >> cspace
                && get_cache_field( fs ,value.result )
            ;

            if( !r ) break;

            m_cache[key] = value;
        }

        fs.close();
    }

    void Save() {
        if( m_persist.empty() ) return;

        std::ofstream fs;

        fs.open( m_persist ); // ,std::ios::in );

        Ta key; Entry value;

        if( fs.is_open() ) {
            for( auto it=m_cache.begin(); it!=m_cache.end(); ++it ) {
                fs << it->first << '\x0' << it->second.timestamp << ':' << it->second.result << '\x0';
            }
        }

        fs.close();

        m_cacheDirty = false;
    }

public:
    bool getCached( const Ta &a ,Tb &b ,bool noInvalidate=false ) {
        m_cacheHit = false;

        auto it = m_cache.find(a);

        if( it == m_cache.end() )
            return false;

        Entry &entry = it->second;

        if( !noInvalidate ) {
            if( entry.timestamp + m_validity < time(NullPtr) ) {
                m_cache.erase(it); m_cacheDirty = true;
                return false;
            }
        }

        b = entry.result;

        return m_cacheHit = true;
    }

    void putCached( const Ta &a ,const Tb &b ) {
        Entry entry = { b ,time(NullPtr) };

        m_cache[a] = entry;

        //TODO ? check that response is actually different (nb also for timestamp)
            //=> maybe cached tracking on request is enough ?

        m_cacheDirty = true;
    }
};

//////////////////////////////////////////////////////////////////////////////
class CServiceQuota {
protected:
    int m_limit; //! request limit per interval
    time_t m_interval;

    ListOf<time_t> m_times;

    time_t m_synth; //! last synth time
    int m_requests; //! last synth request

    //TODO ... delay between calls ... here or in http ??

protected:
    String m_persist; //TODO

public:
    CServiceQuota( int limit=0 ,time_t interval=0 ) :
        m_limit(limit) ,m_interval(interval)
    {
        time( &m_synth );
        m_synth = time(NullPtr);
        m_requests = 0;

        //TODO persist
    }

    int &limit() { return m_limit; }
    time_t &interval() { return m_interval; }

    void setLimit( int limit ,time_t interval ) {
        m_limit = limit; m_interval = interval;
    }

public:
    bool canRequest() const {
        return (m_interval == 0 || m_requests < m_limit);
    }

    void Tidy();

    void accountRequest();
};

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_SERVICE_H