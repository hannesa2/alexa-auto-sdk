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

#ifndef AACE_LOCAL_VOICE_CONTROL_PLATFORM_SERVICES_RUNNER_H
#define AACE_LOCAL_VOICE_CONTROL_PLATFORM_SERVICES_RUNNER_H

#include <future>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/LocalVoiceControl/AuthImpl.h"
#include "AACE/Engine/LocalVoiceControl/LocaleImpl.h"
#include "AACE/Engine/LocalVoiceControl/EndpointImpl.h"
#include "AACE/Engine/LocalVoiceControl/ConnectivityImpl.h"
#include "alexa_hybrid/sdk/IStartable.h"
#include "alexa_hybrid/sdk/IPlatformServicesEndpoint.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

/**
 * PlatformServices lifecycle manager. Helps start the PlatformServices.
 */
class PlatformServicesRunner : public AlexaHybrid::SDK::IStartableObserver {
public:
    /**
     * Constructor
     *
     * @param platformServicesEndpoint Platform Services endpoint
     */
    explicit PlatformServicesRunner(
            std::shared_ptr<AlexaHybrid::SDK::IPlatformServicesEndpoint> platformServicesEndpoint)
            : m_platformServicesEndpoint{platformServicesEndpoint} {
    }

    ~PlatformServicesRunner() = default;

    void onStart() override;

    void onStop() override;

    void onFailure(const std::string &reason, bool isRecoverable) override;

    /**
     * Starts the Platform Services. Returns after startup is complete.
     */
    void run();

    void setAuth(std::shared_ptr<AuthImpl> auth);

    void setConnectivity(std::shared_ptr<ConnectivityImpl> connectivity);

    void setLocale(std::shared_ptr<LocaleImpl> locale);

    void setEndpoint(std::shared_ptr<EndpointImpl> endpoint);

    /**
     * Stops AHE IPC client.
     */
    void stop();

    /**
     * Clean up platform services runner.
     */
    void shutdown();

private:
    std::shared_ptr<AlexaHybrid::SDK::IPlatformServicesEndpoint> m_platformServicesEndpoint;
    std::shared_ptr<AuthImpl> m_auth;
    std::shared_ptr<ConnectivityImpl> m_connectivity;
    std::shared_ptr<LocaleImpl> m_locale;
    std::shared_ptr<EndpointImpl> m_endpoint;
    std::promise<void> m_stopPromise;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_PLATFORM_SERVICES_RUNNER_H