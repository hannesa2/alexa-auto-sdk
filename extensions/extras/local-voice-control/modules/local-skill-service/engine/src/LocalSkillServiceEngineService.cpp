/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 
#include <chrono>
#include <thread>

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <curl/curl.h>

#include "AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localSkillService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localSkillService.LocalSkillServiceEngineService");

// name of the table used for the local storage database
static const std::string LOCAL_SKILL_SERVICE_LOCAL_STORAGE_TABLE = "aace.localSkillService";

// register the service
REGISTER_SERVICE(LocalSkillServiceEngineService);

static size_t curlWriteCallback( char* ptr, size_t size, size_t nmemb, void* userdata ) {
    try {
        size_t result = 0;
        if ( userdata != nullptr ) {
            size_t count = size * nmemb;
            auto buffer = static_cast<std::string*>(userdata);
            buffer->append( ptr, count );
            result = count;
        }
        return result;
    }
    catch ( std::exception& ex ) {
        return 0;
    }
}

LocalSkillServiceEngineService::LocalSkillServiceEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ), m_server( nullptr ) {
}

LocalSkillServiceEngineService::~LocalSkillServiceEngineService() = default;

bool LocalSkillServiceEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    try
    {
        bool handled = false;
        uint32_t timeoutMs = 100;

        ThrowIfNotNull( m_server, "HTTPServer already created" );
        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;
        
        document.ParseStream( isw );
        
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidConfigurationStream" );

        rapidjson::Value* serverEndpoint = GetValueByPointer( document, "/lssSocketPath" );
        if ( serverEndpoint && serverEndpoint->IsString() ) {
            m_lssSocketPath = serverEndpoint->GetString();
            m_server = HttpServer::create( m_lssSocketPath, timeoutMs );
            handled = true;
        }

        ThrowIfNull( m_server, "cannot create HTTPServer" );

        m_server->setRequestHandler([this]( std::shared_ptr<engine::localSkillService::HttpRequest> request ) {
            return handleRequest( request );
        });

        registerHandler("/subscribe",
            [this]( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) -> bool {
                return subscribeHandler( request, response );
            }
        );
        registerHandler("/unsubscribe",
            [this]( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) -> bool {
                return unsubscribeHandler( request, response );
            }
        );

        ThrowIfNot( registerServiceInterface<LocalSkillServiceInterface>( shared_from_this() ), "registerLocalSkillServiceFailed" );

        // get the local storage instance
        m_localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>( "aace.storage" );
        ThrowIfNull( m_localStorage, "invalidLocalStorage" );

        return handled;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        configuration->clear();
        configuration->seekg( 0 );
        return false;
    }
}

bool LocalSkillServiceEngineService::start() {
    if ( !m_server ) return false;
    readSubscriptions();
    m_server->start();
    return true;
}

bool LocalSkillServiceEngineService::stop() {
    if ( !m_server ) return false;
    m_server->stop();
    return true;
}

bool LocalSkillServiceEngineService::shutdown() {
    AACE_INFO(LX(TAG));
    m_publishExecutor.shutdown();
    m_handlerExecutor.shutdown();

    if (m_server) {
        m_server.reset();
    }

    if (m_localStorage) {
        m_localStorage.reset();
    }

    m_requestHandlers.clear();
    m_subscribeHandlers.clear();
    m_publishRequestHandlers.clear();
    m_publishResponseHandlers.clear();
    m_subscriptions.clear();

    return true;
}

void LocalSkillServiceEngineService::registerHandler( const std::string& path, RequestHandler handler ) {
    std::lock_guard<std::mutex> guard( m_handlerMutex );
    if ( m_requestHandlers.find( path ) != m_requestHandlers.end() ) {
        AACE_DEBUG(LX( TAG ).d( "replacing handler", path ));
    }
    m_requestHandlers[ path ] = handler;
}

bool LocalSkillServiceEngineService::registerPublishHandler( const std::string& id, RequestHandler subscribeHandler, PublishRequestHandler requestHandler, PublishResponseHandler responseHandler ) {
    try {
        std::lock_guard<std::mutex> guard( m_subscriptionMutex );
        if ( subscribeHandler ) {
            m_subscribeHandlers[ id ] = subscribeHandler;
        }
        if ( requestHandler ) {
            m_publishRequestHandlers[ id ] = requestHandler;
        }
        if ( responseHandler ) {
            m_publishResponseHandlers[ id ] = responseHandler;
        }
        if ( m_subscriptions.find( id ) == m_subscriptions.end() ) {
            m_subscriptions[ id ] = std::make_shared<Subscriptions>();
        }
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::publishMessage( const std::string& id, std::shared_ptr<rapidjson::Document> message ) {
    try {
        std::lock_guard<std::mutex> guard( m_subscriptionMutex );
        ThrowIf( m_subscriptions.find( id ) == m_subscriptions.end(), "subscriptionNotFound");
        PublishRequestHandler requestHandler = nullptr;
        PublishResponseHandler responseHandler = nullptr;
        if ( m_publishRequestHandlers.find( id ) != m_publishRequestHandlers.end() ) {
           requestHandler = m_publishRequestHandlers[id];
        }
        if ( m_publishResponseHandlers.find( id ) != m_publishResponseHandlers.end() ) {
           responseHandler = m_publishResponseHandlers[id];
        }
        auto subscribers = m_subscriptions[ id ]->getSubscribers();
        for ( auto& subscriber : subscribers ) {
            m_publishExecutor.submit( [this, id, subscriber, message, requestHandler, responseHandler] {
                publishMessageToSubscriber( id, subscriber, message, requestHandler, responseHandler );
            } );
        }
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void LocalSkillServiceEngineService::handleRequest( std::shared_ptr<engine::localSkillService::HttpRequest> request ) {
    try {
        std::lock_guard<std::mutex> guard( m_handlerMutex );
        
        // prepare request
        auto path = request->getPath();
        auto method = request->getMethod();
        AACE_DEBUG(LX(TAG).d("request", path).d("method", method));
        std::shared_ptr<rapidjson::Document> jsonRequest = nullptr;
        if ( method == "POST" ) {
            auto payload = request->getBody();
            jsonRequest = std::make_shared<rapidjson::Document>();
            if ( !payload.empty() && jsonRequest->Parse( payload.c_str() ).HasParseError() ) {
                request->respond( 400, "" );
                return;
            }
        }
        
        if ( m_requestHandlers.find( path ) == m_requestHandlers.end() ) {
            request->respond( 404, "" );
            return;
        }
        auto handler = m_requestHandlers[path];

        std::shared_ptr<rapidjson::Document> jsonResponse = std::make_shared<rapidjson::Document>();
        // send to executor
        m_handlerExecutor.submit([handler, jsonRequest, jsonResponse, request]() {
            try {
                auto path = request->getPath();
                if ( handler(jsonRequest, jsonResponse) ) {
                    if ( jsonResponse->IsObject() ) {
                        rapidjson::StringBuffer sb;
                        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                        jsonResponse->Accept( writer );
                        AACE_DEBUG(LX(TAG).d("request", path).d("status", 200));
                        request->respond( 200, sb.GetString() );
                    }
                    else {
                        AACE_DEBUG(LX(TAG).d("request", path).d("status", 204));
                        request->respond( 204, "" );
                    }
                }
                else {
                    AACE_DEBUG(LX(TAG).d("request", path).d("status", 500));
                    request->respond( 500, "" );
                }
            }
            catch( std::exception& ex ) {
                AACE_ERROR(LX(TAG).m("executor").d("reason", ex.what()));
            }
        });
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool LocalSkillServiceEngineService::readSubscriptions() {
    try {
        std::lock_guard<std::mutex> guard( m_subscriptionMutex );
        auto json = m_localStorage->get(LOCAL_SKILL_SERVICE_LOCAL_STORAGE_TABLE, "subscriptions");
        rapidjson::Document document;
        document.Parse(json);
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        for (auto& itr : document.GetArray()) {
            ThrowIfNot(itr.HasMember("id") && itr["id"].IsString(), "No id");
            std::string id = std::string(itr["id"].GetString());
            if (m_subscriptions.find( id ) == m_subscriptions.end()) {
                m_subscriptions[ id ] = std::make_shared<Subscriptions>();
            }
            ThrowIfNot(itr.HasMember("endpoint") && itr["endpoint"].IsString(), "No endpoint");
            ThrowIfNot(itr.HasMember("path") && itr["path"].IsString(), "No path");
            std::shared_ptr<Subscriptions> subscriptions = m_subscriptions[ id ];
            auto subscriber = std::make_shared<Subscriber>( itr["endpoint"].GetString(), itr["path"].GetString() );
            subscriptions->add( subscriber );
        }
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::writeSubscriptions() {
    try {
        rapidjson::Document document(rapidjson::kArrayType);
        auto& allocator = document.GetAllocator();
        for (auto& pair : m_subscriptions) {
            std::string id = pair.first;
            auto& subscription = pair.second;
            for (auto& subscriber : subscription->getSubscribers()) {
                AACE_DEBUG(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()));
                rapidjson::Value item(rapidjson::kObjectType);
                item.AddMember("id", id, allocator);
                item.AddMember("endpoint", subscriber->getEndpoint(), allocator);
                item.AddMember("path", subscriber->getPath(), allocator);
                document.PushBack(item, allocator);
            }
        }
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        document.Accept(writer);
        m_localStorage->put(LOCAL_SKILL_SERVICE_LOCAL_STORAGE_TABLE, "subscriptions", sb.GetString());
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::addSubscription( const std::string& id, std::shared_ptr<Subscriber> subscriber ) {
    try {
        std::lock_guard<std::mutex> guard( m_subscriptionMutex );
        ThrowIfNot( m_subscriptions.find( id ) != m_subscriptions.end(), "subscriptionNotFound");
        std::shared_ptr<Subscriptions> subscriptions = m_subscriptions[ id ];
        if ( subscriptions->add( subscriber ) ) {
            AACE_DEBUG(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()));
            writeSubscriptions();
        }
        else {
            AACE_DEBUG(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()).d("reason", "subscriberFound"));
        }
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::removeSubscription( const std::string& id, std::shared_ptr<Subscriber> subscriber ) {
    try {
        std::lock_guard<std::mutex> guard( m_subscriptionMutex );
        ThrowIfNot( m_subscriptions.find( id ) != m_subscriptions.end(), "subscriptionNotFound");
        std::shared_ptr<Subscriptions> subscriptions = m_subscriptions[ id ];
        if ( subscriptions->remove( subscriber ) ) {
            AACE_DEBUG(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()));
            writeSubscriptions();
        }
        else {
            AACE_DEBUG(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()).d("reason", "subscriberNotFound"));
        }
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("id", id).d("endpoint", subscriber->getEndpoint()).d("path", subscriber->getPath()).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::publishMessageToSubscriber( const std::string& id, std::shared_ptr<Subscriber> subscriber, std::shared_ptr<rapidjson::Document> message, PublishRequestHandler requestHandler, PublishResponseHandler responseHandler ) {
    bool remove = false;
    try {
        std::shared_ptr<rapidjson::Document> request = nullptr;
        long status = 0;
        std::string data;
        std::string payload;
        std::unique_ptr<CURL, std::function<void(CURL *)>> curl(curl_easy_init(), curl_easy_cleanup);
        std::string url = "http://localhost" + subscriber->getPath();
        ThrowIfNull(curl, "curl_easy_init failed");
        ThrowIfNot( curl_easy_setopt( curl.get(), CURLOPT_URL, url.c_str() ) == CURLE_OK, "setServerUrlFailed" );
        ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_UNIX_SOCKET_PATH, subscriber->getEndpoint().c_str() ) == CURLE_OK, "setSocketPathFailed" );
        if (message ) {
            request = message;
        }
        else if ( requestHandler ) {
            request = std::make_shared<rapidjson::Document>();
            ThrowIfNot( requestHandler( request ), "requestHandlerFailed" );
        }
        if ( request && request->IsObject() ) {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            request->Accept( writer );
            payload = sb.GetString();
            AACE_DEBUG(LX(TAG).sensitive("payload", payload));
            ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, payload.c_str() ) == CURLE_OK, "setPayloadFailed" );
            ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, payload.size()) == CURLE_OK, "setPayloadSizeFailed" );
        }
        ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 1000L) == CURLE_OK, "setConnectTimeoutFailed" );
        ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 20000L) == CURLE_OK, "setTimeoutFailed" );
        ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curlWriteCallback ) == CURLE_OK, "setWriteFunctionFailed" );
        ThrowIfNot( curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, (void *)&data) == CURLE_OK, "writeDataFailed" );

        AACE_DEBUG(LX(TAG).d("id", id));

        auto result = curl_easy_perform( curl.get() );
        if ((result == CURLE_COULDNT_RESOLVE_HOST)
            || (result == CURLE_COULDNT_CONNECT)) {
            remove = true;
            Throw("connectionFailed");
        }
        else if (result == CURLE_OPERATION_TIMEDOUT) {
            m_publishExecutor.submit( [this, id, subscriber, message, requestHandler, responseHandler] {
                publishMessageToSubscriber( id, subscriber, message, requestHandler, responseHandler );
            } );
            AACE_WARN(LX(TAG).d("reason","operationTimeout").m("retrying"));
            return false;
        }
        ThrowIfNot( curl_easy_getinfo( curl.get(), CURLINFO_RESPONSE_CODE, &status ) == CURLE_OK, "getInfoFailed" );
        AACE_DEBUG(LX(TAG).d("status", status).sensitive("response", data));
        if ((status < 200) || (status >= 300)) {
            remove = true;
            Throw("errorResponse");
        }
        if ( !data.empty() && responseHandler ) {
            std::shared_ptr<rapidjson::Document> response = std::make_shared<rapidjson::Document>();
            ThrowIf( response->Parse( data.c_str() ).HasParseError(), "parseResponseFailed");
            ThrowIfNot( responseHandler( response ), "responseHandlerFailed");
        }
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("remove", remove));
        if (remove) {
            removeSubscription( id, subscriber );
        }
        return false;
    }
}

