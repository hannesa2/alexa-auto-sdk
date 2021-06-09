/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_COASSISTANT_COASSISTANT_ENGINE_IMPL_H
#define AACE_ENGINE_COASSISTANT_COASSISTANT_ENGINE_IMPL_H

#include "AACE/Engine/Alexa/WakewordObserverInterface.h"
#include "AACE/Engine/Alexa/WakewordObservableInterface.h"
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AACE/CoAssistant/CoAssistant.h>

#include "CoAssistantCustomDirectiveCapabilityAgent.h"
#include "CoAssistantActiveAgentDelegateInterface.h"

namespace aace {
namespace engine {
namespace coassistant {

class CoAssistantEngineImpl :
    public CoAssistantActiveAgentDelegateInterface,
    public aace::engine::alexa::WakewordObserverInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<CoAssistantEngineImpl> {

private:
    CoAssistantEngineImpl( std::shared_ptr<aace::coassistant::CoAssistant> coAssistantPlatformInterface );

    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext );

public:
    static std::shared_ptr<CoAssistantEngineImpl> create(
        std::shared_ptr<aace::coassistant::CoAssistant> coAssistantPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext );

    // CoAssistantActiveAgentDelegateInterface
    void activeAgentChanged( const std::string& agent ) override;

    bool setPTTAgent( const std::string& agent );
    std::string getPTTAgent();

    // WakewordObserverInterface
    void wakewordDetected( std::string keyword ) override;

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::coassistant::CoAssistant> m_coAssistantPlatformInterface;
    std::shared_ptr<CoAssistantCustomDirectiveCapabilityAgent> m_coAssistantCapabilityAgent;
    std::shared_ptr<aace::engine::alexa::WakewordObservableInterface> m_wakewordObservervableInterfaceFromEngine;
    std::string m_pttAgent;
};
} // aace::engine::coassistant
} // aace::engine
} // aace

#endif // AACE_ENGINE_COASSISTANT_COASSISTANT_ENGINE_IMPL_H