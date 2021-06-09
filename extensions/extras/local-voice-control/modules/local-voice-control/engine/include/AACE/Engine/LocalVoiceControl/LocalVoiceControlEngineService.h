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

#ifndef AACE_ENGINE_LOCAL_VOICE_CONTROL_ENGINE_SERVICE_H
#define AACE_ENGINE_LOCAL_VOICE_CONTROL_ENGINE_SERVICE_H

#include <HybridProxy/HybridProxyCapabilityAgent.h>
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h"
#include "AACE/Engine/LocalVoiceControl/ConfigurationImpl.h"
#include "AACE/Engine/LocalVoiceControl/LocaleImpl.h"
#include "AACE/Engine/LocalVoiceControl/EndpointImpl.h"
#include "AACE/Engine/PropertyManager/PropertyListenerInterface.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class LocalVoiceControlEngineService
        : public aace::engine::core::EngineService
        , public aace::engine::propertyManager::PropertyListenerInterface
        , public std::enable_shared_from_this<LocalVoiceControlEngineService> {
public:
    DESCRIBE(
        "aace.localVoiceControl",
        VERSION("1.0"),
        DEPENDS(aace::engine::alexa::AlexaEngineService),
        DEPENDS(aace::engine::localSkillService::LocalSkillServiceEngineService),
        DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService))

private:
    LocalVoiceControlEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~LocalVoiceControlEngineService() = default;

public:
    // PropertyListenerInterface
    virtual void propertyChanged(const std::string& key, const std::string& newValue) override;

protected:
    bool initialize() override;
    bool configure( std::shared_ptr<std::istream> configuration ) override;
    bool setup() override;
    bool start() override;
    bool stop() override;

private:
    void updateLocale(const std::string& value);

private:

    /// Configuration
    std::shared_ptr<ConfigurationImpl> m_configuration;

    /// Hybrid Proxy capability agent
    std::shared_ptr<alexaClientSDK::capabilityAgents::hybridProxy::HybridProxyCapabilityAgent> m_hybridProxyCapabilityAgent;

    /// current locale
    std::shared_ptr<LocaleImpl> m_locale;

    /// current endpoint
    std::shared_ptr<EndpointImpl> m_endpoint;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_ENGINE_LOCAL_VOICE_CONTROL_ENGINE_SERVICE_H