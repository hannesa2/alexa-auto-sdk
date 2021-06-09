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

#include "AACE/LocalNavigation/LocalSearchProvider.h"

namespace aace {
namespace localNavigation {

LocalSearchProvider::~LocalSearchProvider() = default;

void LocalSearchProvider::setEngineInterface(std::shared_ptr<LocalSearchProviderEngineInterface> engineInterface) {
    m_engineInterface = engineInterface;
}

void LocalSearchProvider::poiSearchResponse(const std::string& response) {
    if (m_engineInterface != nullptr) {
        m_engineInterface->onPoiSearchResponse(response);
    }
}

void LocalSearchProvider::poiLookupResponse(const std::string& response) {
    if (m_engineInterface != nullptr) {
        m_engineInterface->onPoiLookupResponse(response);
    }
}

}  // namespace localNavigation
}  // namespace aace
