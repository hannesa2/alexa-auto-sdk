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

#include "AACE/Engine/LocalVoiceControl/PlatformServicesRunner.h"

namespace aace{
namespace engine {
namespace localVoiceControl {

static const std::string TAG("aace.localVoiceControl.PlatformServicesRunner");

void PlatformServicesRunner::onStart()
{
    AACE_INFO(LX(TAG, "LVC IPC started"));

    if (m_connectivity) {
        auto result = m_connectivity->isConnected();
        AACE_DEBUG(LX(TAG).m("Sending initial connectivity status").d("isConnected", result.get()));
        m_connectivity->notifyObservers();
    }

    if (m_auth)
    {
        auto result = m_auth->isRegistered();
        if (result)
        {
            AACE_DEBUG(LX(TAG, "Sending initial auth status") .d("isRegistered", result.get()));
            m_auth->onRegistrationChanged(result.get());
        }
        else
        {
            AACE_WARN(LX(TAG, "Cannot send initial auth status") .d("reason", result.getErrorReason()));
        }
    }

    if (m_locale)
    {
        AACE_DEBUG(LX(TAG).m("Sending the initial locale"));
        m_locale->notifyObservers();
    }

    if (m_endpoint)
    {
        AACE_INFO(LX(TAG).m("Sending initial endpoint"));
        m_endpoint->notifyObservers();
    }
}

void PlatformServicesRunner::onStop()
{
    AACE_DEBUG(LX(TAG, "LVC IPC stopped"));
    m_stopPromise.set_value();
}

void PlatformServicesRunner::onFailure(const std::string &reason, bool isRecoverable)
{
    AACE_WARN(LX(TAG).m("LVC IPC failed").d("reason", reason).d("isRecoverable", isRecoverable));
}

/**
 * Starts the PlatformServices Services. Returns after startup is complete.
 */
void PlatformServicesRunner::run()
{
    AACE_DEBUG(LX(TAG, "Calling IPlatformServicesServicesEndpoint->start()"));
    m_platformServicesEndpoint->start();
    AACE_INFO(LX(TAG, "Returned from IPlatformServicesServicesEndpoint->start()"));
}

void PlatformServicesRunner::stop() {
    AACE_DEBUG(LX(TAG));
    // Stop the IPC platform services layer
    m_platformServicesEndpoint->stop();
}

void PlatformServicesRunner::shutdown() {
    // Wait for IPC client to stop
    m_stopPromise.get_future().wait_for(std::chrono::seconds(10));
    m_auth.reset();
    m_connectivity.reset();
    m_endpoint.reset();
    m_locale.reset();
    m_platformServicesEndpoint.reset();
}

void PlatformServicesRunner::setAuth(std::shared_ptr<AuthImpl> auth)
{
    m_auth = auth;
}

void PlatformServicesRunner::setConnectivity(std::shared_ptr<ConnectivityImpl> connectivity) {
    m_connectivity = connectivity;
}

void PlatformServicesRunner::setLocale(std::shared_ptr<LocaleImpl> locale)
{
    m_locale = locale;
}

void PlatformServicesRunner::setEndpoint(std::shared_ptr<EndpointImpl> endpoint)
{
    m_endpoint = endpoint;
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace