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

#include "AACE/Engine/CarControlLocalService/Capabilities/CapabilityInterface.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

class PrimitiveController : public CapabilityInterface {
public:
    PrimitiveController(rapidjson::Value& controller);
    std::string getId() override;
    std::string getInstance();
private:
    std::string m_instance;
};

} // aace::engine::carControlLocalService
} // aace::engine
} // aace