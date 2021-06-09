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

#include <typeinfo>

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CoAssistant/CoAssistantEngineService.h"

namespace aace {
namespace engine {
namespace coassistant {

// String to identify log entries originating from this file.
static const std::string TAG("aace.coassistant.CoAssistantEngineService");

// register the service
REGISTER_SERVICE(CoAssistantEngineService);

CoAssistantEngineService::CoAssistantEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool CoAssistantEngineService::shutdown()
{
    if( m_coAssistantEngineImpl != nullptr ) {
        m_coAssistantEngineImpl->shutdown();
    }
    
    return true;
}

bool CoAssistantEngineService::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
{
    try
    {
        ReturnIf( registerPlatformInterfaceType<aace::coassistant::CoAssistant>( platformInterface ), true );
        return false;
    }
    catch( std::exception& ex) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool CoAssistantEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::coassistant::CoAssistant> coAssistant)
{
    try
    {
        ThrowIfNotNull( m_coAssistantEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the coassistant engine implementation
        auto alexaComponents = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponents, "invalidAlexaComponentInterface" );

        auto directiveSequencer = alexaComponents->getDirectiveSequencer();
        ThrowIfNull( directiveSequencer, "directiveSequencerInvalid" );

        auto capabilitiesDelegate = alexaComponents->getCapabilitiesDelegate();
        ThrowIfNull( capabilitiesDelegate, "capabilitiesDelegateInvalid" );

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull( exceptionSender, "exceptionSenderInvalid" );

        m_coAssistantEngineImpl = aace::engine::coassistant::CoAssistantEngineImpl::create( coAssistant, directiveSequencer, capabilitiesDelegate, exceptionSender, getContext() );
        ThrowIfNull( m_coAssistantEngineImpl, "createCoAssistantEngineImplFailed" );

        return true;

    }
    catch(std::exception& ex) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<CoAssistant>").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::coassistant
} // aace::engine
} // aace
