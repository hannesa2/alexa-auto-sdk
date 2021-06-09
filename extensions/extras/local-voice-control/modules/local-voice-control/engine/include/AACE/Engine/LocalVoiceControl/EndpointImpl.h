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

#ifndef AACE_LOCAL_VOICE_CONTROL_ENDPOINT_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_ENDPOINT_IMPL_H

#include <vector>
#include <regex> 

#include "alexa_hybrid/sdk/Result.h"
#include "alexa_hybrid/sdk/IEndpoint.h"

#include <AVSCommon/SDKInterfaces/AVSGatewayObserverInterface.h>

#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Network/NetworkEngineInterfaces.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"

namespace aace {
namespace engine {
namespace localVoiceControl {


class EndpointImpl :
    public AlexaHybrid::SDK::IEndpoint,
    public alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayObserverInterface,
    public std::enable_shared_from_this<EndpointImpl> {

public:
    /**
     * Default constructor.
     */
    EndpointImpl();

    EndpointImpl(std::string avsGateway);

    /**
     * Indicates the map of endpoints currently recommended for AHE (based on device location and user account
     * settings).
     *
     * @return @c map of endpoints (endpoint -> URI) for AHE.
     */
    AlexaHybrid::SDK::Result<std::map<std::string, std::string>> getEndpoints() const override;

    /**
     * Adds an observer to observe value changes for any of the properties queryable on this object (IEndpoint).
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
    void addObserver(std::weak_ptr<IObserver> observer) override;

    /**
     * Removes a previously added observer.
     *
     * Passing a weak_ptr to a nullptr has no effect.
     *
     * @param observer A weak pointer to a previously added observer. If the observer is not added, this
     * method has no effect.
     */
    void removeObserver(std::weak_ptr<IObserver> observer) override;

    /// @name AVSGatewayObserver method overrides.
    /// @{
    void onGatewaySet(const std::string& avsGateway) override;
    /// @}

private:

    /// Observers 
    std::unordered_set<std::shared_ptr<IObserver>> m_observers;
    /// Mutex protecting @c m_observers
    std::mutex m_mutex;
    /// The current AVS gateway
    mutable std::string m_avsGateway;
    /// The list of current endpoints (e.g. AVS gateway, DAVS, MLIS)
    std::map<std::string, std::string> m_endpoints;

    /// Refresh the list of endpoints based on the provided @c avsGateway
    bool updateEndpointsList(const std::string& avsGateway);
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_ENDPOINT_IMPL_H