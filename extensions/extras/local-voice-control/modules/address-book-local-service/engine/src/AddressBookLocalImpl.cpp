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

// JSON for Modern C++
#include <nlohmann/json.hpp>

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

/// Max allowed phonenumbers per entry
static const int MAX_ALLOWED_ADDRESSES_PER_ENTRY = 30;

/// Max allowed characters
static const int MAX_ALLOWED_CHARACTERS = 1000;

/// Max allowed label characters
static const int MAX_ALLOWED_LABEL_CHARACTERS = 100;

/// Max allowed phone number characters
static const int MAX_ALLOWED_PHONE_NUMBER_CHARACTERS = 100;

/// Max allowed EntryId size
static const int MAX_ALLOWED_ENTRY_ID_SIZE = 40;

using json = nlohmann::json;

AddressBookLocalImpl::AddressBookLocalImpl() : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

std::shared_ptr<AddressBookLocalImpl> AddressBookLocalImpl::create (
        std::shared_ptr<aace::engine::addressBook::AddressBookServiceInterface> addressBookService,
        std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceInterface> localSkillService ) {
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
    std::shared_ptr<aace::engine::localSkillService::LocalSkillServiceInterface> localSkillService ) {
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
    PhoneContactFactory(std::shared_ptr<AddressBookEntity> addressBook, std::shared_ptr<rapidjson::Document> response) :
            m_addressBookEntity(addressBook), m_response(response) {
        auto& allocator = response->GetAllocator();
        response->SetObject();
        response->AddMember( "addressBookSourceId", addressBook->getSourceId(), allocator );
        rapidjson::Value entries(rapidjson::kArrayType);
        response->AddMember( "entries", entries, allocator );
    }

    bool isEntryPresent(const std::string& entryId) {
        auto it = m_ids.find(entryId);
        if (it == m_ids.end()) {
            return false;
        }
        return true;
    }

    rapidjson::Value& getEntryDataNode(const std::string& entryId) {
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

    bool addEntry(const std::string& payload) {
        bool success = true;
        try {
            ThrowIf(payload.empty(), "payloadEmpty");
            auto entryPayload = json::parse(payload);

            ThrowIfNot(
                entryPayload.contains("entryId") && entryPayload["entryId"].is_string(), "entryIdMissingOrNotString");
            ThrowIf(entryPayload["entryId"].empty(), "entryIdEmpty");

            std::string entryId = entryPayload["entryId"];
            AACE_DEBUG(LX(TAG).d("entryId", entryId));

            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdSizeExceedsMaxSize");
            ThrowIfNot(entryPayload.contains("name") && entryPayload["name"].is_object(), "nameMissingOrInvalid");

            auto& nameNode = entryPayload["name"];

            // clang-format off
            // Sanitize name  field types
            ThrowIfNot((nameNode.contains("firstName") ? nameNode["firstName"].is_string() : true), "firstNameInvalid");
            ThrowIfNot((nameNode.contains("lastName") ? nameNode["lastName"].is_string() : true), "lastNameInvalid");
            ThrowIfNot((nameNode.contains("nickName") ? nameNode["nickName"].is_string() : true), "nickNameInvalid");
            ThrowIfNot((nameNode.contains("phoneticFirstName") ? nameNode["phoneticFirstName"].is_string() : true), "phoneticFirstNameInvalid");
            ThrowIfNot((nameNode.contains("phoneticLastName") ? nameNode["phoneticLastName"].is_string() : true), "phoneticLastNameInvalid");
            // clang-format on

            std::string firstName = nameNode.value("firstName", "");
            std::string lastName = nameNode.value("lastName", "");
            std::string nickName = nameNode.value("nickName", "");
            std::string phoneticFirstName = nameNode.value("phoneticFirstName", "");
            std::string phoneticLastName = nameNode.value("phoneticLastName", "");

            // Sanitize field size
            auto totalSize = firstName.size() + lastName.size() + nickName.size() + phoneticFirstName.size() +
                                phoneticLastName.size();
            if (totalSize > MAX_ALLOWED_CHARACTERS) {
                AACE_ERROR(LX(TAG)
                                .m("nameTotalLengthExceedsMaxSize")
                                .d("entryId", entryId)
                                .d("size", totalSize)
                                .d("maxSize", MAX_ALLOWED_CHARACTERS));
                return false;
            }

            ThrowIf(isEntryPresent(entryId), "entryAlreadyExists");

            auto& allocator = m_response->GetAllocator();
            auto& data = getEntryDataNode(entryId);

            rapidjson::Value name(rapidjson::kObjectType);

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);
            if (!nickName.empty()) name.AddMember("nickName", nickName, allocator);
            // phoneticFirstName and phoneticLastName not supported in LVC

            data.AddMember("name", name, allocator);

            if (entryPayload.contains("phoneNumbers") && !entryPayload["phoneNumbers"].empty()) {
                // Consider phone numbers only when the address book type is CONTACT
                if (m_addressBookEntity->getType() == AddressBookType::CONTACT) {
                    if (!entryPayload["phoneNumbers"].is_array()) {
                        Throw("phoneNumbersFieldIsNotAnArrary");
                    }

                    if (!data.HasMember("addresses")) {
                        auto addresses(rapidjson::kArrayType);
                        data.AddMember("addresses", addresses, allocator);
                    }

                    auto& addresses = data["addresses"];

                    int counter = 0;
                    for (auto& phoneNumber : entryPayload["phoneNumbers"]) {
                        if (++counter > MAX_ALLOWED_ADDRESSES_PER_ENTRY) {
                            AACE_WARN(LX(TAG).m("maxAllowedPhoneNumberEntriesReached"));
                            success = false;
                            break;  // bail out
                        }

                        // clang-format off
                        // Sanitize phone number field types
                        ThrowIfNot((phoneNumber.contains("label") ? phoneNumber["label"].is_string() : true), "phoneNumberLabelInvalid");
                        ThrowIfNot((phoneNumber.contains("number") ? phoneNumber["number"].is_string() : true), "phoneNumberNumberInvalid");
                        // clang-format on

                        // Sanitize phone number field sizes
                        std::string label = phoneNumber.value("label", "");
                        std::string number = phoneNumber.value("number", "");

                        totalSize = label.size() + number.size();
                        if (totalSize > MAX_ALLOWED_LABEL_CHARACTERS) {
                            AACE_WARN(LX(TAG)
                                            .m("phoneNumberFieldExceedsMaxSize")
                                            .d("label", label)
                                            .d("number", number)
                                            .d("size", totalSize)
                                            .d("maxSize", MAX_ALLOWED_CHARACTERS));
                            success = false;
                            continue;
                        }

                        rapidjson::Value address(rapidjson::kObjectType);
                        address.AddMember("addressType", "phoneNumber", allocator);
                        if (!label.empty()) address.AddMember("rawType", label, allocator);
                        if (!number.empty()) address.AddMember("value", number, allocator);

                        addresses.PushBack(address, allocator);
                    }
                } else {
                    AACE_WARN(LX(TAG).m("phoneNumbersNotSupportedInNavigationType"));
                    success = false;
                }
            }

            if (entryPayload.contains("postalAddresses") && !entryPayload["postalAddresses"].empty()) {
                // Consider postal addresses  only when the address book type is NAVIGATION
                if (m_addressBookEntity->getType() == AddressBookType::NAVIGATION) {
                    if (!entryPayload["postalAddresses"].is_array()) {
                        Throw("postalAddressesFieldIsNotAnArrary");
                    }

                    if (!data.HasMember("addresses")) {
                        auto addresses(rapidjson::kArrayType);
                        data.AddMember("addresses", addresses, allocator);
                    }

                    auto& addresses = data["addresses"];

                    int counter = 0;
                    for (auto& postalAddress : entryPayload["postalAddresses"]) {
                        if (++counter > MAX_ALLOWED_ADDRESSES_PER_ENTRY) {
                            AACE_WARN(LX(TAG).m("maxAllowedPostalAddressEntriesReached"));
                            success = false;
                            break;  // bail out
                        }

                        // clang-format off
                        // Sanitize postal address fields types
                        ThrowIfNot((postalAddress.contains("label") ? postalAddress["label"].is_string() : true), "postalAddressLabelInvalid");
                        ThrowIfNot((postalAddress.contains("addressLine1") ? postalAddress["addressLine1"].is_string() : true), "postalAddressAddressLine1Invalid");
                        ThrowIfNot((postalAddress.contains("addressLine2") ? postalAddress["addressLine2"].is_string() : true), "postalAddressAddressLine2Invalid");
                        ThrowIfNot((postalAddress.contains("city") ? postalAddress["city"].is_string() : true), "postalAddressCityInvalid");
                        ThrowIfNot((postalAddress.contains("stateOrRegion") ? postalAddress["stateOrRegion"].is_string() : true), "postalAddressStateOrRegionInvalid");
                        ThrowIfNot((postalAddress.contains("districtOrCounty") ? postalAddress["districtOrCounty"].is_string() : true), "postalAddressDistrictOrCountyInvalid");
                        ThrowIfNot((postalAddress.contains("postalCode") ? postalAddress["postalCode"].is_string() : true), "postalAddressPostalCodeInvalid");
                        ThrowIfNot((postalAddress.contains("countryCode") ? postalAddress["countryCode"].is_string() : true), "postalAddressCountryCodeInvalid");
                        ThrowIfNot(postalAddress.contains("latitudeInDegrees") && postalAddress["latitudeInDegrees"].is_number(), "postalAddressLatitudeInDegreesNotPresetOrInvalid");
                        ThrowIfNot(postalAddress.contains("longitudeInDegrees") && postalAddress["longitudeInDegrees"].is_number(), "postalAddressLongitudeInDegreesNotPresetOrInvalid");
                        ThrowIfNot((postalAddress.contains("accuracyInMeters") ? postalAddress["accuracyInMeters"].is_number() : true), "postalAddressAccuracyInMetersInvalid");
                        // clang-format on

                        std::string label = postalAddress.value("label", "");
                        std::string addressLine1 = postalAddress.value("addressLine1", "");
                        std::string addressLine2 = postalAddress.value("addressLine2", "");
                        std::string addressLine3 = postalAddress.value("addressLine3", "");
                        std::string city = postalAddress.value("city", "");
                        std::string stateOrRegion = postalAddress.value("stateOrRegion", "");
                        std::string districtOrCounty = postalAddress.value("districtOrCounty", "");
                        std::string postalCode = postalAddress.value("postalCode", "");
                        std::string countryCode = postalAddress.value("countryCode", "");
                        float latitudeInDegrees = postalAddress.value("latitudeInDegrees", 0.0f);
                        float longitudeInDegrees = postalAddress.value("longitudeInDegrees", 0.0f);
                        float accuracyInMeters = postalAddress.value("accuracyInMeters", 0.0f);

                        // Sanitize postal address fields sizes
                        if (label.size() > MAX_ALLOWED_LABEL_CHARACTERS) {
                            AACE_WARN(LX(TAG).m("labelExceedsMaxSize").d("label", label));
                            success = false;
                            continue;
                        }
                        int totalSize = addressLine1.size() + addressLine2.size() + addressLine3.size() + city.size() +
                                        stateOrRegion.size() + districtOrCounty.size() + postalCode.size() +
                                        countryCode.size();

                        if (totalSize > MAX_ALLOWED_CHARACTERS) {
                            AACE_WARN(LX(TAG)
                                            .m("postalAddressExceedsMaxCharacterSize")
                                            .d("entryId", entryId)
                                            .d("size", totalSize)
                                            .d("maxSize", MAX_ALLOWED_CHARACTERS));
                            success = false;
                            continue;
                        }

                        if (!(latitudeInDegrees >= -90 && latitudeInDegrees <= 90)) {
                            AACE_WARN(LX(TAG).m("latitudeInDegreesInvalid").d("latitudeInDegrees", latitudeInDegrees));
                            success = false;
                            continue;
                        }

                        if (!(longitudeInDegrees >= -180 && longitudeInDegrees <= 180)) {
                            AACE_WARN(
                                LX(TAG).m("longitudeInDegreesInvalid").d("longitudeInDegrees", longitudeInDegrees));
                            success = false;
                            continue;
                        }

                        if (accuracyInMeters < 0) {
                            AACE_WARN(LX(TAG).m("accuracyInMetersInvalid").d("accuracyInMeters", accuracyInMeters));
                            success = false;
                            continue;
                        }

                        rapidjson::Value address(rapidjson::kObjectType);
                        address.AddMember("addressType", "PostalAddress", allocator);
                        if (!label.empty()) address.AddMember("rawType", label, allocator);

                        rapidjson::Value postalAddressValue(rapidjson::kObjectType);
                        if (!addressLine1.empty())
                            postalAddressValue.AddMember("addressLine1", addressLine1, allocator);
                        if (!addressLine2.empty())
                            postalAddressValue.AddMember("addressLine2", addressLine2, allocator);
                        if (!addressLine3.empty())
                            postalAddressValue.AddMember("addressLine3", addressLine3, allocator);
                        if (!city.empty()) postalAddressValue.AddMember("city", city, allocator);
                        if (!stateOrRegion.empty())
                            postalAddressValue.AddMember("stateOrRegion", stateOrRegion, allocator);
                        if (!districtOrCounty.empty())
                            postalAddressValue.AddMember("districtOrCounty", districtOrCounty, allocator);
                        if (!postalCode.empty()) postalAddressValue.AddMember("postalCode", postalCode, allocator);
                        if (!countryCode.empty()) postalAddressValue.AddMember("countryCode", countryCode, allocator);

                        rapidjson::Value coordinate(rapidjson::kObjectType);
                        coordinate.AddMember("latitudeInDegrees", latitudeInDegrees, allocator);
                        coordinate.AddMember("longitudeInDegrees", longitudeInDegrees, allocator);
                        if (accuracyInMeters > 0) coordinate.AddMember("accuracyInMeters", accuracyInMeters, allocator);
                        postalAddressValue.AddMember("coordinate", coordinate, allocator);

                        address.AddMember("postalAddressValue", postalAddressValue, allocator);

                        addresses.PushBack(address, allocator);
                    }
                } else {
                    AACE_WARN(LX(TAG).m("postalAddressesNotSupportedInContactType"));
                    success = false;
                }
            }
            return success;
        } catch (json::parse_error& ex) {
            AACE_ERROR(LX(TAG).m("payLoadParseError").d("exception", ex.what()));
            return false;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(const std::string& entryId, const std::string& name) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(name.size() > MAX_ALLOWED_CHARACTERS, "nameExceedsMaxSize");

            ThrowIfNot(addName(entryId, name, "", ""), "addNameFailed");
            return true;

        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(const std::string& entryId, const std::string& firstName, const std::string& lastName) {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameExceedsMaxSize");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameExceedsMaxSize");
            ThrowIf(firstName.size() + lastName.size() > MAX_ALLOWED_CHARACTERS, "nameExceedsMaxSize");

            ThrowIfNot(addName(entryId, firstName, lastName, ""), "addNameFailed");
            return true;

            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addName(
        const std::string& entryId,
        const std::string& firstName,
        const std::string& lastName,
        const std::string& nickName,
        const std::string& phoneticFirstName = "",
        const std::string& phoneticLastName = "") {
        try {
            ThrowIf(entryId.empty(), "entryIdEmpty");
            ThrowIf(entryId.size() > MAX_ALLOWED_ENTRY_ID_SIZE, "entryIdExceedsMaxSize");
            ThrowIf(firstName.size() > MAX_ALLOWED_CHARACTERS, "firstNameExceedsMaxSize");
            ThrowIf(lastName.size() > MAX_ALLOWED_CHARACTERS, "lastNameExceedsMaxSize");
            ThrowIf(nickName.size() > MAX_ALLOWED_CHARACTERS, "nickNameExceedsMaxSize");
            ThrowIf(phoneticFirstName.size() > MAX_ALLOWED_CHARACTERS, "phoneticFirstNameExceedsMaxSize");
            ThrowIf(phoneticLastName.size() > MAX_ALLOWED_CHARACTERS, "phoneticLastNameExceedsMaxSize");

            auto totalSize = firstName.size() + lastName.size() + nickName.size() + phoneticFirstName.size() +
                             phoneticLastName.size();
            if (totalSize > MAX_ALLOWED_CHARACTERS) {
                AACE_ERROR(LX(TAG)
                               .m("nameTotalLengthExceedsMaxSize")
                               .d("entryId", entryId)
                               .d("size", totalSize)
                               .d("maxSize", MAX_ALLOWED_CHARACTERS));
                return false;
            }

            auto& allocator = m_response->GetAllocator();
            auto& data = getEntryDataNode(entryId);

            ThrowIf(data.HasMember("name"), "nameFound");

            rapidjson::Value name(rapidjson::kObjectType);

            if (!firstName.empty()) name.AddMember("firstName", firstName, allocator);
            if (!lastName.empty()) name.AddMember("lastName", lastName, allocator);
            if (!nickName.empty()) name.AddMember("nickName", nickName, allocator);
            // phoneticFirstName and phoneticLastName not supported in LVC
            
            data.AddMember("name", name, allocator);

            return true;
        } catch (std::exception& ex) {
            AACE_ERROR(LX(TAG, "addName").d("entryId", entryId).d("reason", ex.what()));
            return false;
        }
    }

    bool addPhone( const std::string& entryId, const std::string& type, const std::string& number ) {
        if( m_addressBookEntity->isAddressTypeSupported( AddressBookEntity::AddressType::PHONE ) ) {
            auto& allocator = m_response->GetAllocator();
            auto& data = getEntryDataNode(entryId);

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
            auto& data = getEntryDataNode(entryId);

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
