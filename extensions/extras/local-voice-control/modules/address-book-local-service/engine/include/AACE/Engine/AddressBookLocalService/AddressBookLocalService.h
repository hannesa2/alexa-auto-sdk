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

#ifndef AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_SERVICE_H
#define AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_SERVICE_H

#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/AddressBook/AddressBookEngineService.h>
#include <AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h>
#include <AACE/Engine/AddressBookLocalService/AddressBookLocalImpl.h>

namespace aace {
namespace engine {
namespace addressBook {
namespace localService {

using AddressBookEntity = aace::engine::addressBook::AddressBookEntity;

class AddressBookLocalService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.addressBookLocalService",VERSION("1.0"),DEPENDS(aace::engine::addressBook::AddressBookEngineService),DEPENDS(aace::engine::localSkillService::LocalSkillServiceEngineService))

private:
    AddressBookLocalService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~AddressBookLocalService() = default;

protected:
    // EngineService
    bool configure( const std::shared_ptr<std::istream> configuration ) override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;

private:
    std::shared_ptr<AddressBookLocalImpl> m_addressBookLocalImpl;
};

} // aace::engine::addressBook::localService
} // aace::engine::addressBook
} // aace::engine
} // aace

#endif // AACE_ENGINE_ADDRESS_BOOK_LOCAL_SERVICE_ADDRESS_BOOK_LOCAL_SERVICE_H
