/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <ACL/Transport/HTTP2TransportFactory.h>
#include <ACL/Transport/MessageRouter.h>
#include <ACL/Transport/PostConnectSequencerFactory.h>
#include <alexa_hybrid/router/integration/HybridMessageRouter.h>
#include <alexa_hybrid/router/integration/HybridMessageRouterFactory.h>
#include <alexa_hybrid/sdk/IPlatformServicesEndpoint.h>
#include <AVSCommon/Utils/AudioFormat.h>
#include <AVSCommon/Utils/LibcurlUtils/LibcurlHTTP2ConnectionFactory.h>
#include <Endpoints/EndpointBuilder.h>
#include <Settings/DeviceSettingsManager.h>

#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/LocalVoiceControl/AuthImpl.h"
#include "AACE/Engine/LocalVoiceControl/ClockImpl.h"
#include "AACE/Engine/LocalVoiceControl/ConfigurationImpl.h"
#include "AACE/Engine/LocalVoiceControl/ConnectivityImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/ConnectivityImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceAdapterImpl.h"
#include "AACE/Engine/LocalVoiceControl/DummyAuthDelegate.h"
#include "AACE/Engine/LocalVoiceControl/LocaleImpl.h"
#include "AACE/Engine/LocalVoiceControl/LocalVoiceControlEngineService.h"
#include "AACE/Engine/LocalVoiceControl/MetricsEmitterImpl.h"
#include "AACE/Engine/LocalVoiceControl/PlatformServicesRunner.h"
#include <AACE/Engine/Network/NetworkObservableInterface.h>
#include "AACE/Engine/Utils/Metrics/Metrics.h"

#include <nlohmann/json.hpp>

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/pointer.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

namespace aace {
namespace engine {
namespace localVoiceControl {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.LocalVoiceControlEngineService");

/// Message router socket file name
static const std::string MESSAGE_ROUTER_SOCKET_FILE_NAME("hec-http2.sock");

// String to identify metrics emitted from this file
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "LocalVoiceControlEngineService";

// register the service
REGISTER_SERVICE(LocalVoiceControlEngineService)

LocalVoiceControlEngineService::LocalVoiceControlEngineService(
    const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
    m_configuration = std::make_shared<ConfigurationImpl>();
}

bool LocalVoiceControlEngineService::initialize() {
    try {
        AACE_INFO(LX(TAG));

        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        // Add listener to LOCALE property
        ThrowIfNot(
            propertyManager->addListener(aace::alexa::property::LOCALE, shared_from_this()), "failedToAddListener");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        AACE_INFO(LX(TAG));

        rapidjson::IStreamWrapper isw(*configuration);
        rapidjson::Document document;
        rapidjson::Document lvcDocument;

        document.ParseStream(isw);

        ThrowIf(document.HasParseError(), GetParseError_En(document.GetParseError()));

        rapidjson::Value* ecDirectory = nullptr;
        rapidjson::Value* psDirectory = nullptr;
        rapidjson::Value* ecPerms = nullptr;
        rapidjson::Value* psPerms = nullptr;
        rapidjson::Value* address = nullptr;
        rapidjson::Value* mrDirectory = nullptr;

        // Look for config file path or directories passed in explicitly
        auto filePath = GetValueByPointer(document, "/configFilePath");

        // If the file path is given, parse the file
        if (filePath) {
            auto config = aace::core::config::ConfigurationFile::create(filePath->GetString());
            auto configStream = config->getStream();

            ThrowIfNull(configStream, "Unable to open local voice control config file");
            rapidjson::IStreamWrapper iswLVC(*configStream);

            lvcDocument.ParseStream(iswLVC);
            ThrowIf(lvcDocument.HasParseError(), GetParseError_En(lvcDocument.GetParseError()));

            // Retrieve the values inside the lvc file
            ecDirectory = rapidjson::Pointer("/" + ConfigurationImpl::EXECUTION_CONTROLLER_DIRECTORY).Get(lvcDocument);
            ecPerms = rapidjson::Pointer("/" + ConfigurationImpl::EXECUTION_CONTROLLER_PERMISSIONS).Get(lvcDocument);
            psDirectory = rapidjson::Pointer("/" + ConfigurationImpl::PLATFORM_SERVICES_DIRECTORY).Get(lvcDocument);
            psPerms = rapidjson::Pointer("/" + ConfigurationImpl::PLATFORM_SERVICES_PERMISSIONS).Get(lvcDocument);
            address = rapidjson::Pointer("/" + ConfigurationImpl::ADDRESS).Get(lvcDocument);
            mrDirectory = rapidjson::Pointer("/" + ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY).Get(lvcDocument);
        } else {
            // Find the platform services socket directory, permissions and controller address, uds directory
            ecDirectory = GetValueByPointer(document, "/controllerSocketDirectory");
            ecPerms = GetValueByPointer(document, "/controllerSocketPermissions");
            psDirectory = GetValueByPointer(document, "/platformSocketDirectory");
            psPerms = GetValueByPointer(document, "/platformSocketPermissions");
            address = GetValueByPointer(document, "/address");
            mrDirectory = GetValueByPointer(document, "/messageRouterSocketDirectory");
        }

        ThrowIfNot(ecDirectory, "missing Execution Controller directory");
        AACE_DEBUG(LX(TAG).d("ecDirectory", ecDirectory->GetString()));
        m_configuration->setProperty(ConfigurationImpl::EXECUTION_CONTROLLER_DIRECTORY, ecDirectory->GetString());

        ThrowIfNot(ecPerms, "missing Execution Controller permissions");
        AACE_DEBUG(LX(TAG).d("ecPerms", ecPerms->GetString()));
        m_configuration->setProperty(ConfigurationImpl::EXECUTION_CONTROLLER_PERMISSIONS, ecPerms->GetString());

        ThrowIfNot(psDirectory, "missing Platform Services directory");
        AACE_DEBUG(LX(TAG).d("psDirectory", psDirectory->GetString()));
        m_configuration->setProperty(ConfigurationImpl::PLATFORM_SERVICES_DIRECTORY, psDirectory->GetString());

        ThrowIfNot(psPerms, "missing Platform Services permissions");
        AACE_DEBUG(LX(TAG).d("psPerms", psPerms->GetString()));
        m_configuration->setProperty(ConfigurationImpl::PLATFORM_SERVICES_PERMISSIONS, psPerms->GetString());

        // Address is not required
        if (address) {
            AACE_DEBUG(LX(TAG).d("address", address->GetString()));
            m_configuration->setProperty(ConfigurationImpl::ADDRESS, address->GetString());
        }

        ThrowIfNot(mrDirectory, "missing Message Router directory");
        AACE_DEBUG(LX(TAG).d("mrDirectory", mrDirectory->GetString()));
        m_configuration->setProperty(ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY, mrDirectory->GetString());

        configuration->clear();
        configuration->seekg(0);

        // Register the MessageRouterInterface factory. This requires that
        // 1. AlexaEngineService registered the service interfaces needed for HybridMessageRouter during its own
        //    configure() implementation.
        // 2. AlexaEngineService invokes the factory only after configure()

        auto alexaEngineService = getContext()->getService<aace::engine::alexa::AlexaEngineService>();
        ThrowIfNull(alexaEngineService, "AlexaEngineService is null");

        auto messageRouterFactory = [this]() {
            AACE_DEBUG(LX(TAG, "Creating factory for HybridMessageRouter"));
            try {
                // Get the Alexa component interface, which exposes all service interfaces
                auto alexaComponentInterface =
                    this->getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
                ThrowIfNull(alexaComponentInterface, "Alexa component interface is null");

                // Get all service interfaces needed to create hybrid router

                auto attachmentManager = alexaComponentInterface->getAttachmentManager();
                ThrowIfNull(attachmentManager, "Attachment manager is null");

                auto authDelegate = alexaComponentInterface->getAuthDelegate();
                ThrowIfNull(authDelegate, "Auth delegate is null");

                auto deviceInfo = alexaComponentInterface->getDeviceInfo();
                ThrowIfNull(deviceInfo, "Device info is null");

                auto postConnectSequencerFactory = alexaComponentInterface->getPostConnectSequencerFactory();
                ThrowIfNull(postConnectSequencerFactory, "Post connect sequencer factory is null");

                auto transportFactory = alexaComponentInterface->getTransportFactory();
                ThrowIfNull(transportFactory, "Transport factory is null");

                auto deviceSerialNumber = deviceInfo->getDeviceSerialNumber();
                m_connectivityAdapter = std::make_shared<deviceAdapter::ConnectivityImpl>();
                m_metricsEmitter = std::make_shared<MetricsEmitterImpl>();

                auto deviceAdapterImpl =
                    std::unique_ptr<deviceAdapter::DeviceAdapterImpl>(new deviceAdapter::DeviceAdapterImpl(
                        deviceSerialNumber,
                        m_connectivityAdapter,
                        m_metricsEmitter,
                        [&, alexaComponentInterface]() {
                            std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> deviceSettingsManager =
                                nullptr;
                            if (alexaComponentInterface != nullptr) {
                                deviceSettingsManager = alexaComponentInterface->getDeviceSettingsManager();
                            }
                            return deviceSettingsManager;
                        },
                        this->getContext()));

                // Create the path to the hybrid message router socket using the directory from configuration

                std::string messageRouterDir =
                    m_configuration->getProperty(ConfigurationImpl::MESSAGE_ROUTER_DIRECTORY);
                std::string udsSockFilePath = "";
                ThrowIf(messageRouterDir.empty(), "Could not retrieve configured message router directory");
                // Avoid appending an extra slash to the file path
                std::string udsSockFileName = messageRouterDir.back() == '/' ? MESSAGE_ROUTER_SOCKET_FILE_NAME
                                                                             : '/' + MESSAGE_ROUTER_SOCKET_FILE_NAME;
                udsSockFilePath = messageRouterDir + udsSockFileName;
                AACE_DEBUG(LX(TAG, "Setting UDS path").sensitive("udsSockFilePath", udsSockFilePath));

                // clang-format off
                std::string hrJsonConfig = nlohmann::json({
                    {"Alexa.HybridRouter.AlexaHybridEngineEndpoint",{
                        {"type","uds"},
                        {"path",udsSockFilePath}
                    }}
                }).dump();
                // clang-format on

                auto hybridMessageRouterFactory =
                    std::make_shared<AlexaHybrid::SDK::Router::Integration::HybridMessageRouterFactory>(
                        postConnectSequencerFactory, std::move(deviceAdapterImpl), hrJsonConfig);
                ThrowIfNull(hybridMessageRouterFactory, "Failed to create HybridMessageRouterFactory");

                auto hybridMessageRouter =
                    hybridMessageRouterFactory->createMessageRouter(authDelegate, attachmentManager, transportFactory);
                ThrowIfNull(hybridMessageRouter, "Failed to create HybridMessageRouter");
                return hybridMessageRouter;
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG).d("reason", ex.what()));
                return std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface>(nullptr);
            }
        };

