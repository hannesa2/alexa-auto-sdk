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

#ifndef AASB_ENGINE_LOCAL_NAVIGATION_AASB_LOCAL_NAVIGATION_H
#define AASB_ENGINE_LOCAL_NAVIGATION_AASB_LOCAL_NAVIGATION_H

#include <AACE/LocalNavigation/LocalSearchProvider.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace localNavigation {

class AASBLocalSearchProvider :
        public aace::localNavigation::LocalSearchProvider,
        public std::enable_shared_from_this<AASBLocalSearchProvider> {

private:
    AASBLocalSearchProvider() = default;

    bool initialize( std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker );

public:
    static std::shared_ptr<AASBLocalSearchProvider> create( std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker );

    /// @name LocalSearchProvider methods
    /// @{
    bool poiSearchRequest(const std::string& request) override;
    bool poiLookupRequest(const std::string& request) override;
    /// @}

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

} // aasb::engine::localNavigation
} // aasb::engine
} // aasb

#endif // AASB_ENGINE_LOCAL_NAVIGATION_AASB_LOCAL_NAVIGATION_H
