/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CarControlLocalService/CarControlLocalService.h"
#include "AACE/Engine/CarControlLocalService/Endpoint.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.carControlLocalService");

/// The name of the 'carControl' table in the database
static const std::string CAR_CONTROL_CONFIG_TABLE = "carControl";
/// The key for the 'configutation' in the database 'carControl' table
static const std::string CAR_CONTROL_CONFIG_KEY = "configuration";

// register the service
REGISTER_SERVICE(CarControlLocalService);

CarControlLocalService::CarControlLocalService(const aace::engine::core::ServiceDescription& description) : aace::engine::core::EngineService(description) {
}

CarControlLocalService::~CarControlLocalService() = default;

bool CarControlLocalService::initialize() {
    try
    {
        auto carControlEngineService = getContext()->getServiceInterface<aace::engine::carControl::CarControlEngineService>("aace.carControl");
        ThrowIfNull(carControlEngineService, "no carControlEngineService");
        carControlEngineService->setLocalServiceAvailability(true);
        return true;
    }
    catch(std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlLocalService::start() {
    try
    {        
        auto localSkillService = getContext()->getServiceInterface<aace::engine::localSkillService::LocalSkillServiceEngineService>("aace.localSkillService");
        ThrowIfNull(localSkillService, "no localSkillService");

        localSkillService->registerHandler("/carControl/configuration",
            [this](std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) -> bool {
                return getConfiguration(request, response);
            }
        );
        localSkillService->registerHandler("/carControl/handleRequest",
            [this](std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) -> bool {
                return handleRequest(request, response);
            }
        );

        localSkillService->registerPublishHandler("/carControl/discovery",
            nullptr,
            [this]( std::shared_ptr<rapidjson::Document> request ) -> bool {
                    return handleDiscoveryRequest( request );
            },
            [this]( std::shared_ptr<rapidjson::Document> response ) -> bool {
                    return handleDiscoveryResponse( response );
            }
        );

        auto carControlServiceInterface = getContext()->getServiceInterface<aace::engine::carControl::CarControlServiceInterface>("aace.carControl");
        ThrowIfNull(carControlServiceInterface, "no carControlServiceInterface");
        m_carControlService = carControlServiceInterface;

        auto localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");
        ThrowIfNull(localStorage, "invalidLocalStorage");

        ThrowIfNot(localStorage->containsKey(CAR_CONTROL_CONFIG_TABLE, CAR_CONTROL_CONFIG_KEY), "configurationNotFound");
        rapidjson::Document document;
        auto configuration = localStorage->get(CAR_CONTROL_CONFIG_TABLE, CAR_CONTROL_CONFIG_KEY);
        document.Parse(configuration);

        ThrowIfNot(document.HasMember("endpoints"), "configurationHasNoEndpoints");
        auto& endpoints = document["endpoints"];
        for (auto& itr : endpoints.GetArray()) {
            auto endpoint = std::make_shared<Endpoint>(itr);
            m_endpoints[endpoint->getId()] = endpoint;
        }

        localSkillService->publishMessage("/carControl/discovery", nullptr);
        return true;
    }
    catch(std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlLocalService::stop() {
    return true;
}

bool CarControlLocalService::shutdown() {
    if (m_carControlService) {
        m_carControlService.reset();
    }
    
    return true;
}

// Car Control implementation

bool CarControlLocalService::getConfiguration(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) {
    try {
        if (!response) return false;
        auto localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");
        ThrowIfNull(localStorage, "invalidLocalStorage");
        auto configuration = localStorage->get(CAR_CONTROL_CONFIG_TABLE, CAR_CONTROL_CONFIG_KEY);

        response->Parse(configuration);
        return true;
    }
    catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlLocalService::handleDiscoveryRequest( std::shared_ptr<rapidjson::Document> request ) {
    try {
        if ( !request ) return false;
        auto localStorage = getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>("aace.storage");
        ThrowIfNull(localStorage, "invalidLocalStorage");
        auto configuration = localStorage->get(CAR_CONTROL_CONFIG_TABLE, CAR_CONTROL_CONFIG_KEY);
        request->Parse(configuration);
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlLocalService::handleDiscoveryResponse( std::shared_ptr<rapidjson::Document> response ) {
    try {
        if ( response->IsObject() ) {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            response->Accept( writer );
            AACE_WARN(LX(TAG).d("response", sb.GetString()));
        }
        else {
            AACE_INFO(LX(TAG).d("response", "empty"));
        }
        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool CarControlLocalService::handleRequest(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) {
    std::string endpointId;
    bool result = true;
    try {
        if (!request) return false;
        if (!response) return false;
        // prepare response
        response->SetObject();
        auto& allocator = response->GetAllocator();
        auto event(rapidjson::kObjectType);
        response->AddMember("event", event, allocator);
        // get endpointId
        auto* value = GetValueByPointer(*request, "/directive/endpoint/endpointId");
        ThrowIfNull(value, "No endpoint");
        ThrowIfNot(value->IsString(), "Endpoint is no string");
        endpointId = value->GetString();
        // execute directive
        if (m_endpoints.find(endpointId) != m_endpoints.end()) {
            if (m_carControlService) {
                auto& endpoint = m_endpoints[endpointId];
                endpoint->handleRequest(m_carControlService, request, response);
            }
            else {
                Endpoint::addResponseHeader(request, response, "ErrorResponse");
                Endpoint::addResponseEndpoint(request, response, endpointId);
                Endpoint::addResponseErrorPayload(request, response, "NOT_IN_OPERATION", "Platform does not exist");
            }
        }
        else {
            Endpoint::addResponseHeader(request, response, "ErrorResponse");
            Endpoint::addResponseEndpoint(request, response, endpointId);
            Endpoint::addResponseErrorPayload(request, response, "NO_SUCH_ENDPOINT", "Endpoint '" + endpointId + "' does not exist");
        }
    }
    catch (std::exception& ex) {
        result = false;
    }
    // skill only ingests payload
    auto* value = GetValueByPointer(*response, "/event/payload");
    if (value) {
        response->CopyFrom(*value, response->GetAllocator());
    }
    return result;
}

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
