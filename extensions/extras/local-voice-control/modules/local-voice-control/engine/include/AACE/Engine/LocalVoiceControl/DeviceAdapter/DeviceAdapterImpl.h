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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_ADAPTER_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_ADAPTER_IMPL_H

#include <alexa_hybrid/router/device_adapter/IDeviceAdapter.h>
#include <alexa_hybrid/sdk/IMetricsEmitter.h>
#include <alexa_hybrid/sdk/Result.h>

#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/ConnectivityImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceCapabilitiesImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceInfoImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceSettingsImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/LocaleImpl.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/TimeZoneImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * IDeviceAdapter implementation
 */
class DeviceAdapterImpl : public AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceAdapter {
private:
    using SettingsSource = std::function<std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager>()>;

    /// Locale adapter
    LocaleImpl m_localeImpl;
    /// TimeZone adapter
    TimeZoneImpl m_timeZoneImpl;
    /// Device Info adapter
    DeviceInfoImpl m_deviceInfoImpl;
    /// Device Settings adapter
    DeviceSettingsImpl m_deviceSettingsImpl;
    /// Device Capabilities adapter
    std::shared_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilities> m_deviceCapabilitiesImpl;
    /// Connectivity adapter
    std::shared_ptr<ConnectivityImpl> m_connectivityImpl;
    /// Metrics Emitter adapter
    std::shared_ptr<AlexaHybrid::SDK::IMetricsEmitter> m_metricsEmitterImpl;

public:
    /**
     * Constructor
     * @param dsn Device Serial Number
     * @param connectivity Connectivity adapter implementation
     * @param metricsEmitter Metrics adapter implementation
     * @param settings Settings source
     * @param engineContext Engine context
     */
    DeviceAdapterImpl(
        const std::string& dsn,
        std::shared_ptr<ConnectivityImpl> connectivity,
        std::shared_ptr<AlexaHybrid::SDK::IMetricsEmitter> metricsEmitter,
        const SettingsSource& settings,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext);

    /// @name IDeviceAdapter functions.
    /// @{
    AlexaHybrid::SDK::Router::DeviceAdapter::ILocale& getLocale() override;
    AlexaHybrid::SDK::Router::DeviceAdapter::ITimeZone& getTimeZone() override;
    AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceInfo& getDeviceInfo() override;
    AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceSettings& getDeviceSettings() override;
    std::shared_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceCapabilities> getDeviceCapabilities() override;
    AlexaHybrid::SDK::Router::DeviceAdapter::IConnectivity& getConnectivity() override;
    std::shared_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IAlexaConnectivity> getAlexaConnectivity() override;
    std::shared_ptr<AlexaHybrid::SDK::IMetricsEmitter> getMetricsEmitter() override;
    /// @}
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_IMPL_H