        // Register the hybrid message router factory
        ThrowIfNot(
            alexaEngineService->registerServiceFactory<alexaClientSDK::acl::MessageRouterInterface>(
                messageRouterFactory),
            "Unable to register hybrid proxy message router factory");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        configuration->clear();
        configuration->seekg(0);
        return false;
    }
}

bool LocalVoiceControlEngineService::setup() {
    AACE_INFO(LX(TAG));
    try {
        // Get alexa engine service
        auto alexaComponentInterface =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponentInterface, "AlexaComponentInterface is null");

        // Get the currently set AVS Gateway
        auto alexaEndpoints =
            this->getContext()->getServiceInterface<aace::engine::alexa::AlexaEndpointInterface>("aace.alexa");
        auto avsGateway = alexaEndpoints->getAVSGateway() /*"https://127.0.0.1:9999"*/;
        ThrowIf(avsGateway == std::string(), "AVS Gateway is not set");
        AACE_VERBOSE(LX(TAG).d("avsGateway", avsGateway));

        // Create the IEndpoint instance
        m_endpoint = std::make_shared<EndpointImpl>(avsGateway);
        ThrowIfNull(m_endpoint, "EndpointImpl create failed");

        // Get AVS Gateway Manager
        auto avsGatewayManager = alexaComponentInterface->getAVSGatewayManager();
        ThrowIfNull(avsGatewayManager, "AVS Gateway Manager is null");
        avsGatewayManager->addObserver(m_endpoint);

        // Initialize the connectivity device adapter with the NetworkInfoProvider
        ThrowIfNull(m_connectivityAdapter, "connectivity adapter is null");
        auto networkProvider =
            getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>("aace.network");
        m_connectivityAdapter->setNetworkProvider(networkProvider);
        emitBufferedMetrics(METRIC_PROGRAM_NAME_SUFFIX, "setup", {{"AutoSDK.Extension.LocalVoiceControl", 1}});
        
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "setup").d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::start() {
    AACE_INFO(LX(TAG, "LocalVoiceControl start"));
    try {
        auto alexaComponentInterface =
            getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>("aace.alexa");
        ThrowIfNull(alexaComponentInterface, "AlexaComponentInterface is null");

        // Initialize AuthImpl
        auto auth = std::make_shared<AuthImpl>();
        auth->setAuthDelegate(alexaComponentInterface->getAuthDelegate());

        // Get network engine service
        auto networkObserver =
            getContext()->getServiceInterface<aace::engine::network::NetworkObservableInterface>("aace.network");

        // Initialize ConnectivityImpl
        auto connectivity = std::make_shared<ConnectivityImpl>();

        if (networkObserver != nullptr) {
            // Register connectivity object as network info observer
            networkObserver->addObserver(connectivity);
        } else {
            // NetworkInfoProvider interface is not registered
            AACE_DEBUG(LX(TAG).m("networkObserverNotAvailable"));
        }

        // Initialize ClockImpl
        auto clock = std::make_shared<ClockImpl>();

        // Initialize Metrics Emitter
        ThrowIfNull(m_metricsEmitter, "no metrics emitter");
        auto metricsEmitter = m_metricsEmitter;

        auto localSkillService = getContext()->getServiceInterface<aace::engine::localSkillService::LocalSkillServiceInterface>("aace.localSkillService");
        ThrowIfNull(localSkillService, "no localSkillService");

        localSkillService->registerHandler(
            "/metrics/emit",
            [metricsEmitter](
                std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response) -> bool {
                if (!request) return false;
                return metricsEmitter->emitHandler(*request);
            });

        auto propertyManager =
            getContext()->getServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(
                "aace.propertyManager");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        // Initialize Locale
        auto locale = propertyManager->getProperty(aace::alexa::property::LOCALE);
        ThrowIf(locale == std::string(), "Locale is null");
        AACE_INFO(LX(TAG, "LocalVoiceControlEngineService::start").d("locale", locale));
        m_locale = std::make_shared<LocaleImpl>(locale);
        ThrowIfNull(m_locale, "m_locale is null");

        // Create AHE platform services endpoint
        std::shared_ptr<AlexaHybrid::SDK::IPlatformServicesEndpoint> platformServicesEndpoint =
            AlexaHybrid::SDK::IPlatformServicesEndpoint::create();

        // Configure interface implementations on platform services endpoint
        platformServicesEndpoint->setConfiguration(m_configuration);
        platformServicesEndpoint->setAuthInterface(auth);
        platformServicesEndpoint->setConnectivityInterface(connectivity);
        platformServicesEndpoint->setClockInterface(clock);
        platformServicesEndpoint->setMetricsEmitter(m_metricsEmitter);
        platformServicesEndpoint->setLocaleInterface(m_locale);
        platformServicesEndpoint->setEndpointInterface(m_endpoint);

        // Create platform services runner
        m_platformServicesRunner = std::make_shared<PlatformServicesRunner>(platformServicesEndpoint);
        ThrowIfNull(m_platformServicesRunner, "PlatformServicesRunner is null");

        // Configure platform services runner
        platformServicesEndpoint->setObserver(m_platformServicesRunner);
        m_platformServicesRunner->setAuth(auth);
        m_platformServicesRunner->setConnectivity(connectivity);
        m_platformServicesRunner->setLocale(m_locale);
        m_platformServicesRunner->setEndpoint(m_endpoint);

        // Start IPC endpoint
        AACE_INFO(LX(TAG, "Starting IPC endpoint"));
        m_platformServicesRunner->run();

        auto networkProvider = getContext()->getServiceInterface<aace::network::NetworkInfoProvider>("aace.network");
        auto networkStatus = networkProvider != nullptr ? networkProvider->getNetworkStatus()
                                                        : aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED;
        connectivity->onNetworkInfoChanged(networkStatus, 0);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "start").d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::stop() {
    AACE_INFO(LX(TAG));
    try {
        AACE_DEBUG(LX(TAG));
        m_platformServicesRunner->stop();
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "stop").d("reason", ex.what()));
        return false;
    }
}

bool LocalVoiceControlEngineService::shutdown() {
    AACE_DEBUG(LX(TAG));

    // If start() is not called then this will not be initalized
    if (m_platformServicesRunner != nullptr) {
        m_platformServicesRunner->shutdown();
        m_platformServicesRunner.reset();
    }

    m_configuration.reset();
    m_locale.reset();
    m_endpoint.reset();

    return true;
}

void LocalVoiceControlEngineService::propertyChanged(const std::string& key, const std::string& newValue) {
    AACE_INFO(LX(TAG));
    updateLocale(newValue);
}

void LocalVoiceControlEngineService::updateLocale(const std::string& value) {
    AACE_INFO(LX(TAG).sensitive("value", value));
    if (m_locale) {
        m_locale->onLocaleChange(value);
    }
}

}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
