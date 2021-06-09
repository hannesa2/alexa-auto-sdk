/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <stddef.h>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/LocalSkillService/HTTP.h"

namespace aace {
namespace engine {
namespace localSkillService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localSkillService.HttpServer");

#define INVALID_SOCKET -1
static const std::chrono::milliseconds SHORT_TIMEOUT_MS = std::chrono::milliseconds(1);

// HttpServer

std::shared_ptr<HttpServer> HttpServer::create( uint32_t port, uint32_t timeoutMs ) {
    return std::make_shared<HttpServer>( port, timeoutMs );
}

std::shared_ptr<HttpServer> HttpServer::create( const std::string& path, uint32_t timeoutMs ) {
    return std::make_shared<HttpServer>( path, timeoutMs );
}

HttpServer::HttpServer( uint32_t port, uint32_t timeoutMs ) : m_path(""), m_port(port), m_server(INVALID_SOCKET), m_timeoutMs(timeoutMs) {
}

HttpServer::HttpServer( const std::string& path, uint32_t timeoutMs ) : m_path(path), m_port(0), m_server(INVALID_SOCKET), m_timeoutMs(timeoutMs) {
}

HttpServer::~HttpServer() {
}

void HttpServer::acceptRequest()
{
    int selector, socket;
    struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_server, &rfds);

    tv.tv_sec = m_timeoutMs / 1000;
    tv.tv_usec = (m_timeoutMs % 1000) * 1000;

    selector = select( m_server + 1, &rfds, (fd_set *)0, (fd_set *)0, &tv );
    if ( selector > 0 ) {
        struct sockaddr_in client_addr;
        socklen_t size = sizeof(client_addr);
        if (( socket = accept( m_server, (struct sockaddr *)&client_addr,&size )) > 0) {
            auto request = HttpRequest::create( socket );
            if ( m_requestHandler && request ) {
                m_requestHandler( request );
            }
        }
    }
}

bool HttpServer::isRunning() {
    return m_server != INVALID_SOCKET;
}

void HttpServer::run() {
    while ( !isStopping ) {
        acceptRequest();
    }
}

void HttpServer::setRequestHandler(HTTPRequestHandler handler) {
    m_requestHandler = handler;
}

bool HttpServer::start() {
    if ( m_path.empty() && m_port) {
        return startInet();
    }
    else if ( !m_path.empty() && !m_port ) {
        return startUnix();
    }
    return false;
}

bool HttpServer::startInet() {
    if ( isRunning() ) return false;
    struct sockaddr_in server_addr;

    memset( &server_addr,0,sizeof( server_addr ));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( m_port );
    server_addr.sin_addr.s_addr = INADDR_ANY;

    try {
        m_server = socket( PF_INET, SOCK_STREAM, 0 );
        if ( m_server == INVALID_SOCKET ) {
            throw std::runtime_error( strerror( errno ));
        }
        int reuse = 1;
        if ( setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        if ( bind(m_server, (const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        if ( listen(m_server, SOMAXCONN) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        isStopping = false;
        m_running = m_executor.submit([this]() {
            run();
        });
        m_running.wait_for( SHORT_TIMEOUT_MS );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        close( m_server );
        m_server = INVALID_SOCKET;
    }
    return false;
}

bool HttpServer::startUnix() {
    if (isRunning()) return false;
    struct sockaddr_un server_addr;

    try {
        size_t maxLen = sizeof(server_addr) - offsetof(struct sockaddr_un, sun_path) - 1;
        ThrowIf(m_path.size() > maxLen, "socketPathTooLong");
        memset( &server_addr,0,sizeof(server_addr) );
        server_addr.sun_family = AF_UNIX;
        strncpy( server_addr.sun_path, m_path.c_str(), maxLen );
        m_server = socket( AF_UNIX, SOCK_STREAM, 0 );
        if ( m_server == INVALID_SOCKET ) {
            throw std::runtime_error(strerror( errno ));
        }
        int reuse = 1;
        if ( setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        unlink( m_path.c_str() );
        if ( bind(m_server, (const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        if ( chmod(m_path.c_str(), 0600) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        if ( listen(m_server, SOMAXCONN) < 0 ) {
            throw std::runtime_error(strerror(errno));
        }
        isStopping = false;
        m_running = m_executor.submit([this]() {
            run();
        });
        m_running.wait_for( SHORT_TIMEOUT_MS );
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        close( m_server );
        m_server = INVALID_SOCKET;
    }
    return false;
}

bool HttpServer::stop() {
    if (!isRunning()) return false;
    isStopping = true;
    m_executor.shutdown();
    close( m_server );
    m_server = INVALID_SOCKET;
    return true;
}

// HttpRequest

const std::string HttpRequest::CRLF = std::string("\r\n");

std::shared_ptr<HttpRequest> HttpRequest::create( int socket ) {
    auto request = std::make_shared<HttpRequest>(socket);
    std::string buffer = "";
    int index;
    int state = 0;
    int bodySize = 0;
    
    while ( request->read( buffer )) {
        if (state < 2) { // headers
			while (( index = buffer.find( HttpRequest::CRLF )) > 0 ) {
				if ( index > 0 ) {
					std::string line = buffer.substr( 0, index );
					buffer = buffer.substr( index + 2 );
					switch ( state ) {
						case 0: {
							state = 1;
							int index = line.find( " " );
							if ( index > 0 ) {
								request->onMethod(line.substr( 0, index ));
								index = line.find_first_not_of( " ", index );
								line = line.substr( index );
							}
							index = line.find( " " );
							if ( index > 0 ) {
								request->onPath( line.substr( 0, index ) );
							}
						}
						break;
						case 1: {
							index = line.find( ":" );
							std::string key = line.substr( 0, index );
							transform( key.begin(), key.end(), key.begin(),::tolower );
							index = line.find_first_not_of( " ", index + 1 );
							std::string value = line.substr( index );
							request->onHeader( key, value );
							if (key == "content-length") {
								bodySize = stoi( value );
							}
						}
						break;
					}
				}
			}
			if ( index == 0 ) {
				buffer = buffer.substr( 2 );
				state = 2;
			}
        }
        if ( state == 2 ) { // body
			if ( buffer.length() == bodySize ) {
				if ( bodySize ) {
					request->onBody( buffer );
				}
				state = 3;
				break;
			}
        }
    }
	if ( state != 3 ) {
		return nullptr;
	}
	return request;
}

HttpRequest::HttpRequest( int socket ) : m_socket( socket ) {
    m_size = 1024;
    m_buffer = new char[m_size + 1];
}

HttpRequest::~HttpRequest() {
    close(m_socket);
    m_socket = INVALID_SOCKET;
    delete [] m_buffer;
    m_buffer = nullptr;
    m_size = 0;
}

const std::string& HttpRequest::getBody() {
    return m_body;
}

const std::string& HttpRequest::getHeader( const std::string& key ) {
    return m_headers[key];
}

const std::string& HttpRequest::getMethod() {
    return m_method;
}

const std::string& HttpRequest::getPath() {
    return m_path;
}

void HttpRequest::onHeader( std::string key, std::string value ) {
	m_headers[key] = value;
}

void HttpRequest::onBody( std::string value ) {
	m_body = value;
}

void HttpRequest::onMethod( std::string value ) {
	m_method = value;
}

void HttpRequest::onPath( std::string value ) {
	m_path = value;
}

bool HttpRequest::read( std::string& buffer ) {
    int nread = recv( m_socket, m_buffer, 1024, 0 );
    if ( nread < 0 ) {
        if ( errno == EINTR ) // the socket call was interrupted -- try again
            return true;
        else // an error occurred, so break out
            return false;
    }
    else if ( nread == 0 ) { // the socket is closed
        throw std::runtime_error( "Connection Closed!" );
    }
    buffer.append( m_buffer, nread );
    return true;
}

void HttpRequest::respond( int status, const std::string& payload ) {
    try {
        switch ( status ) {
        case 200:
            write("HTTP/1.1 200 OK");
            break;
        case 204:
            write("HTTP/1.1 204 No Content");
            break;
        case 400:
            write("HTTP/1.1 400 Bad Request");
            break;
        case 404:
            write("HTTP/1.1 404 Not Found");
            break;
        default:
            write("HTTP/1.1 500 Internal Server Error");
            break;
        }
        write( engine::localSkillService::HttpRequest::CRLF );
        write( "Connection: close" + engine::localSkillService::HttpRequest::CRLF );
        write( "Content-Length: " + std::to_string(payload.size()) + engine::localSkillService::HttpRequest::CRLF );
        write( engine::localSkillService::HttpRequest::CRLF );
        write( payload );
    } catch ( std::exception & ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("status", status));
    }
}

void HttpRequest::write( const std::string& buffer ) {
    const char* ptr = buffer.c_str();
    int length = buffer.length();
    int count;
    // loop to be sure it is all sent
    while ( length ) {
        if (( count = send(m_socket, ptr, length, 0) ) < 0) {
            if ( errno == EINTR ) { // the socket call was interrupted -- try again
                continue;
            } else { // an error occurred, so break out
                throw std::runtime_error( "Write error! errno: " + std::to_string(errno));
            }
        } else if (count == 0) { // the socket is closed
            throw std::runtime_error( "Connection Closed!" );
        }
        length -= count;
        ptr += count;
    }
}

} // aace::engine::localSkillService
} // aace::engine
} // aace

