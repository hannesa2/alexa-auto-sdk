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

#ifndef AACE_LOCAL_VOICE_CONTROL_CONNECTIVITYIMPL_H
#define AACE_LOCAL_VOICE_CONTROL_CONNECTIVITYIMPL_H

#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Network/NetworkEngineInterfaces.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"
#include "alexa_hybrid/sdk/IConnectivity.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class ConnectivityImpl :
    public AlexaHybrid::SDK::IConnectivity,
    public aace::engine::network::NetworkInfoObserver,
    public std::enable_shared_from_this<ConnectivityImpl> {

public:
    /**
     * Default constructor.
     */
    ConnectivityImpl();

    /**
     * Set the network observable interface and register this object as an observer.
     */
    void setNetworkObservableInterface( std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObservableInterface );

public:
    /**
     * Indicates whether internet connectivity exists.
     *
     * @return @c true if device is connected to the internet, @c false otherwise.
     */
    AlexaHybrid::SDK::Result<bool> isConnected() const override;

    /**
     * Indicates whether the internet connection is metered.
     *
     * Returns if the connection is metered. A network is classified as metered when the user is sensitive to heavy data usage
     * on that connection due to monetary costs, data limitations or battery/performance issues.
     * Large data transfers should be avoided
     *
     * @return @c true if connection is metered, @c false otherwise.
     */
    AlexaHybrid::SDK::Result<bool> isConnectionMetered() const override;

    /**
     * Indicates maximum allowed download spped
     *
     * Returns the maximum download bandwith that can be used.
     *
     * @return max download speed in Kb/s. A value of @c -1 means that there is no speed limitation,  @c 0 means that downloads are not allowed.
     */
    AlexaHybrid::SDK::Result<int32_t> maxDownloadSpeed() const override;

    /**
     * DEPRECATED: use registerObserver() and deregisterObserver() instead
     *
     * Register a callback to observe internet connectivity changes.
     *
     * Only one callback may be registered at a time. Subsequent calls to this
     * function will overwrite the previously set callback. To remove a callback
     * @c nullptr may be passed.
     *
     * @param callback        Callback function or @c nullptr. The current connectivity
     *                        status will be passed to the callback function. A value of @c true
     *                        indicates that the device has internet connectivity, @c false indicates
     *                        that it does not have internet connectivity.
     */
    void setConnectivityChangedCallback( std::function<void(bool)> callback ) override;

    /**
     * Adds an observer to observe value changes for any of the properties queryable on this object (IConnectivity).
     * Upon receiving a callback, observers are expected to read the properties they are interested in.
     *
     * Notes:
     * 1. Adding the same observer multiple times will have the same effect as adding it once, i.e.,
     * subsequent calls to this function with an observer that is already added will be ignored.
     * 2. If a weak_ptr expires the observer will be safely removed when the next callback is to be fired.
     * 3. Passing a weak_ptr to a nullptr has no effect.
     *
     * @param observer A weak pointer to an observer instance. If the observer is already added, this method has
     * no effect.
     */
    void addObserver( std::weak_ptr<IObserver> observer ) override;

    /**
     * Removes a previously added observer.
     *
     * Passing a weak_ptr to a nullptr has no effect.
     *
     * @param observer A weak pointer to a previously added observer. If the observer is not added, this
     * method has no effect.
     */
    void removeObserver( std::weak_ptr<IObserver> observer ) override;

public:
    void onNetworkInfoChanged( aace::network::NetworkInfoProviderEngineInterface::NetworkStatus status, int wifiSignalStrength ) override;
    void onNetworkInterfaceChangeStatusChanged( const std::string& networkInterface, NetworkInterfaceChangeStatus status ) override;

    /**
     * Notify observers of network change.
     */
    void notifyObservers();

private:
    /// Is device connected to the internet
    bool m_connected;

    /// Current network status
    aace::network::NetworkInfoProviderEngineInterface::NetworkStatus m_networkStatus;

    // Connectivity changed callback function
    std::function<void(bool)> m_connectivityChangedFn;

    /// Observers set 
    std::unordered_set<std::shared_ptr<IObserver>> m_observers;

    /// Mutex
    std::mutex m_mutex;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_CONNECTIVITYIMPL_H