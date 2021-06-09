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

#ifndef AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_IMPL_H
#define AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_IMPL_H

#include <unordered_map>

#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h>
#include <AACE/Engine/AddressBook/AddressBookEngineService.h>
#include <AACE/Engine/AddressBook/AddressBookServiceInterface.h>

namespace aace {
namespace engine {
namespace addressBook {
namespace localService {

using AddressBookEntity = aace::engine::addressBook::AddressBookEntity;
using AddressBookType = aace::addressBook::AddressBook::AddressBookType;

class AddressBookLocalImpl :
    public aace::engine::addressBook::AddressBookObserver,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<AddressBookLocalImpl> {
private:
    AddressBookLocalImpl();

    bool initialize( std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
            std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceEngineService> localSkillService );

protected:
    // RequiresShutdown
    void doShutdown() override;

public:
    static std::shared_ptr<AddressBookLocalImpl> create( std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
            std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceEngineService> localSkillService );

    // AddressBookObserver
    bool addressBookAdded( std::shared_ptr<AddressBookEntity> addressBookEntity ) override;
    bool addressBookRemoved( std::shared_ptr<AddressBookEntity> addressBookEntity ) override;

private:
    bool handleGetAddressBooksRequest( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response );
    bool handleGetEntriesRequest( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response );
    std::string toJSONAddressBookType(AddressBookType type);

private:
    std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> m_addressBookService;
    std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceEngineService> m_localSkillService;

    std::mutex m_mutex;
    std::unordered_map<std::string,std::shared_ptr<AddressBookEntity>> m_addressBooks;
};

} // aace::engine::addressBook::localService
} // aace::engine::addressBook
} // aace::engine
} // aace

#endif // AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_IMPL_H
