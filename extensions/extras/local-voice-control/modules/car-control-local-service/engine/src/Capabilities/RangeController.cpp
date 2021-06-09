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
#include "AACE/Engine/CarControlLocalService/Capabilities/RangeController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

static const std::string TAG("aace.engine.carControlLocalService.RangeController");

RangeController::RangeController(rapidjson::Value& controller) : PrimitiveController(controller) {
    auto* minimumValue = GetValueByPointer(controller, "/configuration/supportedRange/minimumValue");
    auto* maximumValue = GetValueByPointer(controller, "/configuration/supportedRange/maximumValue");
    if (minimumValue && minimumValue->IsNumber()) {
        m_minimum = minimumValue->GetDouble();
    }
    if (maximumValue && maximumValue->IsNumber()) {
        m_maximum = maximumValue->GetDouble();
    }
    m_value = (m_minimum + m_maximum) / 2;
}

bool RangeController::addResponseRangeErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& message) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    std::string type = "VALUE_OUT_OF_RANGE";
    std::string text = getId() + " - " + message;
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember("type", type, allocator);
    payload.AddMember("message", text, allocator);
    rapidjson::Value validRange(rapidjson::kObjectType);
    validRange.AddMember("minimumValue", m_minimum, allocator);
    validRange.AddMember("maximumValue", m_maximum, allocator);
    payload.AddMember("validRange", validRange, allocator);
    event.AddMember("payload", payload, allocator);
    AACE_WARN(LX(TAG).d("type", type).d("message", text));
    return true;
}

bool RangeController::addProperty(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator) {
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

bool RangeController::handleRequest(
    std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
    std::shared_ptr<Endpoint> endpoint,
    std::shared_ptr<rapidjson::Document> request,
    std::shared_ptr<rapidjson::Document> response) {
    bool isValid = false;
    std::string name;
    try {
        ThrowIfNot(JSONUtils::getRequestValue(request, "/directive/header/name", name), "Directive name not found");
        isValid = true;
        if (name == "AdjustRangeValue") {
            auto* value = GetValueByPointer(*request, "/directive/payload/rangeValueDelta");
            if (!value || !value->IsNumber()) {
                std::string errorMessage = "'rangeValueDelta' is not a number";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_VALUE", errorMessage);
                return false;
            }
            auto rangeValueDelta = value->GetDouble();
            ThrowIfNot(carControlService->adjustRangeControllerValue(endpoint->getId(), getInstance(), rangeValueDelta), "Platform interface 'adjustRangeControllerValue' failed");
            double rangeValue = getPropertyValue() + rangeValueDelta;
            if (rangeValue < m_minimum) {
                rangeValue = m_minimum;
            } else if (rangeValue > m_maximum) {
                rangeValue = m_maximum;
            }
            // Update property value since new value is not retrievable via getRangeControllerValue()
            setPropertyValue(rangeValue);
        }
        else if (name == "SetRangeValue") {
            auto* value = GetValueByPointer(*request, "/directive/payload/rangeValue");
            if (!value || !value->IsNumber()) {
                std::string errorMessage = "'rangeValue' is not a number";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseErrorPayload(request, response, "INVALID_VALUE", errorMessage);
                return false;
            }
            auto rangeValue = value->GetDouble();
            if ((m_minimum <= rangeValue) && (rangeValue <= m_maximum)) {
                ThrowIfNot(carControlService->setRangeControllerValue(endpoint->getId(), getInstance(), rangeValue), "Platform interface 'setRangeControllerValue' failed");
                // Update property value since new value is not retrievable via getRangeControllerValue()
                setPropertyValue(rangeValue);
            }
            else {
                std::string errorMessage = "The value " + std::to_string(rangeValue) + " is out of range";
                AACE_ERROR(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("reason", errorMessage));
                addResponseRangeErrorPayload(request, response, errorMessage);
                return false;
            }
        }
        else {
            isValid = false;
            Throw("Name '" + name + "' is not supported by " + getNamespace());
        }
        AACE_DEBUG(LX(TAG).d("endpoint", endpoint->getId()).d("name", name).d("instance", getInstance()).d("value", getPropertyValue()));
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
