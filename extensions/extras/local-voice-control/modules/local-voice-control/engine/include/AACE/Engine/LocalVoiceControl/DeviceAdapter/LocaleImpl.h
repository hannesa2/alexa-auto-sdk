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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_LOCALE_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_LOCALE_IMPL_H

#include <alexa_hybrid/router/device_adapter/ILocale.h>
#include <alexa_hybrid/sdk/Result.h>
#include <hr/commons/ObserverRegistry.h>

#include <Settings/DeviceSettingsManager.h>
#include <Settings/SettingCallbackAdapter.h>

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * ILocale implementation
 */
class LocaleImpl
        : public AlexaHybrid::SDK::Router::DeviceAdapter::ILocale
        , public HRExt::Common::ObserverRegistry<AlexaHybrid::SDK::Router::DeviceAdapter::ILocaleObserver> {
public:
    using SettingsSource = std::function<std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager>()>;

    using SettingsAdapter = alexaClientSDK::settings::
        SettingCallbackAdapter<alexaClientSDK::settings::DeviceSettingsManager, alexaClientSDK::settings::LOCALE>;

    using ILocaleObserver = std::weak_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::ILocaleObserver>;

    /**
     * Constructor
     * @param settings Device settings manager to read device settings
     */
    LocaleImpl(SettingsSource settings);

    /// @name ILocale functions.
    AlexaHybrid::SDK::Result<std::vector<std::string>> getLocale() override;
    void addObserver(ILocaleObserver observer) override;
    void removeObserver(ILocaleObserver observer) override;
    /// @}

private:
    /// Lock to serialize access to init()
    std::mutex m_lock_init;
    /// Lock to serialize access to m_locale
    std::mutex m_lock_locale;
    /// Settings adapter
    std::shared_ptr<SettingsAdapter> m_adapter;
    /// Settings source
    SettingsSource m_settings;
    /// List of currently enabled locales
    std::vector<alexaClientSDK::settings::Locale> m_locale;

    /**
     * Callback called when the locale changes
     * @param locale The new locale
     * @param notifications Type of locale change notification
     */
    void onLocaleChanged(
        const std::vector<std::string>& locale,
        alexaClientSDK::settings::SettingNotifications notifications);

    /**
     * Initialize with the locale setting provider.
     */
    void init();

    /**
     * Set the current locale.
     * @param locale The current locale.
     */
    void set(const std::vector<std::string>& locale);
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_LOCALE_IMPL_H
