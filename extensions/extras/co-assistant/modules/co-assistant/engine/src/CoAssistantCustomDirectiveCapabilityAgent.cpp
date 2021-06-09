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

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/CoAssistant/CoAssistantCustomDirectiveCapabilityAgent.h"

#include "rapidjson/document.h"
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace coassistant {

// String to identify log entries originating from this file.
static const std::string TAG("aace.coassistant.CoAssistantCustomDirectiveCapabilityAgent");

/// The namespace for this capability agent.
static const std::string NAMESPACE{"Custom.Firebird"};



/// Capability constants
/// interface type
static const std::string COASSISTANT_CAPABILITY_INTERFACE_TYPE = "CustomInterfaceController.SendDirective";
/// interface name
static const std::string COASSISTANT_CAPABILITY_INTERFACE_NAME = "SetAgent";
/// interface version
static const std::string COASSISTANT_CAPABILITY_INTERFACE_VERSION = "1.0";

/// SetAgent directive
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SET_AGENT{NAMESPACE, "SetAgent"};

/// HandOff directive
static const alexaClientSDK::avsCommon::avs::NamespaceAndName HAND_OFF_AUDIO_START{NAMESPACE, "HandOffAudioWillStart"};

static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getCoAssistantCapabilityConfigurations();
/*
* Creates capability configuration
*/
std::shared_ptr<CoAssistantCustomDirectiveCapabilityAgent> CoAssistantCustomDirectiveCapabilityAgent::create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
std::weak_ptr<CoAssistantActiveAgentDelegateInterface> coassistantActiveAgentDelegate )
{
    try
    {
        ThrowIfNull( exceptionSender, "nullExceptionSender" );
        
        auto coassistantCustomDirectiveCapabilityAgent = std::shared_ptr<CoAssistantCustomDirectiveCapabilityAgent>( new CoAssistantCustomDirectiveCapabilityAgent( exceptionSender , coassistantActiveAgentDelegate ) );

        ThrowIfNull ( coassistantCustomDirectiveCapabilityAgent, "nullCoAssistantCapabilityAgent");
        
        return coassistantCustomDirectiveCapabilityAgent;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void CoAssistantCustomDirectiveCapabilityAgent::handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) {
    preHandleDirective( std::make_shared<DirectiveInfo>( directive, nullptr ) );
}

void CoAssistantCustomDirectiveCapabilityAgent::preHandleDirective( std::shared_ptr<DirectiveInfo> info )
{
    try
    {
        ThrowIfNot( info && info->directive, "nullDirectiveInfo" );
        
        if( info->directive->getName() == SET_AGENT.name ) {
            handleSetAgentDirective( info );
        } else if (info->directive->getName() == HAND_OFF_AUDIO_START.name ) {
            handleHandOffAudioStartDirective( info );
        }
        else {
            handleUnknownDirective( info );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"preHandleDirective").d("reason", ex.what()));
    }
}

void CoAssistantCustomDirectiveCapabilityAgent::handleDirective( std::shared_ptr<DirectiveInfo> info ) {
    // Do nothing here as directives are handled in the preHandle stage.
}

void CoAssistantCustomDirectiveCapabilityAgent::cancelDirective( std::shared_ptr<DirectiveInfo> info ) {
    removeDirective( info );
}

void CoAssistantCustomDirectiveCapabilityAgent::removeDirective( std::shared_ptr<DirectiveInfo> info ) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if( info->directive && info->result ) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective( info->directive->getMessageId() );
    }
}


alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration CoAssistantCustomDirectiveCapabilityAgent::getConfiguration() const
{
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto blockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy( alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[SET_AGENT] = blockingPolicy;
    return configuration;
}


CoAssistantCustomDirectiveCapabilityAgent::CoAssistantCustomDirectiveCapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::weak_ptr<CoAssistantActiveAgentDelegateInterface> coassistantActiveAgentDelegate) :
    alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
    alexaClientSDK::avsCommon::utils::RequiresShutdown{"CoAssistantCustomDirectiveCapabilityAgent"},
    m_activeAgentDelegate(coassistantActiveAgentDelegate) {
    AACE_INFO(LX(TAG, "CoAssistantCustomDirectiveCapabilityAgent | Constructor called"));    
    m_capabilityConfigurations.insert(getCoAssistantCapabilityConfigurations());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getCoAssistantCapabilityConfigurations() {
    std::unordered_map<std::string, std::string> configMap;

    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, COASSISTANT_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, COASSISTANT_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY, COASSISTANT_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void CoAssistantCustomDirectiveCapabilityAgent::doShutdown() {
    m_executor.shutdown();
}

void CoAssistantCustomDirectiveCapabilityAgent::setHandlingCompleted( std::shared_ptr<DirectiveInfo> info )
{
    if( info && info->result ) {
        info->result->setCompleted();
    }
    
    removeDirective( info );
}

void CoAssistantCustomDirectiveCapabilityAgent::handleSetAgentDirective( std::shared_ptr<DirectiveInfo> info )
{
    m_executor.submit([this, info]() {
        // TODO : set the Agent
        AACE_INFO(LX(TAG, "handleSetAgentDirective | SetAgent directive"));
        AACE_INFO(LX(TAG, "handleSetAgentDirective | SetAgent payload"));
        AACE_INFO(LX(TAG, info->directive->getPayload()));

        // parsing payload to get agent information
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse( info->directive->getPayload().c_str() );
        if( !result ) {
            AACE_ERROR(LX(TAG, "handleSetAgentDirective").d("reason", rapidjson::GetParseError_En(result.Code())).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed( info, "Unable to parse payload", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        if( !document.HasMember( "agent" ) ) {
            AACE_ERROR(LX(TAG, "handleSetAgentDirective").d("reason", "missing agent"));
            sendExceptionEncounteredAndReportFailed( info, "Missing agent", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        auto agent = document["agent"].GetString();
        AACE_INFO(LX(TAG, "handleSetAgentDirective").d("agent ", agent));
        m_activeAgentDelegate->activeAgentChanged( agent );
        setHandlingCompleted( info );
    });
}

void CoAssistantCustomDirectiveCapabilityAgent::handleHandOffAudioStartDirective (std::shared_ptr<DirectiveInfo> info )
{
    m_executor.submit([this, info]() {
        AACE_INFO(LX(TAG, "handleHandOffAudioStartDirective"));
        AACE_INFO(LX(TAG, info->directive->getPayload()));

        // parsing payload to get agent information
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse( info->directive->getPayload().c_str() );
        if( !result ) {
            AACE_ERROR(LX(TAG, "handleHandOffAudioStartDirective").d("reason", rapidjson::GetParseError_En(result.Code())).d("messageId", info->directive->getMessageId()));
            sendExceptionEncounteredAndReportFailed( info, "Unable to parse payload", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        if( !document.HasMember( "toAgent" ) ) {
            AACE_ERROR(LX(TAG, "handleHandOffAudioStartDirective").d("reason", "missing toAgent"));
            sendExceptionEncounteredAndReportFailed( info, "Missing toAgent", alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
            return;
        }

        auto agent = document["toAgent"].GetString();
        AACE_INFO(LX(TAG, "handleHandOffAudioStartDirective").d("toAgent ", agent));
        //TODO : communicate to apps via platfrom api that handoff will start so that they can disable ww detection.
        setHandlingCompleted( info );
    });
}

void CoAssistantCustomDirectiveCapabilityAgent::sendExceptionEncounteredAndReportFailed( std::shared_ptr<DirectiveInfo> info, const std::string& message, alexaClientSDK::avsCommon::avs::ExceptionErrorType type )
{
    m_exceptionEncounteredSender->sendExceptionEncountered( info->directive->getUnparsedDirective(), type, message );
    
    if( info && info->result ) {
        info->result->setFailed( message );
    }
    
    removeDirective( info );
}

void CoAssistantCustomDirectiveCapabilityAgent::handleUnknownDirective( std::shared_ptr<DirectiveInfo> info )
{
    AACE_ERROR(LX(TAG,"handleDirectiveFailed")
                    .d("reason", "unknownDirective")
                    .d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));

    m_executor.submit([this, info] {
        const std::string exceptionMessage =
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();

        sendExceptionEncounteredAndReportFailed( info, exceptionMessage, alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED );
    });
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> CoAssistantCustomDirectiveCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

} // aace::engine::coassistant
} // aace::engine
} // aace