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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_CAPABILITIES_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_CAPABILITIES_IMPL_H

#include <alexa_hybrid/router/device_adapter/IDeviceCapabilities.h>
#include <alexa_hybrid/sdk/Result.h>

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * IDeviceCapabilities implementation
 */
class DeviceCapabilitiesImpl : public AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilities {
private:
    AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilities::Capabilities m_capabilities;

public:
    /**
     * Constructor
     */
    DeviceCapabilitiesImpl();

    /// @name IDeviceCapabilities functions.
    /// @{
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilities::Capabilities>
    getCapabilities() override;
    /// @}

    void addObserver(
        std::weak_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilitiesObserver> observer) override;
    void removeObserver(
        std::weak_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilitiesObserver> observer) override;
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_CAPABILITIES_IMPL_H
