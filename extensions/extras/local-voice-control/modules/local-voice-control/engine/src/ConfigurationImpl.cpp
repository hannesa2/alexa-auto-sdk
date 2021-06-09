/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/LocalVoiceControl/ConfigurationImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.ConfigurationImpl");

// Keys to LVC IPC config
const std::string ConfigurationImpl::EXECUTION_CONTROLLER_DIRECTORY   = "AlexaHybrid.SDK.ExecutionController.SocketDirectory";
const std::string ConfigurationImpl::EXECUTION_CONTROLLER_PERMISSIONS = "AlexaHybrid.SDK.ExecutionController.SocketPermissions";
const std::string ConfigurationImpl::PLATFORM_SERVICES_DIRECTORY      = "AlexaHybrid.SDK.PlatformServices.SocketDirectory";
const std::string ConfigurationImpl::PLATFORM_SERVICES_PERMISSIONS    = "AlexaHybrid.SDK.PlatformServices.SocketPermissions";
const std::string ConfigurationImpl::ADDRESS                          = "AlexaHybrid.SDK.ExecutionController.Address";
const std::string ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY         = "AlexaHybrid.SDK.ExecutionController.UnixDomainSocketDirectory";

ConfigurationImpl::ConfigurationImpl() {
}

std::string ConfigurationImpl::getProperty(const std::string &name, const std::string &defaultValue) const {
    AACE_INFO(LX(TAG, "Getting property").d("name", name)); 

    // If the value exists in the map, return it 
    auto search = m_properties.find(name);
    if (search != m_properties.end()) {
        AACE_DEBUG(LX(TAG, "Returning found value").d("value", search->second));
        return search->second;
    }

    AACE_DEBUG(LX(TAG, "Returning default value").d("value", defaultValue));

    return defaultValue;
}

bool ConfigurationImpl::hasProperty(const std::string &name) const {
    return m_properties.find(name) != m_properties.end();
}

void ConfigurationImpl::setProperty(std::string key, std::string value) {
    m_properties[key] = value;
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace