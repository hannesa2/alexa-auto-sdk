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

#include <ACL/Transport/AHEPostConnectSequencerFactory.h>
#include <ACL/Transport/HTTP2TransportFactory.h>
#include <ACL/Transport/HybridProxyMessageRouter.h>
#include <AVSCommon/Utils/LibcurlUtils/LibcurlHTTP2ConnectionFactory.h>
#include <Endpoints/EndpointBuilder.h>

#include "AACE/Engine/LocalVoiceControl/LocalVoiceControlEngineService.h"
#include "AACE/Engine/LocalVoiceControl/AuthImpl.h"
#include "AACE/Engine/LocalVoiceControl/ClockImpl.h"
#include "AACE/Engine/LocalVoiceControl/ConnectivityImpl.h"
#include "AACE/Engine/LocalVoiceControl/ConfigurationImpl.h"
#include "AACE/Engine/LocalVoiceControl/DummyAuthDelegate.h"
#include "AACE/Engine/LocalVoiceControl/MetricsEmitterImpl.h"
#include "AACE/Engine/LocalVoiceControl/LocaleImpl.h"
#include "AACE/Engine/LocalVoiceControl/PlatformServicesRunner.h"
#include "AACE/Alexa/AlexaProperties.h"
#include "alexa_hybrid/sdk/IPlatformServicesEndpoint.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Core/EngineConfiguration.h"

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.LocalVoiceControlEngineService");

// Default values for local voice control local ip address and message router socket file name
static const std::string LOCAL_IP_ADDRESS("127.0.0.1");
static const std::string MESSAGE_ROUTER_SOCKET_FILE_NAME("hec-http2.sock");

// register the service
REGISTER_SERVICE(LocalVoiceControlEngineService)

LocalVoiceControlEngineService::LocalVoiceControlEngineService(
    const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
    m_configuration = std::make_shared<ConfigurationImpl>();
}

bool LocalVoiceControlEngineService::initialize()
{
    AACE_INFO(LX(TAG, "LocalVoiceControl initializing"));
    // Get the Alexa Engine Service
    auto alexaEngineService = getContext()->getService<aace::engine::alexa::AlexaEngineService>();
    ThrowIfNull(alexaEngineService, "AlexaEngineService is null");

    // Create a custom function that creates a hybrid proxy message router
    auto messageRouterFactory = [this] () {
        // Get the Alexa component interface which exposes all service interfaces 
        auto alexaComponentInterface = this->getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull(alexaComponentInterface, "alexaComponentInterface is null");

        // Get all services interfaces needed to create hybrid proxy
        auto capabilitiesDelegate = alexaComponentInterface->getCapabilitiesDelegate();
        auto attachmentManager    = alexaComponentInterface->getAttachmentManager();
        auto transportFactory     = alexaComponentInterface->getTransportFactory();
        auto authDelegate         = alexaComponentInterface->getAuthDelegate();

        // Create AHE post connect, dummy auth delegate, and transport factory objects
        auto ahePostConnectSequencerFactory = alexaClientSDK::acl::AHEPostConnectSequencerFactory::create(
            capabilitiesDelegate);
        auto aheAuthDelegate = std::make_shared<DummyAuthDelegate>();
        auto aheTransportFactory = std::make_shared<alexaClientSDK::acl::HTTP2TransportFactory>(
            std::make_shared<alexaClientSDK::avsCommon::utils::libcurlUtils::LibcurlHTTP2ConnectionFactory>(true),
            ahePostConnectSequencerFactory);

        // Retrieve AVS endpoint
        auto alexaEndpoints = this->getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>( "aace.alexa" );
        auto avsEndpoint = alexaEndpoints->getAVSGateway();
    
        // Return hybrid proxy message router instance
        // Since the actual directory for the uds socket file is configurable,
        // we will leave it empty here but set the actual configured value during setup() using setAHEEndpoint()
        return std::make_shared<alexaClientSDK::acl::HybridProxyMessageRouter>(
            authDelegate,
            aheAuthDelegate,
            attachmentManager,
            transportFactory,
            aheTransportFactory,
            avsEndpoint,
            LOCAL_IP_ADDRESS,
            "");
    };

    // Register hybrid proxy message router factory
    ThrowIfNot( alexaEngineService->registerServiceFactory <alexaClientSDK::acl::MessageRouterInterface>(messageRouterFactory), "Unable to register hybrid proxy message router factory" );

    auto propertyManager =
        getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
            "aace.propertyManager");
    ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

    // Add listener to LOCALE property
    ThrowIfNot(propertyManager->addListener(aace::alexa::property::LOCALE, shared_from_this()), "failedToAddListener");
    return true;
}

