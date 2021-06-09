/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <gmock/gmock-more-actions.h>
#include <string>
#include <rapidjson/document.h>
#include "rapidjson/pointer.h"

#include <AVSCommon/Utils/WaitEvent.h>
#include <AACE/AddressBook/AddressBook.h>
#include <AACE/Engine/AddressBook/AddressBookObserver.h>
#include <AACE/Engine/AddressBookLocalService/AddressBookLocalImpl.h>

namespace aace {
namespace test {
namespace unit {

using namespace aace::engine::localSkillService;

/// Plenty of timeout to wait for HTTP timeouts
static std::chrono::seconds TIMEOUT(10);

static const std::string ADDRESS_BOOK_PATH = "/addressBooks";
static const std::string ADDRESS_BOOK_CREATE_PATH = "/addressBooks/create";
static const std::string ADDRESS_BOOK_DELETE_PATH = "/addressBooks/delete";
static const std::string REGISTER_GETENTRIES_HANDLER_PATH = "/addressBooks/getEntries";

class MockAddressBookServiceInterface : public aace::engine::addressBook::AddressBookServiceInterface {
public:
    MOCK_METHOD1(addObserver, void(std::shared_ptr<aace::engine::addressBook::AddressBookObserver> observer));
    MOCK_METHOD1(removeObserver, void(std::shared_ptr<aace::engine::addressBook::AddressBookObserver> observer));
    MOCK_METHOD2(
        getEntries,
        bool(const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory));
};

class MockLocalSkillServiceInterface : public aace::engine::localSkillService::LocalSkillServiceInterface {
public:
    MOCK_METHOD2(registerHandler, void(const std::string& path, RequestHandler handler));
    MOCK_METHOD4(registerPublishHandler, bool(const std::string& id, RequestHandler subscribeHandler, PublishRequestHandler requestHandler, PublishResponseHandler responseHandler));
    MOCK_METHOD2(publishMessage, bool(const std::string& id, std::shared_ptr<rapidjson::Document> message));
    MOCK_METHOD0(getLSSSocketPath, std::string());
};

class AddressBookLocalImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockLocalSkillServiceInterface = std::make_shared<testing::StrictMock<MockLocalSkillServiceInterface>>();
        m_mockAddressBookServiceInterface = std::make_shared<testing::StrictMock<MockAddressBookServiceInterface>>();
        m_mockContactAddressBook = std::make_shared<aace::engine::addressBook::AddressBookEntity>(
            "1000", "TestAddressBook", aace::engine::addressBook::AddressBookType::CONTACT);
        m_mockNavigationAddressBook = std::make_shared<aace::engine::addressBook::AddressBookEntity>(
            "1001", "TestAddressBook", aace::engine::addressBook::AddressBookType::NAVIGATION);

        EXPECT_CALL(*m_mockAddressBookServiceInterface, addObserver(testing::_)).WillOnce(testing::Return());
        EXPECT_CALL(*m_mockLocalSkillServiceInterface, registerHandler(ADDRESS_BOOK_PATH, testing::_)).Times(1).WillOnce(testing::SaveArg<1>(&mGetAddressBookHandler));
        EXPECT_CALL(*m_mockLocalSkillServiceInterface, registerHandler(REGISTER_GETENTRIES_HANDLER_PATH, testing::_)).Times(1).WillOnce(testing::SaveArg<1>(&mGetEntriesHandler));
        EXPECT_CALL(*m_mockLocalSkillServiceInterface, registerPublishHandler(ADDRESS_BOOK_CREATE_PATH, testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::SaveArg<3>(&mAddressBookCreateHandler));
        EXPECT_CALL(*m_mockLocalSkillServiceInterface, registerPublishHandler(ADDRESS_BOOK_DELETE_PATH, testing::_, testing::_, testing::_)).WillOnce(testing::Return(true));
        m_addressBookLocalImpl = aace::engine::addressBook::localService::AddressBookLocalImpl::create(
            m_mockAddressBookServiceInterface,
            m_mockLocalSkillServiceInterface);
    }

    void TearDown() override {
        m_addressBookLocalImpl->shutdown();
    }
    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_mockContactAddressBook;
    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_mockNavigationAddressBook;
    RequestHandler mGetAddressBookHandler;
    RequestHandler mGetEntriesHandler;
    PublishResponseHandler mAddressBookCreateHandler;

