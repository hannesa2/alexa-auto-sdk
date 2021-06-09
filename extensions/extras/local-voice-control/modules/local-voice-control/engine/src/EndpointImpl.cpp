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

#include "AACE/Engine/LocalVoiceControl/EndpointImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.EndpointImpl");

EndpointImpl::EndpointImpl(std::string avsGateway) : m_avsGateway(avsGateway) {
    updateEndpointsList(avsGateway);
}

AlexaHybrid::SDK::Result<std::map<std::string, std::string>> EndpointImpl::getEndpoints() const {
    AACE_DEBUG(LX(TAG));
    return AlexaHybrid::SDK::Result<std::map<std::string, std::string>>::create(m_endpoints);
}

void EndpointImpl::addObserver(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.insert( sharedObs );
        AACE_INFO(LX(TAG, "LVC registered endpoint observer").d("observers size", m_observers.size()));
    }
}

void EndpointImpl::removeObserver(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.erase( sharedObs );
        AACE_INFO(LX(TAG, "LVC deregistered endpoint observer").d("observers size", m_observers.size()));
    }
}

void EndpointImpl::onGatewaySet(const std::string& avsGateway) {
    AACE_INFO(LX(TAG).d("current AVS gateway", m_avsGateway).d("new AVS gateway", avsGateway));
    m_avsGateway = avsGateway;
    updateEndpointsList(avsGateway);

    std::lock_guard<std::mutex> lock( m_mutex );
    for (auto observer : m_observers) {
        AACE_INFO(LX(TAG).m("notifying observer of endpoints"));
        observer->onEndpointChange(m_endpoints);
    }
}

bool EndpointImpl::updateEndpointsList(const std::string& avsEndpoint) {
    // Parse the gateway URL for the region in order to choose the correct MLIS and DAVS endpoints
    // example gateway URL: https://alexa.na.gateway.devices.a2z.com
    std::regex rgx("https?:\\/\\/.+-([A-Za-z]{2})");
    std::smatch matches;
    std::string region;
    m_endpoints.clear();
    if (std::regex_search(avsEndpoint, matches, rgx)) {
        for (size_t i = 0; i < matches.size(); ++i) {
            if (i == 1) {
                region = matches[i].str();
            }
        }
    }
    if (region.rfind("fe", 0) == 0) {
        AACE_INFO(LX(TAG).m("Gateway set to Asia region"));
        m_endpoints.emplace("MLIS", "https://mlis.amazon.co.jp");
        m_endpoints.emplace("DAVS", "https://api.fe.amazonalexa.com");
        return true;
    }
    else if (region.rfind("eu", 0) == 0) {
        AACE_INFO(LX(TAG).m("Gateway set to Europe region"));
        m_endpoints.emplace("MLIS", "https://mlis.amazon.eu");
        m_endpoints.emplace("DAVS", "https://api.eu.amazonalexa.com");
        return true;
    }
    // default to North America region
    AACE_INFO(LX(TAG).m("Gateway set to North America region"));
    m_endpoints.emplace("MLIS", "https://mlis.amazon.com");
    m_endpoints.emplace("DAVS", "https://api.amazonalexa.com");
    return true;
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace