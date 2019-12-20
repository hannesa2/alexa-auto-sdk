/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_BUILDER_H
#define AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_BUILDER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <tuple>

#include <ACL/AVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/LocaleAssetsManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/SystemTimeZoneInterface.h>
#include <DoNotDisturbCA/DoNotDisturbCapabilityAgent.h>
#include <RegistrationManager/CustomerDataManager.h>
#include <Settings/CloudControlledSettingProtocol.h>
#include <Settings/DeviceControlledSettingProtocol.h>
#include <Settings/DeviceSettingsManager.h>
#include <Settings/Setting.h>
#include <Settings/SettingEventMetadata.h>
#include <Settings/SettingEventSender.h>
#include <Settings/SettingsManagerBuilderBase.h>
#include <Settings/SharedAVSSettingProtocol.h>
#include <Settings/Storage/DeviceSettingStorageInterface.h>

namespace aace {
namespace engine {
namespace alexa {

/**
 * The builder for @c DeviceSettingsManager.
 */
class DeviceSettingsManagerBuilder : public alexaClientSDK::settings::SettingsManagerBuilderBase<alexaClientSDK::settings::DeviceSettingsManager> {
public:
    /**
     * Constructor.
     *
     * @param settingStorage The storage used for settings.
     * @param messageSender Sender used to send events related to this setting changes.
     * @param connectionManager The ACL connection manager.
     * @param dataManager A dataManager object that will track the CustomerDataHandler.
     */
    DeviceSettingsManagerBuilder(
        std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> settingStorage,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager);

    /**
     * Configures do not disturb setting.
     *
     * @param dndCA The do not disturb capability agent which is actually responsible for building the setting.
     *
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withDoNotDisturbSetting(
        const std::shared_ptr<alexaClientSDK::capabilityAgents::doNotDisturb::DoNotDisturbCapabilityAgent>& dndCA);

    /**
     * Configures wake word confirmation setting.
     *
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withWakeWordConfirmationSetting();

    /**
     * Configures speech confirmation setting.
     *
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withSpeechConfirmationSetting();

    /**
     * Configures time zone setting.
     *
     * @param systemTimeZone The system timezone is an optional parameter responsible for validating / applying
     * timezone changes system wide.
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withTimeZoneSetting(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SystemTimeZoneInterface> systemTimeZone = nullptr);

    /**
     * Configures locale setting.
     *
     * @param localeAssetsManager The locale assets manager is responsible for validating / applying locale changes.
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withLocaleSetting(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface> localeAssetsManager);

    /**
     * Configures locale and wake words setting.
     *
     * @param localeAssetsManager The locale assets manager is responsible for validating / applying locale
     * related changes.
     * @return This builder to allow nested calls.
     */
    DeviceSettingsManagerBuilder& withLocaleAndWakeWordsSettings(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface> localeAssetsManager);

    /**
     * Gets the setting for the given @c index.
     *
     * @tparam index The setting index.
     * @return A pointer for the setting kept in @c index if the setting has been built; @c nullptr otherwise.
     * @note This function should be used after @c build() has been called.
     */
    template <size_t index>
    std::shared_ptr<SettingType<index>> getSetting() const;

   /**
    * Gets the setting configuration for the given @c index.
    *
    * @tparam index The setting index.
    * @return The setting configuration. An empty setting will be returned if the setting wasn't configured.
    * @note This function should be used after @c build() has been called.
    */
   template <size_t index>
   alexaClientSDK::settings::SettingConfiguration<SettingType<index>> getConfiguration() const;

    /**
     * Builds a @c DeviceSettingsManager with the settings previously configured.
     *
     * @return A pointer to a new DeviceSettingsManager if all settings were successfully built; @c nullptr otherwise.
     */
    std::unique_ptr<alexaClientSDK::settings::DeviceSettingsManager> build() override;

private:
    /**
     * Builds a setting that follows the given synchronization protocol.
     *
     * @tparam index The setting index.
     * @tparam ProtocolT The type of the setting protocol.
     * @param metadata The setting event metadata.
     * @param defaultValue The setting default value.
     * @param applyFn Function responsible for validating and applying a new setting value.
     * @return This builder to allow nested calls.
     */
    template <size_t index, class ProtocolT>
    DeviceSettingsManagerBuilder& withSynchronizedSetting(
        const alexaClientSDK::settings::SettingEventMetadata& metadata,
        const ValueType<index>& defaultValue,
        std::function<bool(const ValueType<index>&)> applyFn = std::function<bool(const ValueType<index>&)>());

    /// The setting storage used to build persistent settings.
    std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface> m_settingStorage;

    /// The message sender used to build settings that are synchronized with AVS.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// The connection manager that manages the connection with AVS.
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_connectionManager;

    /// The dataManager object that will track the CustomerDataHandler.
    std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> m_dataManager;

    /// Flag that indicates if there was any configuration error.
    bool m_foundError;
};

template <size_t index>
alexaClientSDK::settings::SettingConfiguration<DeviceSettingsManagerBuilder::SettingType<index>> DeviceSettingsManagerBuilder::getConfiguration() const {
   return std::get<index>(m_settingConfigs);
}

template <size_t index>
std::shared_ptr<DeviceSettingsManagerBuilder::SettingType<index>> DeviceSettingsManagerBuilder::getSetting() const {
    return std::get<index>(m_settingConfigs).setting;
}

//}  // namespace defaultClient
//}  // namespace alexaClientSDK

}  // aace::engine:alexa
}  // aace::engine
}  // aace

#endif  // AACE_ENGINE_ALEXA_DEVICE_SETTINGS_MANAGER_BUILDER_H
