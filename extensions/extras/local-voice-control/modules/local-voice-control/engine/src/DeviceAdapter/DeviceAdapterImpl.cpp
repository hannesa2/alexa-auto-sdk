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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceAdapterImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.DeviceAdapterImpl");

using namespace AlexaHybrid::SDK;
using namespace AlexaHybrid::SDK::Router::DeviceAdapter;

DeviceAdapterImpl::DeviceAdapterImpl(
    const std::string& deviceSerialNumber,
    std::shared_ptr<ConnectivityImpl> connectivity,
    std::shared_ptr<AlexaHybrid::SDK::IMetricsEmitter> metricsEmitter,
    const SettingsSource& settings,
    std::shared_ptr<aace::engine::core::EngineContext> engineContext) :
        m_localeImpl(settings),
        m_timeZoneImpl(settings),
        m_deviceInfoImpl(deviceSerialNumber),
        m_deviceSettingsImpl(engineContext),
        m_deviceCapabilitiesImpl(std::make_shared<DeviceCapabilitiesImpl>()),
        m_connectivityImpl(connectivity),
        m_metricsEmitterImpl(metricsEmitter) {
    AACE_INFO(LX(TAG));
}

ILocale& DeviceAdapterImpl::getLocale() {
    AACE_INFO(LX(TAG));
    return m_localeImpl;
}

ITimeZone& DeviceAdapterImpl::getTimeZone() {
    AACE_INFO(LX(TAG));
    return m_timeZoneImpl;
}

IDeviceSettings& DeviceAdapterImpl::getDeviceSettings() {
    AACE_INFO(LX(TAG));
    return m_deviceSettingsImpl;
}

IDeviceInfo& DeviceAdapterImpl::getDeviceInfo() {
    AACE_INFO(LX(TAG));
    return m_deviceInfoImpl;
}

std::shared_ptr<IDeviceCapabilities> DeviceAdapterImpl::getDeviceCapabilities() {
    AACE_INFO(LX(TAG));
    return m_deviceCapabilitiesImpl;
}

IConnectivity& DeviceAdapterImpl::getConnectivity() {
    AACE_INFO(LX(TAG));
    return *m_connectivityImpl;
}

std::shared_ptr<IAlexaConnectivity> DeviceAdapterImpl::getAlexaConnectivity() {
    AACE_INFO(LX(TAG));
    return nullptr;
}

std::shared_ptr<IMetricsEmitter> DeviceAdapterImpl::getMetricsEmitter() {
    AACE_INFO(LX(TAG));
    return m_metricsEmitterImpl;
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
