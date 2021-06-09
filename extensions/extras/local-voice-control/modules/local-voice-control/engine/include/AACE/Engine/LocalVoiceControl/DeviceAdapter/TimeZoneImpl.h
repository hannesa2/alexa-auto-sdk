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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_TIME_ZONE_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_TIME_ZONE_IMPL_H

#include <alexa_hybrid/router/device_adapter/ITimeZone.h>
#include <alexa_hybrid/sdk/Result.h>
#include <hr/commons/ObserverRegistry.h>

#include <Settings/DeviceSettingsManager.h>
#include <Settings/SettingCallbackAdapter.h>

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * ITimeZone implementation
 */
class TimeZoneImpl
        : public AlexaHybrid::SDK::Router::DeviceAdapter::ITimeZone
        , public HRExt::Common::ObserverRegistry<AlexaHybrid::SDK::Router::DeviceAdapter::ITimeZoneObserver> {
public:
    using SettingsSource = std::function<std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager>()>;

    using SettingsAdapter = alexaClientSDK::settings::
        SettingCallbackAdapter<alexaClientSDK::settings::DeviceSettingsManager, alexaClientSDK::settings::TIMEZONE>;

    using ITimeZoneObserver = std::weak_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::ITimeZoneObserver>;

    /**
     * Constructor
     * @param settings Device settings manager to read device settings
     */
    TimeZoneImpl(SettingsSource settings);

    /// @name ITimeZone functions.
    AlexaHybrid::SDK::Result<ITimeZone::TimeZone> getTimeZone() override;
    void addObserver(ITimeZoneObserver observer) override;
    void removeObserver(ITimeZoneObserver observer) override;
    /// @}

private:
    /// Lock to serialize access to m_timezone
    std::mutex m_lock_timezone;
    /// Lock to serialize access to init()
    std::mutex m_lock_init;
    /// Settings adapter
    std::shared_ptr<SettingsAdapter> m_adapter;
    /// Settings source
    SettingsSource m_settings;
    /// Device timezone
    ITimeZone::TimeZone m_timezone;

    /**
     * Callback called when the timezone changes
     * @param timezone The new timezone
     * @param notifications Type of timezone change notification
     */
    void onTimeZoneChanged(const std::string& timezone, alexaClientSDK::settings::SettingNotifications notifications);

    /**
     * Initialize with platform timezone setting provider
     */
    void init();

    /**
     * Set the current timezone.
     * @param timezone The currently timezone
     */
    void set(const std::string& timezone);
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_TIME_ZONE_IMPL_H
