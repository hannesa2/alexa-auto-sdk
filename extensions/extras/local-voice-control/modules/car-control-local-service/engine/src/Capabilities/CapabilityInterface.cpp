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
#include "AACE/Engine/CarControlLocalService/Capabilities/CapabilityInterface.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

static const std::string TAG("aace.engine.carControlLocalService.Capability");

CapabilityInterface::~CapabilityInterface() = default; // key function

CapabilityInterface::CapabilityInterface(rapidjson::Value& controller) {
    m_namespace = std::string(controller["interface"].GetString());
    m_retrievable = controller["properties"]["retrievable"].GetBool();
}

bool CapabilityInterface::addContext(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator, bool retrieved) {
    if ((retrieved && !m_retrievable) || (!retrieved && !m_changed)) {
        return true;
    }
    addProperty(properties, allocator);
    if (!retrieved) {
        m_changed = false;
    }
    return true;
}

bool CapabilityInterface::addResponseErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& type, const std::string& message) {
    auto& allocator = response->GetAllocator();
    auto& event = (*response)["event"];
    auto text = getId() + " - " + message;
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember("type", type, allocator);
    payload.AddMember("message", text, allocator);
    event.AddMember("payload", payload, allocator);
    AACE_DEBUG(LX(TAG).d("type", type).d("message", text));
    return true;
}

std::string CapabilityInterface::getId() {
    return m_namespace;
}

std::string CapabilityInterface::getNamespace() {
    return m_namespace;
}

std::string CapabilityInterface::getPropertyTime() {
    return m_time;
}

void CapabilityInterface::propertyChanged() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    time_t seconds = tp.tv_sec;
    struct tm *time = gmtime(&seconds);
    char cstr[25];
    
    snprintf( cstr, 25, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
        1900 + time->tm_year, time->tm_mon + 1, time->tm_mday,
        time->tm_hour, time->tm_min, time->tm_sec, (int)(tp.tv_usec / 1000) );
    m_time = cstr;
    m_changed = true;
}

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
