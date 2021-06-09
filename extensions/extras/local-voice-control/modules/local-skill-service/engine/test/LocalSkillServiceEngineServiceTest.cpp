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
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Storage/StorageConfiguration.h>
#include <AACE/LocalSkillService/LocalSkillServiceConfiguration.h>

#include <AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h>

namespace aace {
namespace test {
namespace unit {

#define LOCAL_STORAGE_PATH "/tmp/locaStorage.sqlite"
#define UNIX_SOCKET_PATH "/tmp/localSkillService.ipc"

/// Variable for storing the working directory.  This is where all of the test files will be created.
std::string workingDirectory;

/// Test harness for @c LocalSkillServiceEngineService
class LocalSkillServiceEngineServiceTest : public::testing::Test {
public:
    void SetUp() override {
        m_engine = aace::core::Engine::create();
    }

    void TearDown() override {
        m_engine.reset();// dispose pointer
        EXPECT_EQ(nullptr, m_engine) << "Engine creation did not return null";
    }

    std::shared_ptr<aace::core::Engine> m_engine;
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

TEST_F(LocalSkillServiceEngineServiceTest, localUnixService) {
    ASSERT_TRUE(m_engine->configure( {
        aace::storage::config::StorageConfiguration::createLocalStorageConfig( LOCAL_STORAGE_PATH ),
        aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig( UNIX_SOCKET_PATH )
    } )) << "Engine configuration failed";
    ASSERT_TRUE(m_engine->start()) << "engine should be started successfully";
    EXPECT_TRUE( httpUnixGet( UNIX_SOCKET_PATH, "http://localhost/toto/tutu" ) == 404 ) << "HTTP get an unexpected response";
    ASSERT_TRUE(m_engine->stop()) << "engine did not stop successfully";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
