/*************************************************************************
 * excerpt from 'libjson-rpc-cpp'
 *************************************************************************
 * @file    client.h
 * @date    07.05.2024
 * @author  Danaka, Peter Spiess-Knafl
 ************************************************************************/

//! Copyright (c) 2023-2024 Danaka developers
//! Copyright (c) 2014 Krzysztof Okupski <dev@spiessknafl.at>
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINRPC_CLIENT_H
#define BITCOINRPC_CLIENT_H

///////////////////////////////////////////////////////////////////////////////
#include "iclientconnector.h"

#include <string>

///////////////////////////////////////////////////////////////////////////////
namespace bitcoinrpc {

///////////////////////////////////////////////////////////////////////////////
typedef enum { JSONRPC_CLIENT_V1, JSONRPC_CLIENT_V2 } clientVersion_t;

///////////////////////////////////////////////////////////////////////////////
class Client {
public:
    Client( IClientConnector &connector ,clientVersion_t version=JSONRPC_CLIENT_V2 ,bool omitEndingLineFeed=false );
    virtual ~Client();

    void CallMethod( const std::string &name ,const Json::Value &parameter ,Json::Value &result );
    Json::Value CallMethod( const std::string &name ,const Json::Value &parameter );

protected:
    //! @note from protocol
    static const std::string KEY_PROTOCOL_VERSION;
    static const std::string KEY_PROCEDURE_NAME;
    static const std::string KEY_ID;
    static const std::string KEY_PARAMETER;
    static const std::string KEY_AUTH;
    static const std::string KEY_RESULT;
    static const std::string KEY_ERROR;
    static const std::string KEY_ERROR_CODE;
    static const std::string KEY_ERROR_MESSAGE;
    static const std::string KEY_ERROR_DATA;

    clientVersion_t version;

    void BuildRequest( const std::string &method ,const Json::Value &parameter ,std::string &result ,bool isNotification );
    void BuildRequest( int id ,const std::string &method ,const Json::Value &parameter ,Json::Value &result ,bool isNotification );

    void HandleResponse( const std::string &response ,Json::Value &result );
    Json::Value HandleResponse( const Json::Value &value ,Json::Value &result );

    bool ValidateResponse( const Json::Value &response );
    bool HasError(const Json::Value &response);
    void throwErrorException(const Json::Value &response);

private:
    IClientConnector &connector;
};

///////////////////////////////////////////////////////////////////////////////
} //bitcoinrpc

///////////////////////////////////////////////////////////////////////////////
#endif //BITCOINRPC_CLIENT_H