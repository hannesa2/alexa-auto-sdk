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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/TimeZoneImpl.h"
#include "Settings/SettingCallbackAdapter.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.TimeZoneImpl");

using namespace AlexaHybrid::SDK;

TimeZoneImpl::TimeZoneImpl(SettingsSource settings) : m_settings(std::move(settings)) {
    AACE_INFO(LX(TAG));
}

void TimeZoneImpl::onTimeZoneChanged(
    const std::string& timezone,
    alexaClientSDK::settings::SettingNotifications notifications) {
    AACE_INFO(LX(TAG).d("notifications", notifications));
    if (notifications == alexaClientSDK::settings::SettingNotifications::AVS_CHANGE ||
        notifications == alexaClientSDK::settings::SettingNotifications::LOCAL_CHANGE) {
        set(timezone);
    }
}

void TimeZoneImpl::set(const std::string& timezone) {
    AACE_INFO(LX(TAG).sensitive("timezone", timezone));
    std::unique_lock<std::mutex> lock(m_lock_timezone);
    m_timezone.format = AlexaHybrid::SDK::Router::DeviceAdapter::TimeZoneFormat::OLSON;
    m_timezone.value = timezone;
    lock.unlock();
    ObserverRegistry::notifyAll(
        [](Router::DeviceAdapter::ITimeZoneObserver& observer) { observer.onTimeZoneChange(); });
}

Result<TimeZoneImpl::ITimeZone::TimeZone> TimeZoneImpl::getTimeZone() {
    AACE_DEBUG(LX(TAG));
    if (!m_adapter) {
        init();
    }
    if (!m_adapter) {
        AACE_ERROR(LX(TAG).m("Unable to initialize"));
        return Result<TimeZone>::createErrorResponse("InitializationFailed", true);
    }

    std::unique_lock<std::mutex> lock(m_lock_timezone);
    return Result<TimeZone>::create(m_timezone);
}

void TimeZoneImpl::addObserver(TimeZoneImpl::ITimeZoneObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::addObserver(observer);
    if (!m_adapter) {
        init();
    }
}

void TimeZoneImpl::removeObserver(TimeZoneImpl::ITimeZoneObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::removeObserver(observer);
    if (!m_adapter) {
        init();
    }
}

void TimeZoneImpl::init() {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock_init);
    if (m_adapter) {
        AACE_DEBUG(LX(TAG).m("Already initialized"));
        return;
    }
    auto settingsManager = m_settings();
    if (settingsManager) {
        // Register for callbacks
        auto adapter = SettingsAdapter::create(
            [this](const std::string& timezone, alexaClientSDK::settings::SettingNotifications notifications) {
                onTimeZoneChanged(timezone, notifications);
            });
        if (adapter->addToManager(*settingsManager)) {
            m_adapter.swap(adapter);
            AACE_DEBUG(LX(TAG).m("Registered timezone settings observer"));
        } else {
            AACE_DEBUG(LX(TAG).m("Failed to register timezone settings observer"));
        }

        // Read the current timezone value
        std::string defaultValue;
        const std::pair<bool, std::string> result =
            settingsManager->getValue<alexaClientSDK::settings::TIMEZONE>(defaultValue);
        if (result.first) {
            set(result.second);
        } else {
            AACE_INFO(LX(TAG).m("Unable to obtain current timezone"));
        }
    } else {
        AACE_WARN(LX(TAG).m("Unable to initialize with null device settings manager"));
    }
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