    std::shared_ptr<testing::StrictMock<MockAddressBookServiceInterface>> m_mockAddressBookServiceInterface;
    std::shared_ptr<testing::StrictMock<MockLocalSkillServiceInterface>> m_mockLocalSkillServiceInterface;
    std::shared_ptr<aace::engine::addressBook::localService::AddressBookLocalImpl> m_addressBookLocalImpl;
};

TEST_F(AddressBookLocalImplTest, create) {
    ASSERT_NE(nullptr, m_addressBookLocalImpl);
}

TEST_F(AddressBookLocalImplTest, AddSingleContactAddressBookAndExpectGetEntreisCalled) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000", testing::_))
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> bool {
          waitEvent.wakeUp();
          return true;
        }));
    EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_CREATE_PATH, testing::_))
        .WillOnce(testing::InvokeWithoutArgs([this]() -> bool {
          std::shared_ptr<rapidjson::Document> request = std::make_shared<rapidjson::Document>();
          std::shared_ptr<rapidjson::Document> response = std::make_shared<rapidjson::Document>();

          rapidjson::Pointer("/addressBookSourceId").Set(*request, "1000");
          rapidjson::Pointer("/addressBookName").Set(*request, "TestAddressBook");
          rapidjson::Pointer("/addressBookType").Set(*request, "automotive");
          return mGetEntriesHandler(request, response);
        }));
    EXPECT_TRUE(m_addressBookLocalImpl->addressBookAdded(m_mockContactAddressBook));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
}

TEST_F(AddressBookLocalImplTest, AddSingleNavigationAddressBookAndExpectGetEntreisCalled) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent;

    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1001", testing::_))
        .WillOnce(testing::InvokeWithoutArgs([&waitEvent]() -> bool {
          waitEvent.wakeUp();
          return true;
        }));
    EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_CREATE_PATH, testing::_))
        .WillOnce(testing::InvokeWithoutArgs([this]() -> bool {
          std::shared_ptr<rapidjson::Document> request = std::make_shared<rapidjson::Document>();
          std::shared_ptr<rapidjson::Document> response = std::make_shared<rapidjson::Document>();

          rapidjson::Pointer("/addressBookSourceId").Set(*request, "1001");
          rapidjson::Pointer("/addressBookName").Set(*request, "TestAddressBook");
          rapidjson::Pointer("/addressBookType").Set(*request, "automotiveNavigation");
          return mGetEntriesHandler(request, response);
        }));
    

    EXPECT_TRUE(m_addressBookLocalImpl->addressBookAdded(m_mockNavigationAddressBook));
    EXPECT_TRUE(waitEvent.wait(TIMEOUT));
}

TEST_F(AddressBookLocalImplTest, CheckForSequneceCallWhenAddingAndRemovingTwoAddressBook) {
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent1;
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent2;
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent3;
    alexaClientSDK::avsCommon::utils::WaitEvent waitEvent4;
    {
        ::testing::InSequence dummy;
        EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_CREATE_PATH, testing::_))
            .WillOnce(testing::Return(true));

        EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_DELETE_PATH, testing::_))
            .WillOnce(testing::Return(true));

        EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_CREATE_PATH, testing::_))
            .WillOnce(testing::Return(true));

        EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_DELETE_PATH, testing::_))
            .WillOnce(testing::Return(true));
    }


    EXPECT_TRUE(m_addressBookLocalImpl->addressBookAdded(m_mockContactAddressBook))
                    << "Contact AddressBook cannot be added ";

    EXPECT_TRUE(m_addressBookLocalImpl->addressBookRemoved(m_mockContactAddressBook))
                    << "Contact AddressBook cannot be removed ";

    EXPECT_TRUE(m_addressBookLocalImpl->addressBookAdded(m_mockNavigationAddressBook))
                    << "Navigation AddressBook cannot be added ";

    EXPECT_TRUE(m_addressBookLocalImpl->addressBookRemoved(m_mockNavigationAddressBook))
                    << "Navigation AddressBook cannot be removed ";
}

TEST_F(AddressBookLocalImplTest, TestInvalidAddressBookEntryShouldFail) {
    std::string id;
    std::string name;
    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> mockContactAddressBook =
        std::make_shared<aace::engine::addressBook::AddressBookEntity>(id, name, aace::engine::addressBook::AddressBookType::CONTACT);

    EXPECT_CALL(*m_mockLocalSkillServiceInterface, publishMessage(ADDRESS_BOOK_CREATE_PATH, testing::_)).WillOnce(testing::Return(false));
    m_addressBookLocalImpl->addressBookAdded(mockContactAddressBook);
}

} // aace::test::unit
} // aace::test
} // aace