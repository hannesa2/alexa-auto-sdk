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

#ifndef AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_ENGINE_SERVICE_H
#define AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_ENGINE_SERVICE_H

#include <string>

#include <rapidjson/document.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/LocalSkillService/HTTP.h"
#include "AACE/Engine/LocalSkillService/LocalSkillServiceInterface.h"

namespace aace {
namespace engine {
namespace localSkillService {

class Subscriber {
public:
    Subscriber( const std::string& endpoint, const std::string& path ) : m_endpoint( endpoint ), m_path( path ) {}
    virtual ~Subscriber();

    std::string& getEndpoint() {
        return m_endpoint;
    }

    std::string& getPath() {
        return m_path;
    }

    bool isEqual( std::shared_ptr<Subscriber> subscriber ) {
        return isEndpointEqual( subscriber->getEndpoint() )
            && isPathEqual( subscriber->getPath() );
    }

    bool isEndpointEqual( const std::string& endpoint ) {
        return m_endpoint == endpoint;
    }

    bool isPathEqual( const std::string& path ) {
        return m_path == path;
    }
private:
    std::string m_endpoint;
    std::string m_path;
};

class Subscriptions {
public:
    Subscriptions() {}
    virtual ~Subscriptions();

    bool add( std::shared_ptr<Subscriber> subscriber );
    bool remove( std::shared_ptr<Subscriber> subscriber );

    const std::vector<std::shared_ptr<Subscriber>>& getSubscribers() {
        return m_subscribers;
    }
private:
    std::vector<std::shared_ptr<Subscriber>> m_subscribers;
};

class LocalSkillServiceEngineService :
    public aace::engine::core::EngineService,
    public aace::engine::localSkillService::LocalSkillServiceInterface,
    public std::enable_shared_from_this<LocalSkillServiceEngineService> {
public:
    DESCRIBE("aace.localSkillService",VERSION("1.0"),DEPENDS(aace::engine::storage::StorageEngineService))

private:
    LocalSkillServiceEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~LocalSkillServiceEngineService();

    // Services
    void registerHandler( const std::string& path, RequestHandler handler ) override;
    bool registerPublishHandler( const std::string& id, RequestHandler subscribeHandler = nullptr, PublishRequestHandler requestHandler = nullptr, PublishResponseHandler responseHandler = nullptr ) override;
    bool publishMessage( const std::string& id, std::shared_ptr<rapidjson::Document> message = nullptr ) override;
    std::string getLSSSocketPath() override;

protected:
    // EngineService
    bool configure( std::shared_ptr<std::istream> configuration ) override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;

private:
    void handleRequest( std::shared_ptr<engine::localSkillService::HttpRequest> request );  

    // Subscriptions
    bool readSubscriptions();
    bool writeSubscriptions();
    bool addSubscription( const std::string& id, std::shared_ptr<Subscriber> subscriber );
    bool removeSubscription( const std::string& id, std::shared_ptr<Subscriber> subscriber );
    bool publishMessageToSubscriber( const std::string& id, std::shared_ptr<Subscriber> subscriber, std::shared_ptr<rapidjson::Document> message, PublishRequestHandler requestHandler, PublishResponseHandler responseHandler );
    bool subscribeHandler( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response );
    bool unsubscribeHandler( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response );

    std::mutex m_handlerMutex;
    std::unordered_map<std::string, RequestHandler> m_requestHandlers;
    std::unordered_map<std::string, RequestHandler> m_subscribeHandlers;
    std::unordered_map<std::string, PublishRequestHandler> m_publishRequestHandlers;
    std::unordered_map<std::string, PublishResponseHandler> m_publishResponseHandlers;

    std::string m_lssSocketPath;
    std::shared_ptr<HttpServer> m_server;

    std::shared_ptr<aace::engine::storage::LocalStorageInterface> m_localStorage;

    std::mutex m_subscriptionMutex;
    std::unordered_map<std::string, std::shared_ptr<Subscriptions>> m_subscriptions;

    alexaClientSDK::avsCommon::utils::threading::Executor m_handlerExecutor;
    alexaClientSDK::avsCommon::utils::threading::Executor m_publishExecutor;
};

inline std::string LocalSkillServiceEngineService::getLSSSocketPath() {
    return m_lssSocketPath;
}

} // aace::engine::localSkillService
} // aace::engine
} // aace

#endif // AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_ENGINE_SERVICE_H