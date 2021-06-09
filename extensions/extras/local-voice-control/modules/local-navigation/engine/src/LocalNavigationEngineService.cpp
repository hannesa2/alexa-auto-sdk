/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <nlohmann/json.hpp>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/LocalNavigation/LocalNavigationEngineService.h"

namespace aace {
namespace engine {
namespace localNavigation {

using json = nlohmann::json;

/// String to identify log entries originating from this file.
static const std::string TAG("LocalNavigationEngineService");

/// Register the LocalNavigationEngineService with the Engine
REGISTER_SERVICE(LocalNavigationEngineService);

LocalNavigationEngineService::LocalNavigationEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool LocalNavigationEngineService::configure(std::shared_ptr<std::istream> configuration) {
    json jconfiguration;
    try {
        jconfiguration = json::parse(*configuration);
    } catch (json::parse_error& ex) {
        AACE_ERROR(LX(TAG).m("configuration is not valid JSON").d("exception", ex.what()));
        return false;
    }
    AACE_DEBUG(LX(TAG).sensitive("configuration", jconfiguration.dump()));

    // aace.localNavigation.localSearch
    json localSearch;
    try {
        localSearch = jconfiguration.at("localSearch");
    } catch (json::exception& ex) {
        AACE_ERROR(LX(TAG).m("invalid localSearch node").d("exception", ex.what()));
        return false;
    }

    // aace.localNavigation.localSearch.navigationPOISocketPath
    std::string navPoiSocketPath;
    try {
        navPoiSocketPath = localSearch.at("navigationPOISocketPath");
    } catch (json::exception& ex) {
        AACE_ERROR(LX(TAG).m("missing or invalid navigationPOISocketPath").d("exception", ex.what()));
        return false;
    }
    if (navPoiSocketPath.empty()) {
        AACE_ERROR(LX(TAG).m("empty navigationPOISocketPath"));
        return false;
    }

    // aace.localNavigation.localSearch.poiEERSocketPath
    std::string poiEERSocketPath;
    try {
        poiEERSocketPath = localSearch.at("poiEERSocketPath");
    } catch (json::exception& ex) {
        AACE_ERROR(LX(TAG).m("missing or invalid poiEERSocketPath").d("exception", ex.what()));
        return false;
    }
    if (poiEERSocketPath.empty()) {
        AACE_ERROR(LX(TAG).m("empty poiEERSocketPath"));
        return false;
    }

    // Retrieve path to LSS socket for POI ER service to emit metrics
    auto localSkillService =
        getContext()->getServiceInterface<aace::engine::localSkillService::LocalSkillServiceInterface>(
            "aace.localSkillService");
    if (localSkillService == nullptr) {
        AACE_ERROR(LX(TAG).m("LocalSkillServiceEngineService is null"));
        return false;
    }
    const std::string lssSocketPath = localSkillService->getLSSSocketPath();
    if (lssSocketPath.empty()) {
        AACE_ERROR(LX(TAG).m("LSS socket path is not configured"));
        return false;
    }

    m_poiERServiceManager = localSearch::services::IPOIERServiceManager::create();
    if (m_poiERServiceManager == nullptr) {
        AACE_ERROR(LX(TAG).m("failed to create IPOIERServiceManager"));
        return false;
    }
    // clang-format off
    json poiServiceConfigJson = {
        {"navigationPOISocketPath", navPoiSocketPath},
        {"poiEERSocketPath", poiEERSocketPath},
        {"localSkillServiceSocketPath", lssSocketPath},
        {"permissions", "OWNER"}
    };
    // clang-format on
    const std::string poiServiceConfig = poiServiceConfigJson.dump();
    AACE_DEBUG(LX(TAG).m("Configuring IPOIERServiceManager").sensitive("config", poiServiceConfig));
    m_poiERServiceManager->configure(poiServiceConfig);
    return true;
}

bool LocalNavigationEngineService::setup() {
    if (m_localSearchProviderEngineImpl == nullptr) {
        AACE_DEBUG(LX(TAG).m("LocalSearchProvider is not registered. Doing nothing"));
        return true;
    }
    if (m_poiERServiceManager == nullptr) {
        // Since local search is the only feature of this module, it is a fatal error if the service manager doesn't
        // exist when the provider is registered
        AACE_ERROR(LX(TAG).m("Cannot register IPOISearch provider because IPOIERServiceManager is null"));
        return false;
    }
    AACE_DEBUG(LX(TAG).m("Registering IPOISearch provider"));
    m_poiERServiceManager->setSearchProvider(m_localSearchProviderEngineImpl);
    return true;
}

bool LocalNavigationEngineService::start() {
    if (m_localSearchProviderEngineImpl == nullptr) {
        AACE_DEBUG(LX(TAG).m("LocalSearchProvider is not registered. Doing nothing"));
        return true;
    }
    if (m_poiERServiceManager == nullptr) {
        // Since local search is the only feature of this module, it is a fatal error if the service manager doesn't
        // exist when the provider is registered
        AACE_ERROR(LX(TAG).m("Cannot start IPOIERServiceManager because it is null"));
        return false;
    }
    AACE_DEBUG(LX(TAG).m("Starting IPOIERServiceManager"));
    if (!m_poiERServiceManager->start()) {
        AACE_ERROR(LX(TAG).m("Failed to start IPOIERServiceManager"));
        return false;
    }
    return true;
}

bool LocalNavigationEngineService::stop() {
    AACE_DEBUG(LX(TAG));
    if (m_poiERServiceManager != nullptr) {
        if (!m_poiERServiceManager->stop()) {
            // This is not a fatal error blocking Engine stop - just log an error
            AACE_ERROR(LX(TAG).m("Failed to stop IPOIERServiceManager"));
        }
    }
    return true;
}

bool LocalNavigationEngineService::shutdown() {
    AACE_DEBUG(LX(TAG));
    if (m_poiERServiceManager != nullptr) {
        m_poiERServiceManager->setSearchProvider(nullptr);
        if (!m_poiERServiceManager->stop()) {
            // This is not a fatal error blocking shutdown - just log an error
            AACE_ERROR(LX(TAG).m("failed to stop IPOIERServiceManager"));
        }
        m_poiERServiceManager.reset();
    }
    if (m_localSearchProviderEngineImpl != nullptr) {
        m_localSearchProviderEngineImpl->shutdown();
        m_localSearchProviderEngineImpl.reset();
    }
    return true;
}

bool LocalNavigationEngineService::registerPlatformInterface(
    std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
    return registerPlatformInterfaceType<aace::localNavigation::LocalSearchProvider>(platformInterface);
}

bool LocalNavigationEngineService::registerPlatformInterfaceType(
    std::shared_ptr<aace::localNavigation::LocalSearchProvider> localSearchProvider) {
    AACE_DEBUG(LX(TAG));
    m_localSearchProviderEngineImpl =
        aace::engine::localNavigation::LocalSearchProviderEngineImpl::create(localSearchProvider);
    if (m_localSearchProviderEngineImpl == nullptr) {
        AACE_ERROR(LX(TAG).m("Failed to create LocalSearchProviderEngineImpl"));
        return false;
    }
    return true;
}

}  // namespace localNavigation
}  // namespace engine
}  // namespace aace