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
 
#include <curl/curl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include <AACE/Engine/LocalSkillService/HTTP.h>

namespace aace {
namespace test {
namespace unit {

/// Test harness for @c HTTPServer class
class HTTPTest : public::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

auto gErrorRequestHandler = []( std::shared_ptr<engine::localSkillService::HttpRequest> request ) {
    request->write("HTTP/1.1 500 Internal Server Error" + engine::localSkillService::HttpRequest::CRLF);
    request->write("Connection: close" + engine::localSkillService::HttpRequest::CRLF);
    request->write(engine::localSkillService::HttpRequest::CRLF);
 };

auto gNotFoundRequestHandler = []( std::shared_ptr<engine::localSkillService::HttpRequest> request ) {
    request->write("HTTP/1.1 404 Not Found" + engine::localSkillService::HttpRequest::CRLF);
    request->write("Connection: close" + engine::localSkillService::HttpRequest::CRLF);
    request->write(engine::localSkillService::HttpRequest::CRLF);
 };

long httpGet( const std::string& url ) {
    CURL *curl = curl_easy_init();
    long status = 0;
    if (curl) {
        EXPECT_TRUE( curl_easy_setopt( curl, CURLOPT_URL, url.c_str() ) == CURLE_OK ) << "HTTP cannot set URL";
        if ( curl_easy_perform( curl ) == CURLE_OK ) {
            EXPECT_TRUE( curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status ) == CURLE_OK ) << "HTTP cannot get response code";
        }
        curl_easy_cleanup(curl);
    }
    return status;
}

long httpUnixGet( const std::string& path, const std::string& url ) {
    CURL *curl = curl_easy_init();
    long status = 0;
    if (curl) {
        EXPECT_TRUE( curl_easy_setopt( curl, CURLOPT_URL, url.c_str() ) == CURLE_OK ) << "HTTP cannot set URL";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, path.c_str()) == CURLE_OK ) << "HTTP cannot set Unix socket path";
        if ( curl_easy_perform( curl ) == CURLE_OK ) {
            EXPECT_TRUE( curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &status ) == CURLE_OK ) << "HTTP cannot get response code";
        }
        curl_easy_cleanup(curl);
    }
    return status;
}

TEST_F(HTTPTest, oneServer) {
    aace::engine::localSkillService::HttpServer server(8081);
    auto result = server.start();
    EXPECT_TRUE(result) << "server not started";
    EXPECT_TRUE(server.isRunning()) << "HTTP Server not running";
    server.stop();
}

TEST_F(HTTPTest, twoServersOnTheSamePort) {
    aace::engine::localSkillService::HttpServer server1(8081);
    aace::engine::localSkillService::HttpServer server2(8081);
    auto result1 = server1.start();
    EXPECT_TRUE(result1) << "server1 not started";
    EXPECT_TRUE(server1.isRunning()) << "server1 not running";
    auto result2 = server2.start();
    EXPECT_FALSE(result2) << "server2 not started";
    server1.stop();
}

TEST_F(HTTPTest, oneRequest) {
    aace::engine::localSkillService::HttpServer server(8081);
    server.setRequestHandler(gErrorRequestHandler);
    auto result = server.start();
    EXPECT_TRUE(result) << "server not started";
    EXPECT_TRUE(server.isRunning()) << "HTTP Server not running";
    EXPECT_TRUE( httpGet("http://localhost:8081/toto/tutu") == 500 ) << "HTTP response status incorrect";
    server.stop();
}

TEST_F(HTTPTest, oneRequestWithoutHandler) {
    aace::engine::localSkillService::HttpServer server(8081);
    auto result = server.start();
    EXPECT_TRUE(result) << "server not started";
    EXPECT_TRUE(server.isRunning()) << "HTTP Server not running";
    EXPECT_TRUE( httpGet("http://localhost:8081/toto/tutu") == 0 ) << "HTTP get an unexpected response";
    server.stop();
}

TEST_F(HTTPTest, twoServersTwoRequests) {
    aace::engine::localSkillService::HttpServer server1(8081);
    server1.setRequestHandler(gErrorRequestHandler);
    auto result1 = server1.start();
    EXPECT_TRUE(result1) << "server1 not started";
    EXPECT_TRUE(server1.isRunning()) << "server1 not running";

    aace::engine::localSkillService::HttpServer server2(8082);
    server2.setRequestHandler(gNotFoundRequestHandler);
    auto result2 = server2.start();
    EXPECT_TRUE(result2) << "server2 not started";
    EXPECT_TRUE(server2.isRunning()) << "result2 not running";

    EXPECT_TRUE( httpGet("http://localhost:8081/toto/tutu") == 500 ) << "HTTP get an unexpected response";
    EXPECT_TRUE( httpGet("http://localhost:8082/toto/tutu") == 404 ) << "HTTP get an unexpected response";

    server2.stop();
    server1.stop();
}

#define UNIX_SOCKET_PATH "/tmp/localSkillService.ipc"

TEST_F(HTTPTest, oneUnixServer) {
    aace::engine::localSkillService::HttpServer server(UNIX_SOCKET_PATH);
    auto result = server.start();
    EXPECT_TRUE(result) << "server not started";
    EXPECT_TRUE(server.isRunning()) << "HTTP Server not running";
    server.stop();
}

TEST_F(HTTPTest, oneUnixRequest) {
    aace::engine::localSkillService::HttpServer server(UNIX_SOCKET_PATH);
    server.setRequestHandler(gErrorRequestHandler);
    auto result = server.start();
    EXPECT_TRUE(result) << "server not started";
    EXPECT_TRUE(server.isRunning()) << "HTTP Server not running";
    EXPECT_TRUE( httpUnixGet(UNIX_SOCKET_PATH, "http://localhost/toto/tutu") == 500 ) << "HTTP response status incorrect";
    server.stop();
}


}  // namespace unit
}  // namespace test
}  // namespace aace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
