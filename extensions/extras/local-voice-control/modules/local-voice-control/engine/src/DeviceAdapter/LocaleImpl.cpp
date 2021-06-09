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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/LocaleImpl.h"
#include "Settings/SettingCallbackAdapter.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.LocaleImpl");

using namespace alexaClientSDK;
using namespace AlexaHybrid::SDK;

LocaleImpl::LocaleImpl(SettingsSource settings) : m_settings(std::move(settings)) {
    AACE_INFO(LX(TAG));
}

void LocaleImpl::onLocaleChanged(const std::vector<std::string>& locale, settings::SettingNotifications notifications) {
    AACE_INFO(LX(TAG).d("notifications", notifications).d("localeCount", locale.size()));
    if (notifications == settings::SettingNotifications::AVS_CHANGE ||
        notifications == settings::SettingNotifications::LOCAL_CHANGE) {
        set(locale);
    }
}
void LocaleImpl::set(const std::vector<std::string>& locale) {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock_locale);
    m_locale = locale;
    lock.unlock();

    ObserverRegistry::notifyAll([](Router::DeviceAdapter::ILocaleObserver& observer) { observer.onLocaleChange(); });
}

Result<std::vector<std::string>> LocaleImpl::getLocale() {
    AACE_DEBUG(LX(TAG));
    if (!m_adapter) {
        init();
    }
    if (!m_adapter) {
        AACE_ERROR(LX(TAG).m("Unable to initialize"));
        return Result<std::vector<std::string>>::createErrorResponse("InitializationFailed", true);
    }
    std::unique_lock<std::mutex> lock(m_lock_locale);
    return Result<std::vector<std::string>>::create(m_locale);
}

void LocaleImpl::addObserver(LocaleImpl::ILocaleObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::addObserver(observer);
    if (!m_adapter) {
        init();
    }
}

void LocaleImpl::removeObserver(LocaleImpl::ILocaleObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::removeObserver(observer);
    if (!m_adapter) {
        init();
    }
}

void LocaleImpl::init() {
    AACE_INFO(LX(TAG));
    std::unique_lock<std::mutex> lock(m_lock_init);
    if (m_adapter) {
        AACE_DEBUG(LX(TAG).m("Already initialized"));
        return;
    }
    auto settingsManager = m_settings();
    if (settingsManager != nullptr) {
        // Register for callbacks
        auto adapter = SettingsAdapter::create(
            [this](const std::vector<std::string>& locale, settings::SettingNotifications notifications) {
                onLocaleChanged(locale, notifications);
            });

        if (adapter->addToManager(*settingsManager)) {
            m_adapter.swap(adapter);
            AACE_DEBUG(LX(TAG).m("Registered locale settings observer"));
        } else {
            AACE_DEBUG(LX(TAG).m("Failed to register locale settings observer"));
        }

        // Read the current locale value
        std::vector<settings::Locale> defaultValue;
        const std::pair<bool, std::vector<settings::Locale>> result =
            settingsManager->getValue<alexaClientSDK::settings::LOCALE>(defaultValue);
        if (result.first) {
            set(result.second);
        } else {
            AACE_INFO(LX(TAG).m("Unable to obtain current locale"));
        }
    } else {
        AACE_WARN(LX(TAG).m("Unable to initialize with null device settings manager"));
    }
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
