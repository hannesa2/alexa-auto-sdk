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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_CONNECTIVITY_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_CONNECTIVITY_IMPL_H

#include <alexa_hybrid/router/device_adapter/IConnectivity.h>
#include <alexa_hybrid/sdk/Result.h>
#include <hr/commons/ObserverRegistry.h>

#include <AACE/Engine/Network/NetworkInfoObserver.h>
#include <AACE/Engine/Network/NetworkObservableInterface.h>

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * IConnectivity implementation
 */
class ConnectivityImpl
        : public AlexaHybrid::SDK::Router::DeviceAdapter::IConnectivity
        , public HRExt::Common::ObserverRegistry<AlexaHybrid::SDK::Router::DeviceAdapter::IConnectivityObserver>
        , public aace::engine::network::NetworkInfoObserver
        , public std::enable_shared_from_this<ConnectivityImpl> {
public:
    using IConnectivityObserver = AlexaHybrid::SDK::Router::DeviceAdapter::IConnectivityObserver;

    /**
     * Constructor
     */
    ConnectivityImpl();

    /// @name IConnectivity functions.
    /// @{
    AlexaHybrid::SDK::Result<ConnectivityStatus> getConnectivityStatus() override;
    void addObserver(std::weak_ptr<IConnectivityObserver> observer) override;
    void removeObserver(std::weak_ptr<IConnectivityObserver> observer) override;
    /// @}

    /// @name NetworkInfoObserver functions.
    /// @{
    void onNetworkInfoChanged(
        aace::network::NetworkInfoProviderEngineInterface::NetworkStatus status,
        int wifiSignalStrength) override;
    void onNetworkInterfaceChangeStatusChanged(const std::string& networkInterface, NetworkInterfaceChangeStatus status)
        override;
    /// @}

    /**
     * Set the network provider for observing network status changes.
     * @param networkProvider The network provider or @c nullptr if one is not available
     */
    void setNetworkProvider(std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkProvider);

private:
    /**
     * Set if device is connected to internet.
     * @param isConnected @c true if the device has internet connectivity.
     */
    void setConnected(bool isConnected);

    /**
     * Function to call when device connectivity changes.
     */
    void onConnectivityChange();

    /// Lock to serialize access to members.
    std::mutex m_lock;

    /// Current connectivity status.
    ConnectivityStatus m_connectivityStatus;

    /// Whether there is a network provider available to observe for connectivity status updates.
    bool m_isNetworkProviderAvailable;
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_CONNECTIVITY_IMPL_H
