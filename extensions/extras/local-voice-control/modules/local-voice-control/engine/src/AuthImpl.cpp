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

#include "AACE/Engine/LocalVoiceControl/AuthImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.AuthImpl");

AuthImpl::AuthImpl() {
        m_authState = alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED;
}

void AuthImpl::setAuthDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate ) {
    AACE_INFO(LX(TAG, "setAuthDelegate"));
    m_authDelegate = authDelegate;
    m_authDelegate->addAuthObserver(shared_from_this());
 }

AlexaHybrid::SDK::Result<bool> AuthImpl::isRegistered() const {
    AACE_INFO(LX(TAG, "isRegistered").d("state", !m_accessToken.empty()));

    if ( m_accessToken.empty() ) {
        return AlexaHybrid::SDK::Result<bool>::createErrorResponse("Failed to fetch registered status", false);
    }

    return AlexaHybrid::SDK::Result<bool>::create(true);
} 

AlexaHybrid::SDK::Result<bool> AuthImpl::getOAuthToken(std::string &token, bool forceRefresh) const {

    if (forceRefresh) {
        fetchAuthToken();
    }

    AACE_DEBUG(LX(TAG, "AccessToken").d("available", !m_accessToken.empty()).d("refresh", forceRefresh));

    if ( m_accessToken.empty() ) {
        return AlexaHybrid::SDK::Result<bool>::createErrorResponse("Failed to fetch OAuth token", false);
    }

    // Update the reference parameter
    token = m_accessToken;

    return AlexaHybrid::SDK::Result<bool>::create(true);
} 

void AuthImpl::setRegistrationChangedCallback(std::function<void(bool)> callbackFn) {
    AACE_DEBUG(LX(TAG, "RegistrationChangedCallback").d("isNull", callbackFn == nullptr));
    m_registrationChangedFn = callbackFn;
}

void AuthImpl::onAuthStateChange(alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState, 
                                 alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) {
    AACE_INFO(LX(TAG, "AuthImpl::onAuthStateChange").d("state",newState).d("error",error));

    if (m_authState != newState) {
        m_authState = newState;
        fetchAuthToken();
        if (!m_accessToken.empty()) {
            if (m_registrationChangedFn != nullptr) {
                m_registrationChangedFn(true);
            }
        }
    }
    AACE_DEBUG(LX(TAG, "onAuthStateChange").d("token available", !m_accessToken.empty()));
}

void AuthImpl::onRegistrationChanged(bool isRegistered) {
    std::function<void(bool)> callback = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_lock);
        callback = m_registrationChangedFn;
    }

    if (callback != nullptr) {
        callback(isRegistered);
    }
}

void AuthImpl::fetchAuthToken() const {
   m_accessToken = m_authDelegate->getAuthToken();
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace