/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/ConnectivityImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.ConnectivityImpl");

using namespace AlexaHybrid::SDK;

ConnectivityImpl::ConnectivityImpl() : m_connectivityStatus{false}, m_isNetworkProviderAvailable{true} {
}

void ConnectivityImpl::onNetworkInfoChanged(
    aace::network::NetworkInfoProviderEngineInterface::NetworkStatus newStatus,
    int newWifiSignalStrength) {
    AACE_INFO(LX(TAG, "onNetworkInfoChanged").d("status", newStatus));
    setConnected(newStatus == aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTED);
    onConnectivityChange();
}

void ConnectivityImpl::onNetworkInterfaceChangeStatusChanged(
    const std::string& networkInterface,
    NetworkInterfaceChangeStatus status) {
    // no-op
}

AlexaHybrid::SDK::Result<ConnectivityImpl::ConnectivityStatus> ConnectivityImpl::getConnectivityStatus() {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock);
    if (!m_isNetworkProviderAvailable) {
        return AlexaHybrid::SDK::Result<ConnectivityStatus>::createErrorResponse(
            "Connectivity status not available", false);
    } else {
        return AlexaHybrid::SDK::Result<ConnectivityStatus>::create(m_connectivityStatus);
    }
}

void ConnectivityImpl::addObserver(std::weak_ptr<IConnectivityObserver> observer) {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock);
    ObserverRegistry::addObserver(observer);
}

void ConnectivityImpl::removeObserver(std::weak_ptr<IConnectivityObserver> observer) {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock);
    ObserverRegistry::removeObserver(observer);
}

void ConnectivityImpl::setNetworkProvider(
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkProvider) {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock);
    if (networkProvider != nullptr) {
        networkProvider->addObserver(shared_from_this());
        m_isNetworkProviderAvailable = true;
    } else {
        AACE_INFO(LX(TAG).m("Network info provider is not available"));
        m_isNetworkProviderAvailable = false;
    }
}

void ConnectivityImpl::setConnected(bool isConnected) {
    AACE_INFO(LX(TAG).d("isConnected", isConnected));
    std::unique_lock<std::mutex> lock(m_lock);
    m_connectivityStatus.isConnected = isConnected;
}

void ConnectivityImpl::onConnectivityChange() {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock);
    ObserverRegistry::notifyAll([](IConnectivityObserver& observer) { observer.onConnectivityChange(); });
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
