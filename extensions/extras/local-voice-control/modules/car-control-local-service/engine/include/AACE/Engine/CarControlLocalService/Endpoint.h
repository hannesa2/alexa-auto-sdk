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

#include "AACE/Engine/CarControl/CarControlServiceInterface.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

class CapabilityInterface;

class Endpoint : public std::enable_shared_from_this<Endpoint> {
public:

    static bool addResponseHeader(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& name);
    static bool addResponseEndpoint(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& endpointId);
    static bool addResponsePayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response);
    static bool addResponseErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& type, const std::string& message);

    Endpoint(rapidjson::Value& endpoint);
    
    std::string getId();
    bool handleRequest(
        std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
        std::shared_ptr<rapidjson::Document> request,
        std::shared_ptr<rapidjson::Document> response);
    bool addContext(std::shared_ptr<rapidjson::Document> response, bool retrieved);

private:

    std::unordered_map<std::string, std::shared_ptr<CapabilityInterface>> m_capabilities;
    std::string m_id;
    std::string m_description;
};

} // aace::engine::carControlLocalService
} // aace::engine
} // aace