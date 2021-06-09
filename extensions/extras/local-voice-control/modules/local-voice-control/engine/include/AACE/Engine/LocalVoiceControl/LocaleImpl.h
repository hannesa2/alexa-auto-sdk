/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_LOCAL_VOICE_CONTROL_LOCALE_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_LOCALE_IMPL_H

#include <memory>
#include <vector>
#include "alexa_hybrid/sdk/Result.h"
#include "alexa_hybrid/sdk/ILocale.h"
#include "AACE/Engine/Network/NetworkEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Network/NetworkEngineInterfaces.h"
#include "AACE/Engine/Network/NetworkInfoObserver.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class LocaleImpl :
    public AlexaHybrid::SDK::ILocale,
    public std::enable_shared_from_this<LocaleImpl> {

public:
    /**
     * Default constructor.
     */
    LocaleImpl();

    LocaleImpl(std::string locale);

    /**
     * Indicates the list of device locales currently selected on the device by the user (either through the
     * companion app or the on-board display).
     *
     * @return @c vector of locales (in IETF's BCP-47 format) set on the device hosting LVC.
     */
    AlexaHybrid::SDK::Result<std::vector<std::string>> getLocales() const override;

    /**
     * Adds an observer to observe value changes for any of the properties queryable on this object (ILocale).
     * Upon receiving a callback, observers are expected to read the properties they are interested in.
     *
     * Notes:
     * 1. Adding the same observer multiple times will have the same effect as adding it once, i.e.,
     * subsequent calls to this function with an observer that is already added will be ignored.
     * 2. If a weak_ptr expires the observer will be safely removed when the next callback is to be fired.
     * 3. Passing a weak_ptr to a nullptr has no effect.
     *
     * @param observer A weak pointer to an observer instance. If the observer is already added, this method has
     * no effect.
     */
    void addObserver(std::weak_ptr<IObserver> observer) override;

    /**
     * Removes a previously added observer.
     *
     * Passing a weak_ptr to a nullptr has no effect.
     *
     * @param observer A weak pointer to a previously added observer. If the observer is not added, this
     * method has no effect.
     */
    void removeObserver(std::weak_ptr<IObserver> observer) override;

    /**
     * Triggers actions needed upon a locale change on all observers.
     *
     * @param locale A string to the new locale
    */
    void onLocaleChange(std::string locale);

    /**
     * Notify observers of locale change.
     */
    void notifyObservers();

private:

    /// Observers set 
    std::unordered_set<std::shared_ptr<IObserver>> m_observers;

    /// Mutex
    std::mutex m_mutex;

    //current selected locale
    mutable std::string m_locale = "";
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_LOCALE_IMPL_H
