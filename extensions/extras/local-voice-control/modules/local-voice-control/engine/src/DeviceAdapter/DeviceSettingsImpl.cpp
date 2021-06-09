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

#include <nlohmann/json.hpp>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceSettingsImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

using json = nlohmann::json;

#ifdef COASSISTANT
using namespace AlexaHybrid::SDK;

template <int N, typename... Ts>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;
#endif

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.DeviceSettingsImpl");

DeviceSettingsImpl::DeviceSettingsImpl(std::shared_ptr<aace::engine::core::EngineContext> engineContext) {
    AACE_INFO(LX(TAG));
    m_engineContext = engineContext;
}

std::string DeviceSettingsImpl::getDeviceSettings() {
    AACE_INFO(LX(TAG));

    // clang-format off
    json settings = {
        {"version", "1"},
        {"deviceSettings", {
            {"FollowUpMode", {
                {"available", false},
                {"isRetryable", false}
            }},
            {"AudioProcessingMode", {
                {"available", false},
                {"isRetryable", false}
            }}
        }
    }};
    // clang-format on

#ifdef COASSISTANT
    try {
        AACE_INFO(LX(TAG, "Sending MultiAssistant settings to AHE"));
        // get CoAssistant EngineService to have access to MultiAssistant Setting Manager
        if (m_multiAssistantSettingsManager == nullptr) {
            m_coAssistantComponents = m_engineContext->getServiceInterface<aace::engine::coassistant::CoAssistantComponentInterface>("aace.coassistant");
            ThrowIfNull(m_coAssistantComponents, "nullCoAssistantComponents");

            m_multiAssistantSettingsManager = m_coAssistantComponents->getMultiAssistantSettingManager();
            ThrowIfNull(m_multiAssistantSettingsManager, "nullMultiAssistantSettingsManager");

            using ThirdType = NthTypeOf<0, alexaClientSDK::multiassistant::AssistantsSetting>;
            auto observer = std::make_shared<alexaClientSDK::settings::SettingsAdapterObserver<ThirdType>>([&]() {
                AACE_INFO(LX(TAG, "Notifying AHE about MultiAssistant settings changed"));
                ObserverRegistry::notifyAll(
                    [](Router::DeviceAdapter::IDeviceSettingsObserver& observer) { observer.onDeviceSettingsChange(); });
                });
            m_multiAssistantSettingsManager->addObserver<0>(observer);
        }

        std::string settingJsonString = m_multiAssistantSettingsManager->getJsonValue<0>();

        settings["deviceSettings"]["MultiAssistant.assistants"]["available"] = true;
        settings["deviceSettings"]["MultiAssistant.assistants"]["isRetryable"] = true;
        settings["deviceSettings"]["MultiAssistant.assistants"]["setting"] = settingJsonString;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "generating multiassistant settings").d("reason", ex.what()));
    }
#endif

    AACE_INFO(LX(TAG).sensitive("settings", settings.dump()));
    return settings.dump();
}

void DeviceSettingsImpl::addObserver(DeviceSettingsImpl::IDeviceSettingsObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::addObserver(observer);
}

void DeviceSettingsImpl::removeObserver(DeviceSettingsImpl::IDeviceSettingsObserver observer) {
    AACE_INFO(LX(TAG));
    ObserverRegistry::removeObserver(observer);
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
