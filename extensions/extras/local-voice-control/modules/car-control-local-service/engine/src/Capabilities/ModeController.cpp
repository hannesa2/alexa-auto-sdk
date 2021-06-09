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
#include "AACE/Engine/CarControlLocalService/JSONUtils.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/ModeController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

static const std::string TAG("aace.engine.carControlLocalService.ModeController");

ModeController::ModeController(rapidjson::Value& controller) : PrimitiveController(controller) {
    auto* ordered = GetValueByPointer(controller, "/configuration/ordered");
    if (ordered && ordered->IsBool()) {
        m_ordered = ordered->GetBool();
    }
    auto* supportedModes = GetValueByPointer(controller, "/configuration/supportedModes");
    if (supportedModes && supportedModes->IsArray()) {
        for (auto& itr : supportedModes->GetArray()) {
            if (itr.HasMember("value")) {
                auto value = std::string(itr["value"].GetString());
                m_supportedModes.push_back(value);
            }
        }
    }
}

bool ModeController::addProperty(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value property(rapidjson::kObjectType);
    property.AddMember("namespace", getNamespace(), allocator);
    property.AddMember("instance", getInstance(), allocator);
    property.AddMember("name", getPropertyName(), allocator);
    property.AddMember("value", getPropertyValue(), allocator);
    property.AddMember("timeOfSample", getPropertyTime(), allocator);
    property.AddMember("uncertaintyInMilliseconds", "500", allocator);
    properties.PushBack(property, allocator);
    return true;
}

bool ModeController::handleRequest(
    std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
    std::shared_ptr<Endpoint> endpoint,
    std::shared_ptr<rapidjson::Document> request,
    std::shared_ptr<rapidjson::Document> response) {
    bool isValid = false;
    std::string name;
    try {
        ThrowIfNot(JSONUtils::getRequestValue(request, "/directive/header/name", name), "Directive name not found");
        isValid = true;
        if (name == "AdjustMode") {
            auto* value = GetValueByPointer(*request, "/directive/payload/modeDelta");
            if (!value || !value->IsInt()) {
                std::string errorMessage = "'modeDelta' is not an integer";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_VALUE", errorMessage);
                return false;
            }
            auto modeDelta = value->GetInt();
            if (m_ordered) {
                ThrowIfNot(carControlService->adjustModeControllerValue(endpoint->getId(), getInstance(), modeDelta), "Platform interface 'adjustModeControllerValue' failed");
            }
            else {
                std::string errorMessage = "This " + getNamespace() + " is not ordered";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_DIRECTIVE", errorMessage);
                return false;
            }
        }
        else if (name == "SetMode") {
            auto* value = GetValueByPointer(*request, "/directive/payload/mode");
            if (!value || !value->IsString()) {
                std::string errorMessage = "'mode' is not a string";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_VALUE", errorMessage);
                return false;
            }
            auto mode = std::string(value->GetString());
            if (std::find(m_supportedModes.begin(), m_supportedModes.end(), mode) != m_supportedModes.end()) {
                ThrowIfNot(carControlService->setModeControllerValue(endpoint->getId(), getInstance(), mode), "Platform interface 'setModeControllerValue' failed");
             }
            else {
                std::string errorMessage = "Mode '" + mode + "' does not exist";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_VALUE", errorMessage);
                return false;
            }
        }
        else {
            isValid = false;
            Throw("Name '" + name + "' is not supported by " + getNamespace());
        }
        std::string value = "";
        ThrowIfNot(carControlService->getModeControllerValue(endpoint->getId(), getInstance(), value), "Platform interface 'getModeControllerValue' failed");
        setPropertyValue(value);
        AACE_DEBUG(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("value", value));
        return true;
    }
    catch(std::exception& ex) {
        AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", ex.what()));
        addResponseErrorPayload(request, response, isValid ? "INTERNAL_ERROR" : "INVALID_DIRECTIVE", ex.what());
        return false;
    }
}

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
