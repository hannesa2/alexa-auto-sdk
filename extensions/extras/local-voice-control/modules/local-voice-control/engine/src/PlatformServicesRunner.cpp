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
    AACE_DEBUG(LX(TAG, "Notification from IPlatformServicesServicesEndpoint that it is running."));

    // TODO Uncomment once connectivity is implemented
    // if (m_connectivity)
    // {
    //     auto result = m_connectivity->isConnected();
    //     if (result)
    //     {
    //         AACE_DEBUG(LX(TAG, "Sending initial connectivity status")
    //                         .d("isConnected", result.get()));
    //         m_connectivity->onConnectivityChanged(result.get());
    //     }
    //     else
    //     {
    //         AACE_WARN(LX(TAG, "Cannot send initial connectivity status")
    //                        .d("reason", result.getErrorReason()));
    //     }
    // }

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
        auto result = m_locale->getLocales();
        if (result)
        {
            AACE_DEBUG(LX(TAG, "Sending the initial locale") .d("locale", result.get().front()));
            m_locale->onLocaleChange(result.get().front());
        }
        else
        {
            AACE_WARN(LX(TAG, "Cannot get the initial locale") .d("reason", result.getErrorReason()));
        }
    }

    if (m_endpoint)
    {
        auto result = m_endpoint->getEndpoints();

        if (result)
        {
            // TODO
            AACE_INFO(LX(TAG, "Sending initial endpoint") .d("endpoint", result.get().begin()->second));
            // m_endpoint->onEndpointChange(result.get().begin()->second);
        }
        else
        {
            AACE_WARN(LX(TAG, "Cannot send initial endpoint") .d("reason", result.getErrorReason()));
        }
    }
}

void PlatformServicesRunner::onStop()
{
    AACE_INFO(LX(TAG, "Notification from IPlatformServicesServicesEndpoint that it has stopped."));
}

void PlatformServicesRunner::onFailure(const std::string &reason, bool isRecoverable)
{
    AACE_INFO(LX(TAG, "Notification from IPlatformServicesServicesEndpoint that it has failed: " + reason));
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

void PlatformServicesRunner::setAuth(std::shared_ptr<AuthImpl> auth)
{
    m_auth = auth;
}

// TODO Uncomment when implemented
// void setConnectivity(std::shared_ptr<ConnectivityImpl> connectivity) {
//     m_Connectivity = connectivity;
// }

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