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

#include <typeinfo>
#include <unordered_map>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/AddressBookLocalService/AddressBookLocalImpl.h>

namespace aace {
namespace engine {
namespace addressBook {
namespace localService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.addressBook.addressBookLocalImpl");

AddressBookLocalImpl::AddressBookLocalImpl() : 
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

std::shared_ptr<AddressBookLocalImpl> AddressBookLocalImpl::create (
        std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
        std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceEngineService> localSkillService ) {
    try {
        auto addressBookLocalImpl = std::shared_ptr<AddressBookLocalImpl>( new AddressBookLocalImpl() );
        ThrowIfNot( addressBookLocalImpl->initialize( addressBookService, localSkillService ), "initializeAddressBookLocalImplFailed" );

        return addressBookLocalImpl;
    } catch( std::exception &ex ) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

bool AddressBookLocalImpl::initialize( std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
    std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceEngineService> localSkillService ) {
    try {
        m_addressBookService = addressBookService;
        m_localSkillService = localSkillService;

        m_addressBookService->addObserver( shared_from_this() );

        m_localSkillService->registerHandler("/addressBooks",
            [this]( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) -> bool {
                return handleGetAddressBooksRequest( request, response );
            }
        );

        m_localSkillService->registerHandler("/addressBooks/getEntries",
            [this]( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) -> bool {
                return handleGetEntriesRequest( request, response );
            }
        );

        m_localSkillService->registerPublishHandler("/addressBooks/create",
            [this]( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) -> bool {
                return handleGetAddressBooksRequest( request, response );
            }
        );

        m_localSkillService->registerPublishHandler("/addressBooks/delete" );

        return true;
    } catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

void AddressBookLocalImpl::doShutdown() {

}

bool AddressBookLocalImpl::addressBookAdded( std::shared_ptr<AddressBookEntity> addressBook ) {
    std::string sourceAddressBookId = "INVALID";
    try {
        ThrowIfNull( addressBook, "invalidAddressBook" );

        std::lock_guard<std::mutex> guard( m_mutex );

        sourceAddressBookId = addressBook->getSourceId();
        ThrowIfNot( m_addressBooks.find( sourceAddressBookId ) == m_addressBooks.end(), "sourceAddressBookIdFound");

        m_addressBooks[ sourceAddressBookId ] = addressBook;

        std::shared_ptr<rapidjson::Document> message = std::make_shared<rapidjson::Document>();
        auto& allocator = message->GetAllocator();
        message->SetObject();
        message->AddMember( "addressBookSourceId", sourceAddressBookId, allocator );
        message->AddMember( "addressBookName", addressBook->getName(), allocator );
        message->AddMember( "addressBookType", toJSONAddressBookType(addressBook->getType()), allocator );

        m_localSkillService->publishMessage( "/addressBooks/create", message );
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"addAddressBook").d("sourceAddressBookId", sourceAddressBookId).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookLocalImpl::addressBookRemoved( std::shared_ptr<AddressBookEntity> addressBook ) {
    std::string sourceAddressBookId = "INVALID";
    try {
        ThrowIfNull( addressBook, "invalidAddressBook" );

        std::lock_guard<std::mutex> guard( m_mutex );

        sourceAddressBookId = addressBook->getSourceId();
        auto it = m_addressBooks.find( sourceAddressBookId );
        ThrowIf( it == m_addressBooks.end(), "addressBookIdNotFound");

        auto addressBook = m_addressBooks[ sourceAddressBookId ];
        m_addressBooks.erase( it );

        std::shared_ptr<rapidjson::Document> message = std::make_shared<rapidjson::Document>();
        auto& allocator = message->GetAllocator();
        message->SetObject();
        message->AddMember( "addressBookSourceId", sourceAddressBookId, allocator );

        m_localSkillService->publishMessage( "/addressBooks/delete", message );

        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"removeAddressBook").d("sourceAddressBookId", sourceAddressBookId).d("reason", ex.what()));
        return false;
    }
}

bool AddressBookLocalImpl::handleGetAddressBooksRequest( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) {
    try {
        ThrowIfNull( response, "responseIsNull" );

        auto& allocator = response->GetAllocator();
        response->SetObject();
        rapidjson::Value addressBooks(rapidjson::kArrayType);

        for ( auto& pair : m_addressBooks ) {
            auto addressBook = pair.second;
            rapidjson::Value value(rapidjson::kObjectType);

            value.AddMember( "addressBookSourceId", addressBook->getSourceId(), allocator );
            value.AddMember( "addressBookName", addressBook->getName(), allocator );
            value.AddMember( "addressBookType", toJSONAddressBookType(addressBook->getType()), allocator );

            addressBooks.PushBack( value, allocator );
        }
        response->AddMember( "addressBooks", addressBooks, allocator );

        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleGetAddressBooksRequest").d("reason", ex.what()));
        return false;
    }
}

std::string AddressBookLocalImpl::toJSONAddressBookType(AddressBookType type) {
    switch ( type ) {
        case AddressBookType::CONTACT:
            return "automotive";
        case AddressBookType::NAVIGATION:
            return "automotiveNavigation";
    }
    return "error";
}

class PhoneContactFactory : public aace::addressBook::AddressBook::IAddressBookEntriesFactory {
public:
    PhoneContactFactory( std::shared_ptr<AddressBookEntity> addressBook, std::shared_ptr<rapidjson::Document> response ) : m_addressBookEntity( addressBook ), m_response( response ) {
        auto& allocator = response->GetAllocator();
        response->SetObject();
        response->AddMember( "addressBookSourceId", addressBook->getSourceId(), allocator );
        rapidjson::Value entries(rapidjson::kArrayType);
        response->AddMember( "entries", entries, allocator );
    }

    rapidjson::Value& getEntryData( const std::string& entryId ) {
        auto& response = *m_response;
        auto& entries = response["entries"];
        auto it = m_ids.find( entryId );
        
        if( it == m_ids.end() ) {
            auto& allocator = response.GetAllocator();
            rapidjson::Value entry(rapidjson::kObjectType);

            entry.AddMember( "entrySourceId", entryId, allocator );
            rapidjson::Value data(rapidjson::kObjectType);

            entry.AddMember( "data", data, allocator );

            entries.PushBack( entry, allocator );

            // For "m_ids[entryId] = m_ids.size();" on Ubuntu platform, [] increment the m_ids size before assignment which
            // causes incorrect indexes for later usage.
            auto index = m_ids.size();
            m_ids[entryId] = index;
        }
        auto index = m_ids[entryId];

        return entries[index]["data"];
    }

    bool addName( const std::string& entryId, const std::string& name ) {
        auto& allocator = m_response->GetAllocator();
        auto& data = getEntryData( entryId );

        rapidjson::Value nameValue(rapidjson::kObjectType);

        if( data.HasMember("name") ) {
            return false;
        }

        if( !name.empty() ) nameValue.AddMember( "firstName", name, allocator );
 
        data.AddMember( "name", nameValue, allocator );

        return true;
    }

    bool addName( const std::string& entryId, const std::string& firstName, const std::string& lastName ) {
        auto& allocator = m_response->GetAllocator();
        auto& data = getEntryData( entryId );

        rapidjson::Value name(rapidjson::kObjectType);

        if( data.HasMember("name") ) {
            return false;
        }

        if( !firstName.empty() ) name.AddMember( "firstName", firstName, allocator );
        if( !lastName.empty() ) name.AddMember( "lastName", lastName, allocator );

        data.AddMember( "name", name, allocator );

        return true;
    }

    bool addName( const std::string& entryId, const std::string& firstName, const std::string& lastName, const std::string& nickName ) {
        auto& allocator = m_response->GetAllocator();
        auto& data = getEntryData( entryId );

        rapidjson::Value name(rapidjson::kObjectType);

        if( data.HasMember("name") ) {
            return false;
        }

        if( !firstName.empty() ) name.AddMember( "firstName", firstName, allocator );
        if( !lastName.empty() ) name.AddMember( "lastName", lastName, allocator );
        if( !nickName.empty() ) name.AddMember( "nickName", nickName, allocator );

        data.AddMember( "name", name, allocator );

        return true;
    }

    bool addPhone( const std::string& entryId, const std::string& type, const std::string& number ) {
        if( m_addressBookEntity->isAddressTypeSupported( AddressBookEntity::AddressType::PHONE ) ) {
            auto& allocator = m_response->GetAllocator();
            auto& data = getEntryData( entryId );

            if( !data.HasMember("addresses") ) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember( "addresses", addresses, allocator );
            }

            auto& addresses = data["addresses"];
            rapidjson::Value address(rapidjson::kObjectType);
            address.AddMember( "addressType", "phoneNumber", allocator );

            if( !type.empty() ) address.AddMember( "rawType", type, allocator );
            if( !number.empty() ) address.AddMember( "value", number, allocator );

            addresses.PushBack( address, allocator );
            return true;
        } else {
            AACE_WARN(LX(TAG,"addPhone").d("entryId", entryId).m("addressTypeNotSupported"));
            return false;
        }
        
    }

    bool addPostalAddress( const std::string& entryId,
            const std::string& type,
            const std::string& addressLine1,
            const std::string& addressLine2,
            const std::string& addressLine3,
            const std::string& city,
            const std::string& stateOrRegion,
            const std::string& districtOrCounty,
            const std::string& postalCode,
            const std::string& countryCode,
            float latitudeInDegrees,
            float longitudeInDegrees,
            float accuracyInMeters ) {
        if( m_addressBookEntity->isAddressTypeSupported( AddressBookEntity::AddressType::POSTALADDRESS ) ) {
            auto& allocator = m_response->GetAllocator();
            auto& data = getEntryData( entryId );

            if( !data.HasMember("addresses") ) {
                auto addresses(rapidjson::kArrayType);
                data.AddMember( "addresses", addresses, allocator );
            }

            auto& addresses = data["addresses"];
            rapidjson::Value address(rapidjson::kObjectType);
            address.AddMember( "addressType", "PostalAddress", allocator );
            if( !type.empty() ) address.AddMember( "rawType", type, allocator );

            rapidjson::Value postalAddressValue(rapidjson::kObjectType);

            if( !addressLine1.empty() ) postalAddressValue.AddMember( "addressLine1", addressLine1, allocator );
            if( !addressLine2.empty() ) postalAddressValue.AddMember( "addressLine2", addressLine2, allocator );
            if( !addressLine3.empty() ) postalAddressValue.AddMember( "addressLine3", addressLine3, allocator );
            if( !city.empty() ) postalAddressValue.AddMember( "city", city, allocator );
            if( !stateOrRegion.empty() ) postalAddressValue.AddMember( "stateOrRegion", stateOrRegion, allocator );
            if( !districtOrCounty.empty() ) postalAddressValue.AddMember( "districtOrCounty", districtOrCounty, allocator );
            if( !postalCode.empty() ) postalAddressValue.AddMember( "postalCode", postalCode, allocator );
            if( !countryCode.empty() ) postalAddressValue.AddMember( "countryCode", countryCode, allocator );

            rapidjson::Value coordinate(rapidjson::kObjectType);
            coordinate.AddMember( "latitudeInDegrees", latitudeInDegrees, allocator );
            coordinate.AddMember( "longitudeInDegrees", longitudeInDegrees, allocator );
            coordinate.AddMember( "accuracyInMeters", accuracyInMeters, allocator );
            postalAddressValue.AddMember( "coordinate", coordinate, allocator );

            address.AddMember( "postalAddressValue", postalAddressValue, allocator );
            addresses.PushBack( address, allocator );
            return true;
        } else {
            AACE_WARN(LX(TAG,"addPostalAddress").d("entryId", entryId).m("addressTypeNotSupported"));
            return false;
        }
        
    }

private:
    std::shared_ptr<AddressBookEntity> m_addressBookEntity;
    std::shared_ptr<rapidjson::Document> m_response;
    std::unordered_map<std::string, rapidjson::SizeType> m_ids;
};

bool AddressBookLocalImpl::handleGetEntriesRequest( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) {
    try {
        ThrowIfNull( request, "requestIsNull" );
        ThrowIfNull( response, "responseIsNull" );

        rapidjson::Value* value = GetValueByPointer(*request, "/addressBookSourceId");
        ThrowIfNot( value && value->IsString(), "cannotFindAddressBookId" );

        std::string addressBookId = value->GetString();
        ThrowIf( m_addressBooks.find( addressBookId ) == m_addressBooks.end(), "addressBookSourceIdNotFound");

        auto addressBookEntity = m_addressBooks[ addressBookId ];
        auto factory = std::make_shared<PhoneContactFactory>( addressBookEntity, response );

        AACE_INFO(LX(TAG,"handleGetAddressBooksRequest").m("GettingEntries").d("addressBookId",addressBookId));

        ThrowIfNot( m_addressBookService->getEntries( addressBookId, factory ), "getEntriesFailed" );

        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleGetAddressBooksRequest").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::addressBook::localService
} // aace::engine::addressBook
} // aace::engine
} // aace
