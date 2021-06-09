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
#include "AACE/Engine/LocalVoiceControl/DeviceAdapter/DeviceInfoImpl.h"

namespace aace {
namespace engine {
namespace localVoiceControl {
namespace deviceAdapter {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.DeviceAdapter.DeviceInfoImpl");

using namespace AlexaHybrid::SDK;

AlexaHybrid::SDK::Result<std::string> DeviceInfoImpl::getDeviceId() {
    AACE_INFO(LX(TAG));
    return AlexaHybrid::SDK::Result<std::string>::create(m_dsn);
}

DeviceInfoImpl::DeviceInfoImpl(const std::string& deviceSerialNumber) : m_dsn(deviceSerialNumber) {
    AACE_INFO(LX(TAG).sensitive("m_dsn", m_dsn));
}

}  // namespace deviceAdapter
}  // namespace localVoiceControl
}  // namespace engine
}  // namespace aace
