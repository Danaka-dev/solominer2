/*************************************************************************
 * excerpt from 'libjson-rpc-cpp'
 *************************************************************************
 * @file    IClientConnector.h
 * @date    07.05.2024
 * @author  Danaka, Peter Spiess-Knafl
 ************************************************************************/

//! Copyright (c) 2023-2024 Danaka developers
//! Copyright (c) 2014 Krzysztof Okupski <dev@spiessknafl.at>
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINRPC_ICLIENTCONNECTOR_H
#define BITCOINRPC_ICLIENTCONNECTOR_H

///////////////////////////////////////////////////////////////////////////////
#include "exception.h"

#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace bitcoinrpc {

///////////////////////////////////////////////////////////////////////////////
class IClientConnector {
public:
    virtual ~IClientConnector() {}

    virtual void SendMessage( const std::string &message ,std::string &result ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace bitcoinrpc

///////////////////////////////////////////////////////////////////////////////
#endif //BITCOINRPC_ICLIENTCONNECTOR_H