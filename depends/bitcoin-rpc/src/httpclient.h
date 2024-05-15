/*************************************************************************
 * excerpt from 'libjson-rpc-cpp'
 *************************************************************************
 * @file    httpclient.h
 * @date    07.05.2024
 * @author  Danaka, Peter Spiess-Knafl
 ************************************************************************/

//! Copyright (c) 2023-2024 Danaka developers
//! Copyright (c) 2014 Krzysztof Okupski <dev@spiessknafl.at>
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINRPC_HTTPCLIENT_H
#define BITCOINRPC_HTTPCLIENT_H

///////////////////////////////////////////////////////////////////////////////
#include "iclientconnector.h"

#include <curl/curl.h>

#include <string>
#include <map>

///////////////////////////////////////////////////////////////////////////////
namespace bitcoinrpc {

///////////////////////////////////////////////////////////////////////////////
//! HttpClient

class HttpClient : public IClientConnector {
public:
    HttpClient( const std::string &url );
    virtual ~HttpClient();

    virtual void SendMessage( const std::string &message ,std::string &result );

    void SetUrl( const std::string &url );
    void SetTimeout( long timeout );

    void AddHeader( const std::string &attr ,const std::string &val );
    void RemoveHeader( const std::string &attr );

protected:
    std::map<std::string, std::string> headers;
    std::string url;

    long timeout;
    CURL *curl;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace bitcoinrpc

///////////////////////////////////////////////////////////////////////////////
#endif //BITCOINRPC_HTTPCLIENT_H