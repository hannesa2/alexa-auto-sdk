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

#include "AACE/Engine/Alexa/WakewordObservableInterface.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CoAssistant/CoAssistantEngineImpl.h"
#include <AVSCommon/Utils/RequiresShutdown.h>

namespace aace {
namespace engine {
namespace coassistant {

// String to identify log entries originating from this file.
static const std::string TAG("aace.coassistant.CoAssistantEngineImpl");

CoAssistantEngineImpl::CoAssistantEngineImpl( std::shared_ptr<aace::coassistant::CoAssistant> coAssistantPlatformInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_coAssistantPlatformInterface( coAssistantPlatformInterface ) {
}

bool CoAssistantEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext ) {
    
    try
    {
        // passing weak ptr of this class for callback methods
        std::weak_ptr<CoAssistantActiveAgentDelegateInterface> coAssistantActiveAgentDelegate = shared_from_this();
        m_coAssistantCapabilityAgent = CoAssistantCustomDirectiveCapabilityAgent::create ( exceptionSender, coAssistantActiveAgentDelegate );
        ThrowIfNull (m_coAssistantCapabilityAgent, "couldNotCreateCapabilityAgent" );

        ThrowIfNot( directiveSequencer->addDirectiveHandler(m_coAssistantCapabilityAgent), "addDirectiveHandlerFailed" );

        AACE_DEBUG(LX(TAG,"RegisterCapability").d("CoAssistant","successful")); 
        
        // For wakeword observer
        // Get the Alexa Engine Service Handler
        ThrowIfNull( engineContext, "engineContextIsNull" );

        m_wakewordObservervableInterfaceFromEngine = engineContext->getServiceInterface<aace::engine::alexa::WakewordObservableInterface>(  "aace.alexa" );
        ThrowIfNull( m_wakewordObservervableInterfaceFromEngine, "wakewordObservervableInterfaceFromEngineIsNull" );
        if ( m_wakewordObservervableInterfaceFromEngine != nullptr ) {
            m_wakewordObservervableInterfaceFromEngine->addWakewordObserver( shared_from_this() );
            AACE_DEBUG(LX(TAG, "CoAssistantEngineImpl").d("CoAssistant","wakewordObservervableInterfaceFromEngine added observer"));
        }

        return true;
    }
    catch ( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<CoAssistantEngineImpl> CoAssistantEngineImpl::create(
    std::shared_ptr<aace::coassistant::CoAssistant> coassistantPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext ) {

    try
    {
        ThrowIfNull( coassistantPlatformInterface, "couldNotCreateCoAssistantPlatformInterface" );
        ThrowIfNull( capabilitiesDelegate, "couldNotCreateCoAssistantPlatformInterface" );
        ThrowIfNull( directiveSequencer, "couldNotCreateCoAssistantPlatformInterface" );
        ThrowIfNull( exceptionSender, "couldNotCreateCoAssistantPlatformInterface" );

        std::shared_ptr<CoAssistantEngineImpl> coassistantEngineImpl = std::shared_ptr<CoAssistantEngineImpl>( new CoAssistantEngineImpl( coassistantPlatformInterface ) );

        ThrowIfNot( coassistantEngineImpl->initialize( directiveSequencer, capabilitiesDelegate, exceptionSender, engineContext ), "initializeCoAssistantEngineImplFailed" );

        AACE_DEBUG(LX(TAG,"CoAssistantEngineImpl").d("CoAssistant","successful creation on Engine Impl")); 
        return coassistantEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void CoAssistantEngineImpl::doShutdown()
{
    if( m_coAssistantCapabilityAgent != nullptr ) {
        m_coAssistantCapabilityAgent->shutdown();
    }

    if ( m_wakewordObservervableInterfaceFromEngine != nullptr ) {
        m_wakewordObservervableInterfaceFromEngine->removeWakewordObserver( shared_from_this() );
    }
}

void CoAssistantEngineImpl::activeAgentChanged( const std::string& agent ) {
    if ( m_coAssistantPlatformInterface != nullptr ) {
        m_coAssistantPlatformInterface->activeAgentChanged( agent );
    }
}

bool CoAssistantEngineImpl::setPTTAgent( const std::string& agent ) {
    // TODO: Update cloud about the PTT agent.
    m_pttAgent = agent;
    AACE_DEBUG(LX(TAG,"CoAssistantEngineImpl").d("setPTTAgent to agent ",m_pttAgent));
    return true;
}

std::string CoAssistantEngineImpl::getPTTAgent() {
    return m_pttAgent;
}

void CoAssistantEngineImpl::wakewordDetected( std::string keyword ) {
    AACE_DEBUG(LX(TAG,"CoAssistantEngineImpl").d("propagating activeagentchanged | wakewordDetected",keyword));

    if ( m_coAssistantPlatformInterface != nullptr ) {
        m_coAssistantPlatformInterface->activeAgentChanged( keyword );
    }
}

} // aace::engine::coassistant
} // aace::engine
} // aace