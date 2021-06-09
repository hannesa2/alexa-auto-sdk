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

#ifndef AACE_LOCAL_VOICE_CONTROL_AUTHIMPL_H
#define AACE_LOCAL_VOICE_CONTROL_AUTHIMPL_H

#include <mutex>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include "alexa_hybrid/sdk/IAuth.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class AuthImpl : 
    public AlexaHybrid::SDK::IAuth,
    public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface,
    public std::enable_shared_from_this<AuthImpl> {

public:
    /**
     * Default constructor.
     */
    AuthImpl();

    /**
     * Saves reference to the auth delegate and registers this object as an observer.
     */
    void setAuthDelegate( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate );

public:  
    /**
     * Check if device is registered.
     *
     * @return @c true if device is registered to a user, @c false otherwise.
     */
    AlexaHybrid::SDK::Result<bool> isRegistered() const override; 

    /**
     * Obtain OAuth token for foreground user.
     *
     * @param token        Placeholder for the token.
     * @param forceRefresh true: Device will do a blocking force refresh of cached OAuth token.
     *                           It blocks until refresh happens. Use this option only if server
     *                           rejects the cached token.
     *                     false: Get the cached token. This is quick. Use this option by default.
     *
     * @return @c true on success to obtain token. @c false otherwise.
     */
    AlexaHybrid::SDK::Result<bool> getOAuthToken(std::string &token, bool forceRefresh) const override;

    /**
     * Register a callback to observe device registration changes.
     *
     * Only one callback may be registered at a time. Subsequent calls to this
     * function will overwrite the previously set callback. To remove a callback
     * @c nullptr may be passed.
     *
     * @param callback        Callback function or @c nullptr. The current registration
     *                        status will be passed to the callback function. A value of @c true
     *                        indicates that the device is registered, @c false that it is not
     *                        registered.
     */
    void setRegistrationChangedCallback(std::function<void(bool)> callback) override;

    /**
     * Callback receiver when registration changes
     *
     * @param isRegistered @c true if device is registered, @c false if device is deregistered.
     */
    void onRegistrationChanged(bool isRegistered);

public:
    /**
     * Notification that an authorization state has changed.
     *
     * @note Implementations of this method must not call AuthDelegate methods because the AuthDelegate
     * may be in a 'locked' state at the time this call is made.
     *
     * @param newState The new state of the authorization token.
     * @param error The error associated to the state change.
     */
    void onAuthStateChange(alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState, 
                           alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;


private:
    /**
     * Retrieve auth token from auth provider.
     */
    void fetchAuthToken() const;

private:
    /// Access token
    mutable std::string m_accessToken;

    /// Current auth state
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState; 

    /// Registration changed callback function
    std::function<void(bool)> m_registrationChangedFn;

    /// Auth delegate reference 
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> m_authDelegate;

    /// Mutex
    std::mutex m_lock;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_AUTHIMPL_H