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

#ifndef AACE_LOCAL_VOICE_CONTROL_DUMMY_AUTH_DELEGATE_H
#define AACE_LOCAL_VOICE_CONTROL_DUMMY_AUTH_DELEGATE_H

#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>

namespace aace {
namespace engine {
namespace localVoiceControl {

class DummyAuthDelegate : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {

    void addAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    void removeAuthObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) override;
    std::string getAuthToken() override;
    void onAuthFailure(const std::string& token) override;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_DUMMY_AUTH_DELEGATE_H