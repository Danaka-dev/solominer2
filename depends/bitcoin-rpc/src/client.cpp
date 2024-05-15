/*************************************************************************
 * excerpt from 'libjson-rpc-cpp'
 *************************************************************************
 * @file    client.cpp
 * @date    07.05.2024
 * @author  Danaka, Peter Spiess-Knafl
 ************************************************************************/

//! Copyright (c) 2023-2024 Danaka developers
//! Copyright (c) 2014 Krzysztof Okupski <dev@spiessknafl.at>
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string>

///////////////////////////////////////////////////////////////////////////////
#include "client.h"

#include <sstream>

///////////////////////////////////////////////////////////////////////////////
using namespace bitcoinrpc;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
Client::Client( IClientConnector &connector ,clientVersion_t version ,bool omitEndingLineFeed ) :
    version(version) ,connector(connector)
{}

Client::~Client()
{}

///--
void Client::CallMethod(const std::string &name, const Json::Value &parameter, Json::Value &result) {
    std::string request ,response;

    BuildRequest(name, parameter, request, false);
    connector.SendMessage(request, response);
    HandleResponse(response, result);
}

Json::Value Client::CallMethod(const std::string &name, const Json::Value &parameter) {
    Json::Value result;
    this->CallMethod(name, parameter, result);
    return result;
}

///--
const std::string Client::KEY_PROTOCOL_VERSION = "jsonrpc";
const std::string Client::KEY_PROCEDURE_NAME = "method";
const std::string Client::KEY_ID = "id";
const std::string Client::KEY_PARAMETER = "params";
const std::string Client::KEY_AUTH = "auth";
const std::string Client::KEY_RESULT = "result";
const std::string Client::KEY_ERROR = "error";
const std::string Client::KEY_ERROR_CODE = "code";
const std::string Client::KEY_ERROR_MESSAGE = "message";
const std::string Client::KEY_ERROR_DATA = "data";

void Client::BuildRequest(const std::string &method, const Json::Value &parameter, std::string &result, bool isNotification) {
    Json::Value request;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "";
    this->BuildRequest(1, method, parameter, request, isNotification);

    result = Json::writeString(wbuilder, request);
}

void Client::BuildRequest(int id, const std::string &method, const Json::Value &parameter, Json::Value &result, bool isNotification) {
    if (this->version == JSONRPC_CLIENT_V2)
        result[KEY_PROTOCOL_VERSION] = "2.0";
    result[KEY_PROCEDURE_NAME] = method;
    if (parameter != Json::nullValue)
        result[KEY_PARAMETER] = parameter;
    if (!isNotification)
        result[KEY_ID] = id;
    else if (this->version == JSONRPC_CLIENT_V1)
        result[KEY_ID] = Json::nullValue;
}

void Client::HandleResponse(const std::string &response, Json::Value &result) {
    Json::Value value;

    try {
        if (std::istringstream(response) >> value) {
            this->HandleResponse(value, result);
        } else {
            throw BitcoinRpcException(ERROR_RPC_JSON_PARSE_ERROR, " " + response);
        }
    } catch (Json::Exception &e) {
        throw BitcoinRpcException(ERROR_RPC_JSON_PARSE_ERROR, " " + response);
    }
}

Json::Value Client::HandleResponse(const Json::Value &value, Json::Value &result) {
    if (this->ValidateResponse(value)) {
        if (this->HasError(value)) {
            this->throwErrorException(value);
        } else {
            result = value[KEY_RESULT];
        }
    } else {
        throw BitcoinRpcException(ERROR_CLIENT_INVALID_RESPONSE, " " + value.toStyledString());
    }
    return value[KEY_ID];
}

bool Client::ValidateResponse(const Json::Value &response) {
    if (!response.isObject() || !response.isMember(KEY_ID))
        return false;

    if (this->version == JSONRPC_CLIENT_V1) {
        if (!response.isMember(KEY_RESULT) || !response.isMember(KEY_ERROR))
            return false;
        if (!response[KEY_RESULT].isNull() && !response[KEY_ERROR].isNull())
            return false;
        if (!response[KEY_ERROR].isNull() &&
            !(response[KEY_ERROR].isObject() && response[KEY_ERROR].isMember(KEY_ERROR_CODE) && response[KEY_ERROR][KEY_ERROR_CODE].isIntegral()))
            return false;
    } else if (this->version == JSONRPC_CLIENT_V2) {
        if (!response.isMember(KEY_PROTOCOL_VERSION) || response[KEY_PROTOCOL_VERSION] != "2.0")
            return false;
        if (response.isMember(KEY_RESULT) && response.isMember(KEY_ERROR))
            return false;
        if (!response.isMember(KEY_RESULT) && !response.isMember(KEY_ERROR))
            return false;
        if (response.isMember(KEY_ERROR) &&
            !(response[KEY_ERROR].isObject() && response[KEY_ERROR].isMember(KEY_ERROR_CODE) && response[KEY_ERROR][KEY_ERROR_CODE].isIntegral()))
            return false;
    }

    return true;
}

bool Client::HasError(const Json::Value &response) {
    if (this->version == JSONRPC_CLIENT_V1 && !response[KEY_ERROR].isNull())
        return true;
    else if (this->version == JSONRPC_CLIENT_V2 && response.isMember(KEY_ERROR))
        return true;
    return false;
}

void Client::throwErrorException(const Json::Value &response) {
    if (response[KEY_ERROR].isMember(KEY_ERROR_MESSAGE) && response[KEY_ERROR][KEY_ERROR_MESSAGE].isString()) {
        if (response[KEY_ERROR].isMember(KEY_ERROR_DATA)) {
            throw BitcoinRpcException(response[KEY_ERROR][KEY_ERROR_CODE].asInt(), response[KEY_ERROR][KEY_ERROR_MESSAGE].asString());
        } else {
            throw BitcoinRpcException(response[KEY_ERROR][KEY_ERROR_CODE].asInt(), response[KEY_ERROR][KEY_ERROR_MESSAGE].asString());
        }
    } else {
        throw BitcoinRpcException(response[KEY_ERROR][KEY_ERROR_CODE].asInt());
    }
}

///////////////////////////////////////////////////////////////////////////////
//EOF