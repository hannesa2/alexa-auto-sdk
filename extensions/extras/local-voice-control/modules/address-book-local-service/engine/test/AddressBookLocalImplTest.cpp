/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>


#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Storage/StorageConfiguration.h>
#include <AACE/LocalSkillService/LocalSkillServiceConfiguration.h>

#include <AACE/AddressBook/AddressBook.h>
#include <AACE/Engine/AddressBookLocalService/AddressBookLocalImpl.h>

namespace aace {
namespace test {
namespace unit {

#define LOCAL_STORAGE_PATH "/tmp/locaStorage.sqlite"
#define LOCAL_SKILL_SERVICE_PATH "/tmp/ER.socket"

class Utilities {
public:
    static std::string httpGetString(const std::string& url);
    static std::string httpPostString(const std::string& url, const std::string& request);
    static bool hasString(const std::string& result, const std::string& value);
};

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
        if (mem->memory == NULL) {
        /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[ mem->size ]), contents, realsize);
    mem->size += realsize;
    mem->memory[ mem->size ] = 0;

    return realsize;
}


std::string Utilities::httpGetString(const std::string& url) {
    CURL *curl = curl_easy_init();
    std::string response = "";
    long status = 0;
    struct MemoryStruct chunk;
 
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    if (curl) {
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()) == CURLE_OK) << "HTTP cannot set URL";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, LOCAL_SKILL_SERVICE_PATH ) == CURLE_OK) << "HTTP cannot set Unix socket path";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback) == CURLE_OK) << "HTTP cannot set URL";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk) == CURLE_OK) << "HTTP cannot set URL";
        if (curl_easy_perform(curl) == CURLE_OK) {
            EXPECT_TRUE(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK) << "HTTP cannot get response code";
        }
        curl_easy_cleanup(curl);
    }
    if (chunk.size > 0) {
        response = std::string(chunk.memory);
    }
    free(chunk.memory);
    return response;
}

std::string Utilities::httpPostString(const std::string& url, const std::string& request) {
    CURL *curl = curl_easy_init();
    std::string response = "";
    long status = 0;
    struct MemoryStruct chunk;
 
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    if (curl) {
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_URL, url.c_str()) == CURLE_OK) << "HTTP cannot set URL";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, LOCAL_SKILL_SERVICE_PATH ) == CURLE_OK) << "HTTP cannot set Unix socket path";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str()) == CURLE_OK) << "HTTP cannot request content";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback) == CURLE_OK) << "HTTP cannot set URL";
        EXPECT_TRUE(curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk) == CURLE_OK) << "HTTP cannot set URL";
        if (curl_easy_perform(curl) == CURLE_OK) {
            EXPECT_TRUE(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK) << "HTTP cannot get response code";
        }
        curl_easy_cleanup(curl);
    }
    if (chunk.size > 0) {
        response = std::string(chunk.memory);
    }
    free(chunk.memory);
    std::cout << response << std::endl;
    return response;
}

bool Utilities::hasString(const std::string& result, const std::string& value) {
    return result.find(value) != std::string::npos;
}

class AddressBookLocalImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_engine = aace::core::Engine::create();

        // create the engine configuration objects
        ASSERT_TRUE(m_engine->configure({
            aace::storage::config::StorageConfiguration::createLocalStorageConfig(LOCAL_STORAGE_PATH),
            aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig(LOCAL_SKILL_SERVICE_PATH)
        })) << "Engine configuration failed";

        // auto addressBookHandler = AddressBookHandler::create( );
        // if ( !m_engine->registerPlatformInterface( addressBookHandler ) ) {
        //     ASSERT_TRUE( m_engine->start() ); 
        // }
    }

    void TearDown() override {
        ASSERT_TRUE(m_engine->stop());
        m_engine.reset();
    }

    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_contactAddressBook;
    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_navigationAddressBook;

    std::shared_ptr<aace::core::Engine> m_engine;
};

TEST_F( AddressBookLocalImplTest, create ) {
    class AddressBookHandler : public aace::addressBook::AddressBook {
        public:
        static auto create() -> std::shared_ptr<AddressBookHandler> {
            return std::shared_ptr<AddressBookHandler>(new AddressBookHandler());
        }
        
        bool getEntries( const std::string& addressBookSourceId, std::weak_ptr<IAddressBookEntriesFactory> factory ) override {
            return true;
        }
    };
    auto addressBookHandler = AddressBookHandler::create( );
    if ( m_engine->registerPlatformInterface( addressBookHandler ) ) {
        ASSERT_TRUE( m_engine->start() ); 
    }

    ASSERT_NE(nullptr, m_engine);
}

TEST_F( AddressBookLocalImplTest, AddSimpleContactAddressBook ) {
    class AddressBookHandler : public aace::addressBook::AddressBook {
        public:
        static auto create() -> std::shared_ptr<AddressBookHandler> {
            return std::shared_ptr<AddressBookHandler>(new AddressBookHandler());
        }

        bool getEntries( const std::string& addressBookSourceId, std::weak_ptr<IAddressBookEntriesFactory> factory ) override {
            return false;
        }

    };

    auto addressBookHandler = AddressBookHandler::create( );
    if ( m_engine->registerPlatformInterface( addressBookHandler ) ) {
        ASSERT_TRUE( m_engine->start() ); 
    }

    EXPECT_TRUE( addressBookHandler->addAddressBook("1000", "AutoSDK", AddressBookHandler::AddressBookType::CONTACT));

    auto result = Utilities::httpPostString("http://localhost:8080/subscribe", R"({"id" : "/addressBooks/create", "endpoint" : "/Users/soquetas/Desktop/server/python.socket", "path" : "/echo"})");
    EXPECT_TRUE(Utilities::hasString(result, R"("addressBookSourceId":"1000")")) << "Subscribe failed";
}

} // aace::test::unit
} // aace::test
} // aace
