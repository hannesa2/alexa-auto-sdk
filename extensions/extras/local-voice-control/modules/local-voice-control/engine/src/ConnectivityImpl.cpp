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

#include "AACE/Engine/LocalVoiceControl/ConnectivityImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.ConnectivityImpl");

ConnectivityImpl::ConnectivityImpl() {
    m_networkStatus = aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::UNKNOWN;
    m_connected = false;
}

AlexaHybrid::SDK::Result<bool> ConnectivityImpl::isConnected() const {
    AACE_INFO(LX(TAG, "isConnected").d("status", m_connected));
    return AlexaHybrid::SDK::Result<bool>::create( m_connected );
}

AlexaHybrid::SDK::Result<bool> ConnectivityImpl::isConnectionMetered() const {
    // TODO: default to false for now, but we might want to expose this to platform so OEMs can provide whether connection is metered.
    return AlexaHybrid::SDK::Result<bool>::create( false );
}

AlexaHybrid::SDK::Result<int32_t> ConnectivityImpl::maxDownloadSpeed() const {
    // TODO: default to -1 which means no speed limitation, but we might want to expose this to platform so OEMs can provide the max download speed as they needed.
    return AlexaHybrid::SDK::Result<int32_t>::create( -1 );
}

void ConnectivityImpl::setConnectivityChangedCallback(std::function<void(bool)> callbackFn) {
    AACE_INFO(LX(TAG, "ConnectivityChangedCallback").d("isNull", callbackFn == nullptr));
    m_connectivityChangedFn = callbackFn;
}

void ConnectivityImpl::onNetworkInfoChanged( aace::network::NetworkInfoProviderEngineInterface::NetworkStatus newStatus, int newWifiSignalStrength ) {
    AACE_INFO(LX(TAG, "onNetworkInfoChanged").d("status",newStatus).d("wifiSignalStrength",newWifiSignalStrength));
    if ( m_networkStatus != newStatus ) {
        m_networkStatus = newStatus;

        if ( m_networkStatus == aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTED ) {
            m_connected = true;
        } else {
            m_connected = false;
        }

        // Notify callback
        if ( m_connectivityChangedFn != nullptr ) {
            m_connectivityChangedFn(m_connected);
        }

        // Make a copy of observers to notify
        std::unique_lock<std::mutex> lock( m_mutex );
        auto observers = m_observers;
        lock.unlock();

        // Notify observers
        for (auto observer : observers) {
            AACE_INFO(LX(TAG, "onNetworkInfoChanged notifying observer."));
            observer->onConnectivityChange(m_connected);
        }
    }
}

void ConnectivityImpl::onNetworkInterfaceChangeStatusChanged( const std::string& networkInterface, NetworkInterfaceChangeStatus status ) {
    AACE_WARN(LX(TAG, "onNetworkInterfaceChangeStatusChanged").d("networkInterface",networkInterface).d("status",status));
    // TODO: LVC yet to provide support to switching the interface.
}

void ConnectivityImpl::addObserver( std::weak_ptr<IObserver> observer ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.insert( sharedObs );
        AACE_INFO(LX(TAG, "AHE registered observer").d("observers size", m_observers.size()));
    }
}

void ConnectivityImpl::removeObserver( std::weak_ptr<IObserver> observer ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.erase( sharedObs );
        AACE_INFO(LX(TAG, "AHE deregistered observer").d("observers size", m_observers.size()));
    }
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace