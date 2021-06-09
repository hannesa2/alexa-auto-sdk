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

#include "AACE/Engine/CarControlLocalService/Capabilities/PrimitiveController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

class RangeController : public PrimitiveController {
public:
    RangeController(rapidjson::Value& controller);

    bool addProperty(rapidjson::Value& properties, rapidjson::Document::AllocatorType& allocator) override;
    bool addResponseRangeErrorPayload(std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response, const std::string& message);
    bool handleRequest(
        std::shared_ptr<aace::engine::carControl::CarControlServiceInterface> carControlService,
        std::shared_ptr<Endpoint> endpoint,
        std::shared_ptr<rapidjson::Document> request,
        std::shared_ptr<rapidjson::Document> response) override;
    
    std::string getPropertyName() override {
        return "rangeValue";
    }
    
    double getPropertyValue() {
        return m_value;
    }
    
    void setPropertyValue(double value) {
        if (m_value != value) {
            propertyChanged();
            m_value = value;
        }
    }
private:
    double m_value;
    double m_minimum;
    double m_maximum;
};

} // aace::engine::carControlLocalService
} // aace::engine
} // aace