bool LocalSkillServiceEngineService::subscribeHandler( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) {
    std::shared_ptr<Subscriber> subscriber = nullptr;
    std::string id;
    try {
        ThrowIfNot( request->IsObject(), "requestPayloadIsEmpty" );
        auto root = request->GetObject();
        ThrowIfNot( root.HasMember( "id" ) && root["id"].IsString()
            && root.HasMember( "endpoint" ) && root["endpoint"].IsString()
            && root.HasMember( "path" ) && root["path"].IsString(), "requestPayloadInvalid" );
        id = root["id"].GetString();
        ThrowIfNot( m_subscriptions.find( id ) != m_subscriptions.end(), "subscriptionNotFound");
        auto endpoint = root["endpoint"].GetString();
        auto path = root["path"].GetString();
        subscriber = std::make_shared<Subscriber>( endpoint, path );
        ThrowIfNull( subscriber, "subscriberInstanceFailed" );
        ThrowIfNot( addSubscription( id, subscriber ), "addSubscriptionFailed" );
        RequestHandler subscribeHandler = nullptr;
        PublishRequestHandler requestHandler = nullptr;
        PublishResponseHandler responseHandler = nullptr;
        {
            std::lock_guard<std::mutex> guardHandler( m_subscriptionMutex );
            if ( m_subscribeHandlers.find( id ) != m_subscribeHandlers.end() ) {
                subscribeHandler = m_subscribeHandlers[id];
            }
            if ( m_publishRequestHandlers.find( id ) != m_publishRequestHandlers.end() ) {
                requestHandler = m_publishRequestHandlers[id];
            }
            if ( m_publishResponseHandlers.find( id ) != m_publishResponseHandlers.end() ) {
                responseHandler = m_publishResponseHandlers[id];
            }
        }
        if ( subscribeHandler ) {
            ThrowIfNot( subscribeHandler(nullptr, response), "subscribeHandlerFailed");
        }
        if ( requestHandler || responseHandler ) {
            m_publishExecutor.submit( [this, id, subscriber, requestHandler, responseHandler] {
                publishMessageToSubscriber( id, subscriber, nullptr, requestHandler, responseHandler );
            } );
        }

        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalSkillServiceEngineService::unsubscribeHandler( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response ) {
    std::shared_ptr<Subscriber> subscriber = nullptr;
    std::string id;
    try {
        ThrowIfNot( request->IsObject(), "requestPayloadIsEmpty" );
        auto root = request->GetObject();
        ThrowIfNot( root.HasMember( "id" ) && root["id"].IsString()
            && root.HasMember( "endpoint" ) && root["endpoint"].IsString()
            && root.HasMember( "path" ) && root["path"].IsString(), "requestPayloadInvalid" );
        id = root["id"].GetString();
        auto endpoint = root["endpoint"].GetString();
        auto path = root["path"].GetString();
        subscriber = std::make_shared<Subscriber>( endpoint, path );
        ThrowIfNot( removeSubscription( id, subscriber ), "removeSubscriptionFailed" );
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

Subscriber::~Subscriber() = default;

// Subscription::~Subscription() = default;

Subscriptions::~Subscriptions() = default;

bool Subscriptions::add( std::shared_ptr<Subscriber> subscriber ) {
    for ( auto const& it: m_subscribers ) {
        if ( it->isEqual( subscriber ) ) {
            return false;
        }
    }
    m_subscribers.push_back( subscriber );
    return true;
}

bool Subscriptions::remove( std::shared_ptr<Subscriber> subscriber ) {
    for ( auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it ) {
        if ( (*it)->isEqual( subscriber ) ) {
            m_subscribers.erase( it );
            return true;
        }
    }
    return false;
}

} // aace::engine::localSkillService
} // aace::engine
} // aace