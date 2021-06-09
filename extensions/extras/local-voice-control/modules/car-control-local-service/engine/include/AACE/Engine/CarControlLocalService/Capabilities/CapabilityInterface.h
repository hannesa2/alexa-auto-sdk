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

#pragma once

#include <sys/time.h>
#include <rapidjson/document.h>

#include "AACE/Engine/CarControl/CarControlServiceInterface.h"
#include "AACE/Engine/CarControlLocalService/Endpoint.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

class Endpoint;

class CapabilityInterface {
public:
    CapabilityInterface(rapidjson::Value& controller);
    virtual ~CapabilityInterface();
    bool addContext(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator, bool retrieved);
    virtual bool addProperty(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator) = 0;
    
    bool addResponseErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& type, const std::string& message);
    std::string getDate();
    virtual std::string getId();
    std::string getNamespace();
    virtual std::string getPropertyName() = 0;
    std::string getPropertyTime();
    void propertyChanged();

    virtual bool handleRequest(
        std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
        std::shared_ptr<Endpoint> endpoint,
        std::shared_ptr<rapidjson::Document> request,
        std::shared_ptr<rapidjson::Document> response) = 0;
protected:
    std::string m_namespace;
    bool m_retrievable;
    bool m_changed = true;
    std::string m_time = "1970-01-01T00:00:00.00Z";
};

} // aace::engine::carControlLocalService
} // aace::engine
} // aace