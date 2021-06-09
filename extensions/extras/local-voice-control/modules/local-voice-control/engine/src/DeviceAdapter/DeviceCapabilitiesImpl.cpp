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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceCapabilitiesImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.DeviceCapabilitiesImpl");

using namespace AlexaHybrid::SDK;
using namespace AlexaHybrid::SDK::Router::DeviceAdapter;

DeviceCapabilitiesImpl::DeviceCapabilitiesImpl() :
        m_capabilities{
            "20160207",
            {{"Alerts", "1.5"},
             {"Alexa", "3"},
             {"Alexa.ApiGateway", "1.0"},
             {"Alexa.Comms.MessagingController", "1.0"},
             {"Alexa.Comms.PhoneCallController", "2.0"},
             {"Alexa.DoNotDisturb", "1.0"},
             {"Alexa.FavoritesController", "1.0"},
             {"Alexa.Networking.Connectivity", "1.0"},
             {"Alexa.Networking.InternetDataPlan", "1.0"},
             {"Alexa.PlaybackController", "1.0"},
             {"Alexa.PlaybackStateReporter", "1.0"},
             {"Alexa.PlaylistController", "1.0"},
             {"Alexa.SeekController", "1.0"},
             {"Alexa.AudioActivityTracker", "1.0"},
             {"AudioPlayer", "1.5"},
             {"EqualizerController", "1.0"},
             {"ExternalMediaPlayer", "1.1"},
             {"InteractionModel", "1.2"},
             {"Navigation", "2.0"},
             {"Navigation.Assistance", "1.0"},
             {"Navigation.DisplayManager", "1.0"},
             {"Notifications", "1.0"},
             {"PlaybackController", "1.1"},
             {"Speaker", "1.0"},
             {"SpeechRecognizer", "2.3"},
             {"SpeechSynthesizer", "1.3"},
             {"SpeechSynthesizer.Generator", "1.0"},
             {"System", "2.1"},
             {"TemplateRuntime", "1.2"},
             {"VisualActivityTracker", "1.0"}},
            {}} {
}

Result<IDeviceCapabilities::Capabilities> DeviceCapabilitiesImpl::getCapabilities() {
    AACE_INFO(LX(TAG));
    return Result<Capabilities>::create(m_capabilities);
}

void DeviceCapabilitiesImpl::addObserver(std::weak_ptr<IDeviceCapabilitiesObserver> observer) {
    AACE_INFO(LX(TAG));
}

void DeviceCapabilitiesImpl::removeObserver(std::weak_ptr<IDeviceCapabilitiesObserver> observer) {
    AACE_INFO(LX(TAG));
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
