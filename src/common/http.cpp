// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//! HttpStatus code section from:
/*!
 * HTTP Status Codes - C++ Variant
 *
 * https://github.com/j-ulrich/http-status-codes-cpp
 *
 * \ version 1.5.0
 * \ author Jochen Ulrich <jochenulrich@t-online.de>
 * \ copyright Licensed under Creative Commons CC0 (http://creativecommons.org/publicdomain/zero/1.0/)
 */

//////////////////////////////////////////////////////////////////////////////
#include <common/common.h>

#include <curl/curl.h>

#include <string>

#include "http.h"

//////////////////////////////////////////////////////////////////////////////
//! HttpStatus

namespace HttpStatus {

/*! Returns the standard HTTP reason phrase for a HTTP status code.
 * \param code An HTTP status code.
 * \return The standard HTTP reason phrase for the given \p code or an empty \c std::string()
 * if no standard phrase for the given \p code is known.
 */
std::string reasonPhrase( int code ) {
    switch( code ) {
        //####### 1xx - Informational #######
        case 100: return "Continue";
        case 101: return "Switching Protocols";
        case 102: return "Processing";
        case 103: return "Early Hints";

            //####### 2xx - Successful #######
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";
        case 207: return "Multi-Status";
        case 208: return "Already Reported";
        case 226: return "IM Used";

            //####### 3xx - Redirection #######
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 305: return "Use Proxy";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";

            //####### 4xx - Client Error #######
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Content Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 416: return "Range Not Satisfiable";
        case 417: return "Expectation Failed";
        case 418: return "I'm a teapot";
        case 421: return "Misdirected Request";
        case 422: return "Unprocessable Content";
        case 423: return "Locked";
        case 424: return "Failed Dependency";
        case 425: return "Too Early";
        case 426: return "Upgrade Required";
        case 428: return "Precondition Required";
        case 429: return "Too Many Requests";
        case 431: return "Request Header Fields Too Large";
        case 451: return "Unavailable For Legal Reasons";

            //####### 5xx - Server Error #######
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        case 506: return "Variant Also Negotiates";
        case 507: return "Insufficient Storage";
        case 508: return "Loop Detected";
        case 510: return "Not Extended";
        case 511: return "Network Authentication Required";

        default: return std::string();
    }
}

} // namespace HttpStatus

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
namespace solominer {

//////////////////////////////////////////////////////////////////////////////
//! Curl

namespace curl {

struct string {
    char *ptr;
    size_t len;
};

static void init( struct string *s ) {
    s->len = 0;
    s->ptr = static_cast<char *>(malloc( s->len + 1 ));
    s->ptr[0] = '\0';
}

static size_t writeFunction( void *ptr ,size_t size ,size_t nmemb ,struct string *s ) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = (char *) realloc( s->ptr ,new_len + 1 );
    memcpy( s->ptr + s->len ,ptr ,size * nmemb );
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size * nmemb;
}

static iresult_t HttpPerform( const char *url ,HttpMethod method ,const MapOf<std::string,std::string> &headers ,const char *userpass ,const char *body ,HttpResponse &response ,long timeoutMs=10000 ) {
//-- init
    CURL * curl = curl_easy_init();

    curl_easy_setopt( curl ,CURLOPT_NOSIGNAL ,1 );
    curl_easy_setopt( curl ,CURLOPT_URL ,url );
    curl_easy_setopt( curl ,CURLOPT_WRITEFUNCTION ,writeFunction );

//-- headers
    struct curl_slist *curl_headers = nullptr;

    bool hasContentType = false;

    for( auto it = headers.begin(); it != headers.end(); ++it ) {
        const auto &hkey = it->first;
        const auto &hvalue = it->second;

        if( hkey == "Content-Type" )
            hasContentType = true;

        curl_headers = curl_slist_append( curl_headers ,(hkey + ": " + hvalue).c_str() );
    }

    curl_headers = curl_slist_append( curl_headers ,"charsets: utf-8" );

//-- authorization
    if( userpass && userpass[0] ) {
        curl_easy_setopt( curl ,CURLOPT_HTTPAUTH ,CURLAUTH_BASIC ); // CURLAUTH_DIGEST
        curl_easy_setopt( curl ,CURLOPT_USERPWD ,userpass );
    }

//-- method & option
    struct string s;
    init( &s );

    switch( method ) {
        case HttpMethod::methodGET:
            curl_easy_setopt( curl ,CURLOPT_HTTPGET ,1 );
            break;

        case HttpMethod::methodPOST:
            // curl_easy_setopt( curl ,CURLOPT_MIMEPOST ,1 );
            curl_easy_setopt( curl ,CURLOPT_POSTFIELDS ,body );

            if( body && !hasContentType ) {
                curl_headers = curl_slist_append( curl_headers ,"Content-Type: text/plain" );
            }

            break;

        default:
            return INOEXEC;
    }

    curl_easy_setopt( curl ,CURLOPT_WRITEDATA ,&s );
    curl_easy_setopt( curl ,CURLOPT_HTTPHEADER ,curl_headers );
    curl_easy_setopt( curl ,CURLOPT_TIMEOUT_MS ,timeoutMs );

//-- perform
    CURLcode result = curl_easy_perform( curl );

//-- response
    if( result != CURLE_OK ) {
        std::stringstream ss;

        ss << "libcurl error: " << result;

        if( result == CURLE_COULDNT_CONNECT ) {
            ss << " -> Could not connect to " << url;
            response.status = HttpStatus::NotFound;
        }
        else if( result == CURLE_OPERATION_TIMEDOUT ) {
            ss << " -> Operation timed out";
            response.status = HttpStatus::RequestTimeout;
        } else {
            response.status = HttpStatus::BadRequest;
        }

        response.content = ss.str();

        return IERROR;
    }

    //-- ok
    long http_code = 0;

    curl_easy_getinfo( curl ,CURLINFO_RESPONSE_CODE ,&http_code );

    response.status = (HttpStatus::Code) http_code;
    response.content = s.ptr;

//-- return
    free( s.ptr );
    curl_slist_free_all( curl_headers );

    return IOK;
}

}

