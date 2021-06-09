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

#include <rapidjson/document.h>

#include "AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h"
#include "AACE/Engine/CarControl/CarControlEngineService.h"
#include "AACE/Engine/CarControl/CarControlServiceInterface.h"
#include "AACE/Engine/CarControlLocalService/Endpoint.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

class CarControlLocalService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.carControl.carControlLocalService",VERSION("1.0"),
		DEPENDS(aace::engine::localSkillService::LocalSkillServiceEngineService),
		DEPENDS(aace::engine::carControl::CarControlEngineService))

protected:
    // EngineService
    bool initialize() override;
    bool start() override;
    bool stop() override;
    bool shutdown() override;

    CarControlLocalService(const aace::engine::core::ServiceDescription& description);
public:
    virtual ~CarControlLocalService();
private:
    // implementation
    bool getConfiguration(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response);
    bool handleDiscoveryRequest(std::shared_ptr<rapidjson::Document> request);
    bool handleDiscoveryResponse(std::shared_ptr<rapidjson::Document> response);
    bool handleRequest(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response);

    std::unordered_map<std::string, std::shared_ptr<Endpoint>> m_endpoints;
    std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> m_carControlService;
};

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
