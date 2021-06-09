/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/LocalNavigation/AASBLocalSearchProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/LocalNavigation/LocalSearchProvider/PoiLookupRequestMessage.h>
#include <AASB/Message/LocalNavigation/LocalSearchProvider/PoiLookupResponseMessage.h>
#include <AASB/Message/LocalNavigation/LocalSearchProvider/PoiSearchRequestMessage.h>
#include <AASB/Message/LocalNavigation/LocalSearchProvider/PoiSearchResponseMessage.h>

namespace aasb {
namespace engine {
namespace localNavigation {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.localNavigation.AASBLocalSearchProvider");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBLocalSearchProvider> AASBLocalSearchProvider::create( std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker )
{
    try
    {
        ThrowIfNull( messageBroker, "invalidMessageBrokerInterface" );

        auto handler = std::shared_ptr<AASBLocalSearchProvider>( new AASBLocalSearchProvider() );

        // initialize the handler
        ThrowIfNot( handler->initialize( messageBroker ), "initializeAASBLocalSearchProviderFailed" );

        return handler;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBLocalSearchProvider::initialize( std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker )
{
    try
    {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBLocalSearchProvider> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::localNavigation::localSearchProvider::PoiLookupResponseMessage::topic(),
            aasb::message::localNavigation::localSearchProvider::PoiLookupResponseMessage::action(),
            [wp](const Message& message) {

                try
                {
                    auto sp = wp.lock();
                    ThrowIfNull( sp, "invalidWeakPtrReference" );
                    aasb::message::localNavigation::localSearchProvider::PoiLookupResponseMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->poiLookupResponse( payload.response );
                }
                catch( std::exception& ex ) {
                    AACE_ERROR(LX(TAG,"PoiLookupResponseMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::localNavigation::localSearchProvider::PoiSearchResponseMessage::topic(),
            aasb::message::localNavigation::localSearchProvider::PoiSearchResponseMessage::action(),
            [wp](const Message& message) {

                try
                {
                    auto sp = wp.lock();
                    ThrowIfNull( sp, "invalidWeakPtrReference" );
                    aasb::message::localNavigation::localSearchProvider::PoiSearchResponseMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->poiSearchResponse( payload.response );
                }
                catch( std::exception& ex ) {
                    AACE_ERROR(LX(TAG,"PoiSearchResponseMessage").d("reason", ex.what()));
                }
            });
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::localNavigation::localSearchProvider
//

bool AASBLocalSearchProvider::poiLookupRequest( const std::string& request ) {
    try
    {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull( m_messageBroker_lock, "invalidMessageBrokerReference" );

        aasb::message::localNavigation::localSearchProvider::PoiLookupRequestMessage message;

        nlohmann::json request_json = nlohmann::json::parse( request );
        message.header.id = request_json["requestId"];
        message.payload.request = request;

        m_messageBroker_lock->publish( message.toString() ).send();
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBLocalSearchProvider::poiSearchRequest( const std::string& request ) {
    try
    {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull( m_messageBroker_lock, "invalidMessageBrokerReference" );

        aasb::message::localNavigation::localSearchProvider::PoiSearchRequestMessage message;

        nlohmann::json request_json = nlohmann::json::parse( request );
        message.header.id = request_json["requestId"];
        message.payload.request = request;

        m_messageBroker_lock->publish( message.toString() ).send();
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // aasb::engine::localNavigation
}  // aasb::engine
}  // aasb