//////////////////////////////////////////////////////////////////////////////
//! Service

//////////////////////////////////////////////////////////////////////////////
//! CHttpRequest

void CHttpRequest::updateCacheControl() {
    const auto &it = m_headers.find("Cache-Control");

    if( it != m_headers.end() ) {
        m_cacheControl = it->second;
    }
}

///-- synch
iresult_t CHttpRequest::Send( HttpResponse &response ) {
    iresult_t result = curl::HttpPerform( m_url.c_str() ,m_method ,m_headers ,m_userpass.c_str() ,m_body.c_str() ,response );

    IF_IFAILED_RETURN(result);

    m_status = response.status;
    m_response = response.content;

    return IOK;
}

///-- asynch
iresult_t CHttpRequest::Send() {
    m_result = INODATA;
    m_response.clear();

    //TODO send through the one thread and asynch get the result

    return ENOEXEC;
}

void CHttpRequest::gotResponse( const HttpResponse &response ) {
    //TODO critical section to mutually protected got/get response

    m_result = IOK;
    m_status = response.status;
    m_response = response.content;

    if( m_listener ) {
        m_listener->onResponse( *this ,response );
    }
}

iresult_t CHttpRequest::getResponse( HttpResponse &response ) {
    IF_IFAILED_RETURN(m_result);

//LOCK ASYNCH here
    response.status = m_status;
    response.content = m_response;

    return IOK;
}

iresult_t CHttpRequest::Cancel() {
    //TODO advise send thread to cancel

    return ENOEXEC;
}

///--
iresult_t CHttpRequest::adviseResponseValid() {
    if( m_connection ) {
        m_connection->adviseRequestValid( *this );
    }

    return IOK;
}

//////////////////////////////////////////////////////////////////////////////
//! CHttpConnection

String CHttpConnection::makeUrl( const char *host ,const char *path ) {
    std::stringstream ss;

    ss << host << ((path[0] != '/') ? "/" : "") << path;

    return ss.str();
}

///-- request
iresult_t CHttpConnection::makeRequest( const char *url ,HttpMethod method ,const HttpMessage &message ,CHttpRequest &request ) {
    request.url() = url;
    request.method() = method;

    auto &headers = request.headers();

    if( message.content ) {
        headers["Content-type"] = message.content_type ? message.content_type : "text/plain";
        request.body() = message.content;
    }

    if( message.accept_type ) {
        headers["accept"] = message.accept_type;
    }

    return IOK;
}

iresult_t CHttpConnection::Send( CHttpRequest &request ,HttpResponse &response ) {
    bool canRequest = quota().canRequest();

    iresult_t ir;

///-- cached
    request.updateCacheControl();

    if( request.method() == HttpMethod::methodGET && request.cacheControl().empty() ) {
        if( cache().getCached( request.url() ,response.content ,!canRequest ) ) {
            response.status = HttpStatus::OK;
            request.m_cached = true;
            return IOK;
        }
    }

///-- quota
    if( !canRequest )
        return IREFUSED;

///-- send
    request.m_connection = this;

    ir = request.Send( response ); IF_IFAILED_RETURN(ir);

///-- handle response
    quota().accountRequest();

    return IOK;
}

iresult_t CHttpConnection::Send( CHttpRequest &request ,IHttpListener &listener ) {
    request.setListener( &listener );

    return Send( request );
}

iresult_t CHttpConnection::Send( CHttpRequest &request ) {
    //TODO, want to have response coming from call thread even if cached to avoid different synchro behavior

    // check cache + quota

    // ir = request.Send(); ...

    return INOEXEC;
}

///-- callback
iresult_t CHttpConnection::adviseRequestValid( const CHttpRequest &request ) {
    if( request.isFromCache() )
        return IOK;

    if( HttpStatus::isError( request.status() ) )
        return IERROR;

    if( request.method() != HttpMethod::methodGET && request.method() != methodHEAD )
        return IOK;

    if( request.cacheControl() == "no-store" )
        return IOK;

    //TODO use cache max age "max-age=604800"
    m_cache.putCached( request.url() ,request.response() );

    if( m_cache.cacheDirty() ) {
        m_cache.Save();
    }

    return IOK;
}

///-- Helpers
iresult_t CHttpConnection::sendRequest( const char *url ,HttpMethod method ,const HttpMessage &message ,CHttpRequest &request ,HttpResponse &response ) {
    request.m_connection = this;

    iresult_t ir;

    ir = makeRequest( url ,method ,message ,request ); IF_IFAILED_RETURN(ir);

    return Send( request ,response );
}

iresult_t CHttpConnection::sendRequest( const char *url ,HttpMethod method ,const HttpMessage &message ,HttpResponse &response ) {
    CHttpRequest request( this );

    return sendRequest( url ,method ,message ,request ,response );
}

//////////////////////////////////////////////////////////////////////////////
} //namespace solominer

//////////////////////////////////////////////////////////////////////////////
//EOF