/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/LocalVoiceControl/DummyAuthDelegate.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

void DummyAuthDelegate::addAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer)
{
    observer->onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS);
}

void DummyAuthDelegate::removeAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) {}

std::string DummyAuthDelegate::getAuthToken()
{
    return "DummyAuthToken";
}

void DummyAuthDelegate::onAuthFailure(const std::string& token) {}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace