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

#ifndef AACE_LOCAL_VOICE_CONTROL_CONFIGURATION_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_CONFIGURATION_IMPL_H

#include "alexa_hybrid/sdk/IConfiguration.h"
#include <unordered_map>

namespace aace {
namespace engine {
namespace localVoiceControl {

class ConfigurationImpl : 
    public AlexaHybrid::SDK::IConfiguration,
    public std::enable_shared_from_this<ConfigurationImpl> {

public:
    /**
     * Default constructor.
     */
    ConfigurationImpl();

public:
    /**
     * Keys for configuration properties that user can set.
     */
    static const std::string EXECUTION_CONTROLLER_DIRECTORY;
    static const std::string EXECUTION_CONTROLLER_PERMISSIONS;
    static const std::string PLATFORM_SERVICES_DIRECTORY;
    static const std::string PLATFORM_SERVICES_PERMISSIONS;
    static const std::string ADDRESS;
    static const std::string MESSAGE_ROUTER_DIRECTORY;

public:  
    /**
     * Gets the value of @c name.
     *
     * @param name property name
     * @return property value or an empty string if @c name does not exist
     */
    std::string getProperty(const std::string &name) const override { return getProperty(name, ""); }

    /**
     * Gets the value of @c name or returns the provided default value if @c name does not exist.
     *
     * @param name property name
     * @param defaultValue value to return if @c name does not exist
     * @return property value or the provided default value if @c name does not exist
     */
    std::string getProperty(const std::string &name, const std::string &defaultValue) const override;

    /**
     * Tests if a property exists.
     *
     * @param name property name
     * @return @c true if property @c name exists, @c false otherwise
     */
    bool hasProperty(const std::string &name) const override;

public:
    /**
     * Saves the given property and value in the configuration map.
     * 
     * @param key The property key
     * @param value The property value 
     */
    void setProperty(std::string key, std::string value);

private:
    /// Configuation map
    std::unordered_map<std::string, std::string> m_properties;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_CONFIGURATION_IMPL_H