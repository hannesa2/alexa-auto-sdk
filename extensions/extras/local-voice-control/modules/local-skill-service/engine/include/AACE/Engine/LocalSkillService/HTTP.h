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


#ifndef AACE_ENGINE_LOCAL_SKILL_SERVICE_HTTP_H
#define AACE_ENGINE_LOCAL_SKILL_SERVICE_HTTP_H

#include <map>
#include <sstream>

#include <AVSCommon/Utils/Threading/Executor.h>

namespace aace {
namespace engine {
namespace localSkillService {

class HttpRequest;

typedef std::function<void ( std::shared_ptr<HttpRequest> request )> HTTPRequestHandler;

class HttpServer {
public:
    static std::shared_ptr<HttpServer> create( uint32_t port, uint32_t timeoutMs = 100 );
    static std::shared_ptr<HttpServer> create( const std::string& path, uint32_t timeoutMs = 100 );
    HttpServer( uint32_t port, uint32_t timeoutMs = 100 );
    HttpServer( const std::string& path, uint32_t timeoutMs = 100 );
    ~HttpServer();

    void acceptRequest();
    bool isRunning();
    void setRequestHandler( HTTPRequestHandler handler );
    bool start();
    bool stop();
private:
    void run();
    bool startInet();
    bool startUnix();

    bool isStopping{false};
    std::future<void> m_running;
    std::string m_path;
    int m_port;
    int m_server;
    uint32_t m_timeoutMs;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
    HTTPRequestHandler m_requestHandler;
};

class HttpRequest {
public:
    static const std::string CRLF;

    HttpRequest( int socket );
    ~HttpRequest();
    
    static std::shared_ptr<HttpRequest> create( int socket );

    const std::string& getBody();
    const std::string& getHeader( const std::string& key );
    const std::string& getMethod();
    const std::string& getPath();
    void respond( int status, const std::string& payload );

    bool read( std::string& buffer );
    void write( const std::string& buffer );
private:
	void onBody( std::string value );
    void onHeader( std::string key, std::string value );
	void onMethod( std::string value );
	void onPath( std::string value );

	std::string m_body;
	std::map<std::string, std::string> m_headers;
	std::string m_method;
	std::string m_path;
    int m_socket;
    ssize_t m_size{0};
    char* m_buffer{nullptr};
};

} // aace::engine::localSkillService
} // aace::engine
} // aace

#endif // AACE_ENGINE_LOCAL_SKILL_SERVICE_HTTP_H
