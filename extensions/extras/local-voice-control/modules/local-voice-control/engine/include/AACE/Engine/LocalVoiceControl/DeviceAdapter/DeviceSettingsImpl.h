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

#ifndef AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_SETTINGS_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_SETTINGS_IMPL_H

#include <alexa_hybrid/router/device_adapter/IDeviceSettings.h>
#include <alexa_hybrid/sdk/Result.h>
#include <hr/commons/ObserverRegistry.h>

#ifdef COASSISTANT
#include "AACE/Engine/CoAssistant/CoAssistantComponentInterface.h"
#include "MultiAssistant/AssistantInfo.h"
#include "MultiAssistant/MultiAssistantSettingsManager.h"
#include "Settings/SettingsAdapterObserver.h"
#endif

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

/**
 * IDeviceSettings implementation
 *
 */
class DeviceSettingsImpl
        : public AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceSettings
        , public HRExt::Common::ObserverRegistry<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceSettingsObserver> {
public:
    using IDeviceSettingsObserver = std::weak_ptr<AlexaHybrid::SDK::Router::DeviceAdapter::IDeviceSettingsObserver>;

    /**
     * Constructor
     */
    DeviceSettingsImpl(std::shared_ptr<aace::engine::core::EngineContext> engineContext);

    /// @name IDeviceSettings functions.
    std::string getDeviceSettings() override;
    void addObserver(IDeviceSettingsObserver observer) override;
    void removeObserver(IDeviceSettingsObserver observer) override;
    /// @}

private:
    #ifdef COASSISTANT
    std::shared_ptr<aace::engine::coassistant::CoAssistantComponentInterface> m_coAssistantComponents;
    std::shared_ptr<alexaClientSDK::multiassistant::MultiAssistantSettingsManager> m_multiAssistantSettingsManager;
    #endif

    /// Store engine context
    std::shared_ptr<aace::engine::core::EngineContext> m_engineContext;
};

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_LOCAL_VOICE_CONTROL_DEVICE_ADAPTER_DEVICE_SETTINGS_IMPL_H
