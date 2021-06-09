/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_BINDER_H
#define AACE_JNI_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_BINDER_H

#include <AACE/JNI/Core/PlatformInterfaceBinder.h>
#include <AACE/LocalNavigation/LocalSearchProvider.h>

namespace aace {
namespace jni {
namespace localNavigation {

class LocalSearchProviderHandler : public aace::localNavigation::LocalSearchProvider {
public:
    LocalSearchProviderHandler(jobject obj);

    /// @name @c aace::localNavigation::LocalSearchProvider methods
    /// @{
    bool poiSearchRequest(const std::string& request) override;
    bool poiLookupRequest(const std::string& request) override;
    /// @}
private:
    JObject m_obj;
};

class LocalSearchProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    LocalSearchProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_localSearchProviderHandler;
    }

    std::shared_ptr<LocalSearchProviderHandler> getLocalSearchProviderHandler() {
        return m_localSearchProviderHandler;
    }

private:
    std::shared_ptr<LocalSearchProviderHandler> m_localSearchProviderHandler;
};

}  // namespace localNavigation
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_BINDER_H