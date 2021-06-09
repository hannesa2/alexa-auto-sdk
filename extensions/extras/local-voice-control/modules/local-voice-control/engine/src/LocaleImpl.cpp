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

#include "AACE/Engine/LocalVoiceControl/LocaleImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.LocaleImpl");

LocaleImpl::LocaleImpl(std::string locale) : m_locale( locale ) {
}

AlexaHybrid::SDK::Result<std::vector<std::string>> LocaleImpl::getLocales() const {
    AACE_INFO(LX(TAG, "Collecting the currently selected locale and passing down to LVC:").d("locale: ", m_locale));
    std::vector<std::string> locales = {m_locale};
    return AlexaHybrid::SDK::Result<std::vector<std::string>>::create(locales);
}

void LocaleImpl::addObserver(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.insert( sharedObs );
        AACE_INFO(LX(TAG, "LVC registered observer").d("observers size", m_observers.size()));
    }
}

void LocaleImpl::removeObserver(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock( m_mutex );
    if (auto sharedObs = observer.lock()) {
        m_observers.erase( sharedObs );
        AACE_INFO(LX(TAG, "LVC deregistered observer").d("observers size", m_observers.size()));
    }
}

void LocaleImpl::onLocaleChange(std::string locale) {
    //update current locale
    AACE_INFO(LX(TAG, "onLocaleChange").d("current", m_locale).d("new", locale));
    m_locale = locale;

    //notify all observers
    std::lock_guard<std::mutex> lock( m_mutex );
    for (auto observer : m_observers) {
        AACE_INFO(LX(TAG, "onLocaleChange").d("observer->onLocaleChange", m_locale));
        std::vector<std::string> locales = {locale};
        observer->onLocaleChange(locales);
    }
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace
