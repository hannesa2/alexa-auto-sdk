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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CarControlLocalService/Capabilities/PrimitiveController.h"

namespace aace {
namespace engine {
namespace carControlLocalService {

PrimitiveController::PrimitiveController(rapidjson::Value& controller) : CapabilityInterface(controller) {
    ThrowIfNot(controller.HasMember("instance") && controller["instance"].IsString(), "No instance");
    m_instance = std::string(controller["instance"].GetString());
}

std::string PrimitiveController::getId() {
    return m_namespace + "#" + m_instance;
}

std::string PrimitiveController::getInstance() {
    return m_instance;
}

} // aace::engine::carControlLocalService
} // aace::engine
} // aace
