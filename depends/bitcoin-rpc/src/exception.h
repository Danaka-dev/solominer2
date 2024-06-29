/**
 * @file    exception.h
 * @author  Danaka, Krzysztof Okupski
 * @date    07.05.2024
 * @version 1.1
 *
 * Declaration of exception class for the JSON-RPC wrapper.
 */

//! Copyright (c) 2023-2024 Danaka developers
//! Copyright (c) 2014 Krzysztof Okupski
//! Distributed under the MIT software license, see the accompanying
//! file LICENSE or http://www.opensource.org/licenses/mit-license.php.

///////////////////////////////////////////////////////////////////////////////
#ifndef BITCOINRPC_EXCEPTION_H
#define BITCOINRPC_EXCEPTION_H

///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <sstream>
#include <iostream>

// #include <jsoncpp/json/json.h>
#include <json/json.h> //! uncomment if above include is not found on your configuration

///////////////////////////////////////////////////////////////////////////////
using Json::Value;
using Json::Reader;

///////////////////////////////////////////////////////////////////////////////
namespace bitcoinrpc {
    const int ERROR_RPC_JSON_PARSE_ERROR = -32700;
    const int ERROR_RPC_METHOD_NOT_FOUND = -32601;
    const int ERROR_RPC_INVALID_REQUEST = -32600;
    const int ERROR_RPC_INVALID_PARAMS = -32602;
    const int ERROR_RPC_INTERNAL_ERROR = -32603;

    const int ERROR_SERVER_PROCEDURE_IS_METHOD = -32604;
    const int ERROR_SERVER_PROCEDURE_IS_NOTIFICATION = -32605;
    const int ERROR_SERVER_PROCEDURE_POINTER_IS_NULL = -32606;
    const int ERROR_SERVER_PROCEDURE_SPECIFICATION_NOT_FOUND = -32000;
    const int ERROR_SERVER_CONNECTOR = -32002;
    const int ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX = -32007;

    const int ERROR_CLIENT_CONNECTOR = -32003;
    const int ERROR_CLIENT_INVALID_RESPONSE = -32001;
}

///////////////////////////////////////////////////////////////////////////////
class BitcoinRpcException : public std::exception {
private:
	int code;
	std::string msg;

public:
    explicit BitcoinRpcException( int errcode ) {
        this->code = errcode;
    }

	explicit BitcoinRpcException( int errcode , const std::string& message) {
		/* Connection error */
		if(errcode == bitcoinrpc::ERROR_CLIENT_CONNECTOR){
			this->code = errcode;
			this->msg = removePrefix(message, " -> ");		
		/* Authentication error */
		}else if(errcode == bitcoinrpc::ERROR_RPC_INTERNAL_ERROR && message.size() == 18){
			this->code = errcode;
			this->msg = "Failed to authenticate successfully";
		/* Miscellaneous error */
		}else{
			this->code = parseCode(message);
			this->msg = parseMessage(message);
		}
	}

	~BitcoinRpcException() throw() { };

	int getCode(){
		return code;
	}

	std::string getMessage(){
		return msg;
	}

	std::string removePrefix(const std::string& in, const std::string& pattern){
		std::string ret = in;

		unsigned int pos = ret.find(pattern);

		if(pos <= ret.size()){
			ret.erase(0, pos+pattern.size());
		}

		return ret;
	}

	/* Auxiliary JSON parsing */
	int parseCode(const std::string& in){
		Value root;
		Reader reader;

		/* Remove JSON prefix */
		std::string strJson = removePrefix(in, "INTERNAL_ERROR: : ");
		int ret = -1;

		/* Parse error message */
		bool parsingSuccessful = reader.parse(strJson.c_str(), root);
		if(parsingSuccessful) {
			ret = root["error"]["code"].asInt();
		}

		return ret;
	}

	std::string parseMessage(const std::string& in){
		Value root;
		Reader reader;

		/* Remove JSON prefix */
		std::string strJson = removePrefix(in, "INTERNAL_ERROR: : ");
		std::string ret = "Error during parsing of >>" + strJson + "<<";

		/* Parse error message */
		bool parsingSuccessful = reader.parse(strJson.c_str(), root);
		if(parsingSuccessful) {
			ret = removePrefix(root["error"]["message"].asString(), "Error: ");
			ret[0] = toupper(ret[0]);
		}

		return ret;
	}
};

///////////////////////////////////////////////////////////////////////////////
#endif //BITCOINRPC_EXCEPTION_H