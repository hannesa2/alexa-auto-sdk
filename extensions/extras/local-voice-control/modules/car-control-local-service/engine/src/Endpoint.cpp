/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/pointer.h>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CarControlLocalService/Endpoint.h"
#include "AACE/Engine/CarControlLocalService/JSONUtils.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/PowerController.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/ModeController.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/RangeController.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/ToggleController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

// String to identify log entries originating from this file.
static const std::string TAG("aace.carControlLocalService.Endpoint");

bool Endpoint::addResponseHeader(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& name) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    rapidjson::Value header(rapidjson::kObjectType);
    header.AddMember("namespace", "Alexa", allocator);
    header.AddMember("name", name, allocator);
    header.AddMember("payloadVersion", "3", allocator);
    rapidjson::Value* messageId = GetValueByPointer(*request, "/directive/header/messageId");
    if (messageId) {
        header.AddMember("messageId", rapidjson::StringRef(messageId->GetString()), allocator);
    }
    rapidjson::Value* correlationToken = GetValueByPointer(*request, "/directive/header/correlationToken");
    if (correlationToken) {
        header.AddMember("correlationToken", rapidjson::StringRef(correlationToken->GetString()), allocator);
    }
    event.AddMember("header", header, allocator);
    return true;
}

bool Endpoint::addResponseEndpoint(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& endpointId) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    rapidjson::Value endpoint(rapidjson::kObjectType);
    rapidjson::Value* requestScope = GetValueByPointer(*request, "/directive/endpoint/scope");
    if (requestScope) {
        rapidjson::Value scope(rapidjson::kObjectType);
        if (requestScope->HasMember("type")) {
            scope.AddMember("type", rapidjson::StringRef((*requestScope)["type"].GetString()), allocator);
        }
        if (requestScope->HasMember("token")) {
            scope.AddMember("token", rapidjson::StringRef((*requestScope)["token"].GetString()), allocator);
        }
        endpoint.AddMember("scope", scope, allocator);
    }
    endpoint.AddMember("endpointId", endpointId, allocator);
    event.AddMember("endpoint", endpoint, allocator);
    return true;
}

bool Endpoint::addResponsePayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    rapidjson::Value payload(rapidjson::kObjectType);
    event.AddMember("payload", payload, allocator);
    return true;
}

bool Endpoint::addResponseErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& type, const std::string& message) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember("type", type, allocator);
    payload.AddMember("message", message, allocator);
    event.AddMember("payload", payload, allocator);
    AACE_DEBUG(LX(TAG).d("type", type).d("message", message));
    return true;
}

Endpoint::Endpoint(rapidjson::Value& endpoint) {
    ThrowIfNot(endpoint.HasMember("endpointId") && endpoint["endpointId"].IsString(), "No endpointId");
    ThrowIfNot(endpoint.HasMember("capabilities") && endpoint["capabilities"].IsArray(), "No capabilities");
    m_id = std::string(endpoint["endpointId"].GetString());
    if (endpoint.HasMember("description") && endpoint["description"].IsString()) {
        m_description = std::string(endpoint["description"].GetString());
    }
    auto& capabilities = endpoint["capabilities"];
    for (auto& itr : capabilities.GetArray()) {
        ThrowIfNot(itr.HasMember("interface") && itr["interface"].IsString(), "No interface");
        std::shared_ptr<CapabilityInterface> controller;
        auto interface = std::string(itr["interface"].GetString());
        if (interface == "Alexa.PowerController") {
            controller = std::make_shared<PowerController>(itr);
        }
        else if (interface == "Alexa.ModeController") {
            controller = std::make_shared<ModeController>(itr);
        }
        else if (interface == "Alexa.RangeController") {
            controller = std::make_shared<RangeController>(itr);
        }
        else if (interface == "Alexa.ToggleController") {
            controller = std::make_shared<ToggleController>(itr);
        }
        else {
            AACE_INFO(LX(TAG).d("unsupported", interface));
        }
        if (controller) {
            m_capabilities[controller->getId()] = controller;
        }
    }
}

std::string Endpoint::getId() {
    return m_id;
}

bool Endpoint::handleRequest(
    std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
    std::shared_ptr<rapidjson::Document> request,
    std::shared_ptr<rapidjson::Document> response) {
    try {
        std::string id, name;
        ThrowIfNot(JSONUtils::getRequestValue(request, "/directive/header/namespace", id), "Directive namespace not found");
        ThrowIfNot(JSONUtils::getRequestValue(request, "/directive/header/name", name), "Directive name not found");

        auto* instance = GetValueByPointer(*request, "/directive/header/instance");
        if (instance && instance->IsString()) {
            id += "#" + std::string(instance->GetString());
        }
        if ((id == "Alexa") && (name == "ReportState")) {
            addContext(response, true);
            Endpoint::addResponseHeader(request, response, "StateReport");
            Endpoint::addResponseEndpoint(request, response, getId());
            Endpoint::addResponsePayload(request, response);
        }
        else if (m_capabilities.find(id) != m_capabilities.end()) {
            auto& capability = m_capabilities[id];
            ThrowIfNot(capability->handleRequest(carControlService, shared_from_this(), request, response), "capabilityCannotHandleRequest");
            addContext(response, false);
            Endpoint::addResponseHeader(request, response, "Response");
            Endpoint::addResponseEndpoint(request, response, getId());
            Endpoint::addResponsePayload(request, response);
        }
        else {
            Throw(id + " - Controller not found");
        }
        return true;
    }
    catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("endpoint", getId()).d("reason", ex.what()));
        Endpoint::addResponseHeader(request, response, "ErrorResponse");
        Endpoint::addResponseEndpoint(request, response, getId());
        if (!(*response)["event"].HasMember("payload")) {
            Endpoint::addResponseErrorPayload(request, response, "INVALID_DIRECTIVE", ex.what());
        }
        return false;
    }
}

bool Endpoint::addContext(std::shared_ptr<rapidjson::Document> response, bool retrieved) {
    try {
        auto& allocator = response->GetAllocator();
        rapidjson::Value context(rapidjson::kObjectType);
        rapidjson::Value properties(rapidjson::kArrayType);
        for (auto& itr : m_capabilities) {
            auto& capability = itr.second;
            capability->addContext(properties, allocator, retrieved);
        }
        context.AddMember("properties", properties, allocator);
        response->AddMember("context", context, allocator);
        return true;
    }
    catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
