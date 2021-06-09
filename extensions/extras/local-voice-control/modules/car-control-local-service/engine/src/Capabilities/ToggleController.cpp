/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/CarControlLocalService/Capabilities/ToggleController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

static const std::string TAG("aace.engine.carControlLocalService.ToggleController");

ToggleController::ToggleController(rapidjson::Value& controller) : PrimitiveController(controller) {
}

bool ToggleController::addProperty(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator) {
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

bool ToggleController::handleRequest(
    std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
    std::shared_ptr<Endpoint> endpoint,
    std::shared_ptr<rapidjson::Document> request,
    std::shared_ptr<rapidjson::Document> response) {
    bool isValid = false;
    std::string name;
    try {
        ThrowIfNot(JSONUtils::getRequestValue(request, "/directive/header/name", name), "Directive name not found");
        isValid = true;
        if (name == "TurnOn") {
            ThrowIfNot(carControlService->turnToggleControllerOn(endpoint->getId(), getInstance()), "Platform interface 'turnToggleControllerOn' failed");
            // Update property value since new value is not retrievable via isToggleControllerOn()
            setPropertyValue(true);
        }
        else if (name == "TurnOff") {
            ThrowIfNot(carControlService->turnToggleControllerOff(endpoint->getId(), getInstance()), "Platform interface 'turnToggleControllerOff' failed");
            // Update property value since new value is not retrievable via isToggleControllerOn()
            setPropertyValue(false);
        }
        else {
            isValid = false;
            Throw("Name '" + name + "' is not supported by " + getNamespace());
        }
        AACE_DEBUG(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()));
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