bool LocalVoiceControlEngineService::configure(std::shared_ptr<std::istream> configuration)
{
    try
    {
        bool handled = false;

        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;
        rapidjson::Document lvcDocument;
        
        document.ParseStream( isw );
        
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );

        rapidjson::Value* ecDirectory  = nullptr;
        rapidjson::Value* psDirectory  = nullptr;
        rapidjson::Value* ecPerms      = nullptr;
        rapidjson::Value* psPerms      = nullptr;
        rapidjson::Value* address      = nullptr;
        rapidjson::Value* mrDirectory  = nullptr;

        // Look for config file path or directories passed in explicitly
        auto filePath = GetValueByPointer( document, "/configFilePath" );

        // If the file path is given, parse the file 
        if (filePath) {
            auto config = aace::core::config::ConfigurationFile::create(filePath->GetString());
            auto configStream = config->getStream();

            ThrowIfNull(configStream, "Unable to open local voice control config file");
            rapidjson::IStreamWrapper iswLVC( *configStream );

            lvcDocument.ParseStream( iswLVC );
            ThrowIf( lvcDocument.HasParseError(), GetParseError_En( lvcDocument.GetParseError() ) );

            // Retrieve the values inside the lvc file
            ecDirectory = rapidjson::Pointer("/" + ConfigurationImpl::EXECUTION_CONTROLLER_DIRECTORY  ).Get(lvcDocument);
            ecPerms     = rapidjson::Pointer("/" + ConfigurationImpl::EXECUTION_CONTROLLER_PERMISSIONS).Get(lvcDocument);
            psDirectory = rapidjson::Pointer("/" + ConfigurationImpl::PLATFORM_SERVICES_DIRECTORY     ).Get(lvcDocument);
            psPerms     = rapidjson::Pointer("/" + ConfigurationImpl::PLATFORM_SERVICES_PERMISSIONS   ).Get(lvcDocument);
            address     = rapidjson::Pointer("/" + ConfigurationImpl::ADDRESS                         ).Get(lvcDocument);
            mrDirectory = rapidjson::Pointer("/" + ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY        ).Get(lvcDocument);
        } else {
            // Find the platform services socket directory, permissions and controller address, uds directory
            ecDirectory = GetValueByPointer( document, "/controllerSocketDirectory"    );
            ecPerms     = GetValueByPointer( document, "/controllerSocketPermissions"  );
            psDirectory = GetValueByPointer( document, "/platformSocketDirectory"      );
            psPerms     = GetValueByPointer( document, "/platformSocketPermissions"    );
            address     = GetValueByPointer( document, "/address"                      );
            mrDirectory = GetValueByPointer( document, "/messageRouterSocketDirectory" );
        }

        if (ecDirectory) {
            m_configuration->setProperty(ConfigurationImpl::EXECUTION_CONTROLLER_DIRECTORY, ecDirectory->GetString());
            handled = true;
        }

        if (ecPerms) {
            m_configuration->setProperty(ConfigurationImpl::EXECUTION_CONTROLLER_PERMISSIONS, ecPerms->GetString());
            handled = true;
        }

        if (psDirectory) {
            m_configuration->setProperty(ConfigurationImpl::PLATFORM_SERVICES_DIRECTORY, psDirectory->GetString());
            handled = true;
        }
        
        if (psPerms) {
            m_configuration->setProperty(ConfigurationImpl::PLATFORM_SERVICES_PERMISSIONS, psPerms->GetString());
            handled = true;
        }

        if (address) {
            m_configuration->setProperty(ConfigurationImpl::ADDRESS, address->GetString());
            handled = true;
        }

        if (mrDirectory) {
            m_configuration->setProperty(ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY, mrDirectory->GetString());
            handled = true;
        }

        configuration->clear();
        configuration->seekg( 0 );

        if (handled) {
            AACE_INFO(LX(TAG, "LocalVoiceControl configure found options"));
        }
        return handled;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG,"configure").d("reason", ex.what()));
        configuration->clear();
        configuration->seekg( 0 );
        return false;
    }
}

