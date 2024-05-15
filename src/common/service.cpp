// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#include "common.h"
#include "service.h"

//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! CServiceQuota

void CServiceQuota::Tidy() {
    time_t now = Now();

    for( auto it = m_times.begin(); it != m_times.end(); )
    {
        if( *it + m_interval < now ) {
            it = m_times.erase(it); --m_requests;
        }
        else {
            ++it;
        }
    }
}

void CServiceQuota::accountRequest() {
    time_t now = Now();

    m_times.emplace_back( now );
    ++m_requests;

    Tidy();
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF