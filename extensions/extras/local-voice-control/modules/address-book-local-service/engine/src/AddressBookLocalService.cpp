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


#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/AddressBookLocalService/AddressBookLocalService.h"

namespace aace {
namespace engine {
namespace addressBook {
namespace localService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBook.addressBookLocalService");

// register the service
REGISTER_SERVICE(AddressBookLocalService);

AddressBookLocalService::AddressBookLocalService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool AddressBookLocalService::configure( std::shared_ptr<std::istream> configuration ) {
    return true;
}

bool AddressBookLocalService::start() {
    try {
        auto addressBookService = getContext()->getServiceInterface<aace::engine::addressBook::AddressBookServiceInterface>( "aace.addressBook" );
        ThrowIfNull( addressBookService, "addressBookServiceNotFound" );

        auto localSkillService = getContext()->getServiceInterface<aace::engine::localSkillService::LocalSkillServiceEngineService>( "aace.localSkillService" );
        ThrowIfNull( localSkillService, "localSkillServiceNotFound" );

        m_addressBookLocalImpl = aace::engine::addressBook::localService::AddressBookLocalImpl::create( addressBookService, localSkillService  );
        ThrowIfNull( m_addressBookLocalImpl, "createAddressBookLocalImplFailed" );

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"start").d("reason", ex.what()));
        return false;
    }
}

bool AddressBookLocalService::stop() {
    if( m_addressBookLocalImpl != nullptr ) {
        m_addressBookLocalImpl->shutdown();
        m_addressBookLocalImpl.reset();
    } 
    return true;
}

bool AddressBookLocalService::shutdown() {
    if( m_addressBookLocalImpl != nullptr ) {
        m_addressBookLocalImpl->shutdown();
        m_addressBookLocalImpl.reset();
    } 
    return true;
}


} // aace::engine::addressBook::localService
} // aace::engine::addressBook
} // aace::engine
} // aace