bool LocalVoiceControlEngineService::setup()
{
    try
    {
        AACE_INFO( LX( TAG, "Setup" ));
        // Get alexa engine service
        auto alexaComponentInterface = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponentInterface, "AlexaComponentInterface is null" );

        // Get capabilities delegate 
        auto capabilitiesDelegate = alexaComponentInterface->getCapabilitiesDelegate();
        ThrowIfNull( capabilitiesDelegate, "Capabilities delegate is null" );

        // Get hybrid proxy
        auto messageRouter = std::static_pointer_cast<alexaClientSDK::acl::HybridProxyMessageRouter>( alexaComponentInterface->getMessageRouter() );
        ThrowIfNull( messageRouter, "HP MessageRouter is null" );

        // Get exception sender
        auto exceptionSender = alexaComponentInterface->getExceptionEncounteredSender();
        ThrowIfNull( exceptionSender, "Exception sender is null" );

        // Get the currently set AVS Gateway
        auto alexaEndpoints = this->getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>( "aace.alexa" );
        auto avsGateway = alexaEndpoints->getAVSGateway();
        ThrowIf( avsGateway == std::string(), "AVS Gateway is not set" );
        AACE_VERBOSE(LX(TAG).d("avsGateway", avsGateway));

        // Create the IEndpoint instance
        m_endpoint = std::make_shared<EndpointImpl>(avsGateway);
        ThrowIfNull( m_endpoint, "EndpointImpl create failed");

        // Get AVS Gateway Assigner
        auto avsGatewayAssigner = alexaComponentInterface->getAVSGatewayAssigner();
        ThrowIfNull( avsGatewayAssigner, "AVS Gateway Assigner is null" );
        avsGatewayAssigner->addGatewayObserver(m_endpoint);

        // Create Hybrid Proxy capability agent to provide the PreferLocal capability configuration
        m_hybridProxyCapabilityAgent = alexaClientSDK::capabilityAgents::hybridProxy::HybridProxyCapabilityAgent::create(exceptionSender, messageRouter);

        // Get LVC address and uds socket file path
        std::string address = m_configuration->getProperty( ConfigurationImpl::ADDRESS, LOCAL_IP_ADDRESS );
        std::string mrDirectory = m_configuration->getProperty( ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY );
        std::string udsSockFilePath = "";

        // Set local endpoint for message router
        if ( !mrDirectory.empty() ) {
            AACE_DEBUG(LX(TAG, "Using UDS and setting directory path").d("udsSockFilePath", udsSockFilePath));
            // Avoid appending an extra slash to the file path
            std::string udsSockFileName = mrDirectory.back() == '/' ? MESSAGE_ROUTER_SOCKET_FILE_NAME : '/' + MESSAGE_ROUTER_SOCKET_FILE_NAME;
            udsSockFilePath = mrDirectory + udsSockFileName;
        }
        else {
            // Inet socket is only allowed in DEBUG build
            #ifdef DEBUG
                AACE_DEBUG(LX(TAG, "Message router UDS socket directory is not set, fallback to using Inet socket."));
            #else
                Throw( "Configuration with Inet socket is only allowed in debug builds" );
            #endif
        }
        AACE_DEBUG(LX(TAG, "Setting local endpoint").d("address", address));
        messageRouter->setAHEEndpoint( address, udsSockFilePath );
        
        // Register hybrid proxy capability agent with the default endpoint
        auto defaultEndpointBuilder = alexaComponentInterface->getDefaultEndpointBuilder();
        ThrowIfNull( defaultEndpointBuilder, "Default EndpointBuilder is null" );
        defaultEndpointBuilder->withCapability(m_hybridProxyCapabilityAgent, m_hybridProxyCapabilityAgent);

        // Pass DirectiveSequencer to messageRouter so router can tell sequencer if the Directive came from AHE or AVS for AAC to calculate online/offline UPL
        auto directiveSequencer = alexaComponentInterface->getDirectiveSequencer();
        ThrowIfNull( directiveSequencer, "Directive sequencer is null" );
        messageRouter->setDirectiveSequencer(directiveSequencer);

        return true;
    }
    catch (std::exception &ex)
    {
        AACE_ERROR(LX(TAG, "setup").d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::start()
{
    AACE_INFO(LX(TAG, "LocalVoiceControl start"));
    try
    {
        // Get alexa engine service
        auto alexaComponentInterface = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponentInterface, "AlexaComponentInterface is null" );

        // Get capabilities delegate 
        auto capabilitiesDelegate = alexaComponentInterface->getCapabilitiesDelegate();
        ThrowIfNull(capabilitiesDelegate, "Capabilities delegate is null");

        // Get hybrid proxy
        auto messageRouter = std::static_pointer_cast<alexaClientSDK::acl::HybridProxyMessageRouter>( alexaComponentInterface->getMessageRouter() );
        ThrowIfNull(messageRouter, "HP MessageRouter is null ");

        // Register hybrid proxy as a capabilities listener
        capabilitiesDelegate->addCapabilitiesObserver(messageRouter);

        // Initialize AuthImpl
        auto auth = std::make_shared<AuthImpl>();
        auth->setAuthDelegate(alexaComponentInterface->getAuthDelegate());

        // Get network engine service
        auto networkObserver = getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>( "aace.network" );

        // Initialize ConnectivityImpl
        auto connectivity = std::make_shared<ConnectivityImpl>();

        if( networkObserver != nullptr ) { // This could be null when NetworkInfoProvider interface is not registered.
            // Register connectivity object as network info observer
            networkObserver->addObserver( connectivity );
        } else {
            AACE_DEBUG(LX(TAG).m("networkObserverNotAvailable"));
        }

        // Initialize ClockImpl
        auto clock = std::make_shared<ClockImpl>();

        // Initialize Metrics Emitter
        auto metrics = std::make_shared<MetricsEmitterImpl>();

        auto localSkillService = getContext()->getServiceInterface<aace::engine::localSkillService::LocalSkillServiceEngineService>("aace.localSkillService");
        ThrowIfNull(localSkillService, "no localSkillService");

        localSkillService->registerHandler("/metrics/emit",
            [metrics](std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) -> bool {
                if (!request) return false;
                return metrics->emitHandler(*request);
            }
        );

        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        // Initialize Locale
        auto locale = propertyManager->getProperty(aace::alexa::property::LOCALE);
        ThrowIf( locale == std::string(), "Locale is null" );
        AACE_INFO(LX(TAG, "LocalVoiceControlEngineService::start").d("locale", locale));
        m_locale = std::make_shared<LocaleImpl>(locale);
        ThrowIfNull( m_locale, "m_locale is null");

        // Create AHE platform services endpoint
        std::shared_ptr<AlexaHybrid::SDK::IPlatformServicesEndpoint> endpoint = AlexaHybrid::SDK::IPlatformServicesEndpoint::create();

        // Configure interface implementations on platform services endpoint
        endpoint->setConfiguration(m_configuration);
        endpoint->setAuthInterface(auth);
        endpoint->setConnectivityInterface(connectivity);
        endpoint->setClockInterface(clock);
        endpoint->setMetricsEmitter(metrics);
        endpoint->setLocaleInterface(m_locale);
        endpoint->setEndpointInterface(m_endpoint);

        // Create platform services runner
        auto platformServicesRunner = std::make_shared<PlatformServicesRunner>(endpoint);
        ThrowIfNull( platformServicesRunner, "PlatformServicesRunner is null");

        // Configure platform services runner
        endpoint->setObserver(platformServicesRunner);
        platformServicesRunner->setAuth(auth);
        platformServicesRunner->setConnectivity(connectivity);
        platformServicesRunner->setLocale(m_locale);
        platformServicesRunner->setEndpoint(m_endpoint);

        // Start IPC endpoint
        AACE_INFO(LX(TAG, "Starting IPC endpoint"));
        platformServicesRunner->run();

        // If we start offline, enable AHE. Online start is handled by AlexaEngineService
        auto connectionManager = alexaComponentInterface->getConnectionManager();
        if (!connectionManager->isEnabled()) {
            messageRouter->enableAHE();
        }

        auto networkProvider = getContext()->getServiceInterface<aace::network::NetworkInfoProvider>( "aace.network" );
        auto networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus() : aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED;
        connectivity->onNetworkInfoChanged(networkStatus, 0);

        return true;
    }
    catch (std::exception &ex)
    {
        AACE_ERROR(LX(TAG, "start").d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::stop()
{
    try
    {
        return true;
    }
    catch (std::exception &ex)
    {
        AACE_ERROR(LX(TAG, "stop").d("reason", ex.what()));
        return false;
    }
}

void LocalVoiceControlEngineService::propertyChanged(const std::string& key, const std::string& newValue) {
    updateLocale(newValue);
}

void LocalVoiceControlEngineService::updateLocale(const std::string& value) {
    AACE_INFO(LX(TAG).sensitive("value", value));
    if (m_locale) {
        m_locale->onLocaleChange(value);
    }
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace
