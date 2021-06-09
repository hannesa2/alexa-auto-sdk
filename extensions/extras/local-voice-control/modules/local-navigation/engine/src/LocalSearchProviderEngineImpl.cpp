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

#include <nlohmann/json.hpp>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/LocalNavigation/LocalSearchProviderEngineImpl.h"

namespace aace {
namespace engine {
namespace localNavigation {

using json = nlohmann::json;

/**
 * The duration to wait for a search response from @c m_localSearchProvider.
 * @c note: Increasing this timeout beyond the ER timeout configured for AHE has no positive affect on UX. This value
 * ensures the ER thread is not blocked indefinitely by defining a @c IPOISearch return timeout, and it should be less
 * than or equal to AHE's ER timeout.
 */
static const std::chrono::seconds SEARCH_TIMEOUT{4};

/// The duration to wait for a lookup response from @c m_localSearchProvider.
static const std::chrono::seconds LOOKUP_TIMEOUT{10};

/// The key for the request ID in an @c IPOISearch search/lookup request/response.
static const std::string REQUEST_ID_KEY = "requestId";

/// The response status used by @c IPOISearch for an error.
static const std::string ERROR_RESPONSE_STATUS = "FAIL";

/// The error code used by @c IPOISearch for a response constructed internally on behalf of @c m_localSearchProvider.
static const std::string ERROR_CODE_INTERNAL = "INTERNAL_ERROR";

/// A string to identify log entries originating from this file.
static const std::string TAG("LocalSearchProviderEngineImpl");

std::shared_ptr<LocalSearchProviderEngineImpl> LocalSearchProviderEngineImpl::create(
    std::shared_ptr<aace::localNavigation::LocalSearchProvider> localSearchProvider) {
    if (localSearchProvider == nullptr) {
        AACE_ERROR(LX(TAG).m("create failed").m("null LocalSearchProvider platform interface"));
        return nullptr;
    }
    std::shared_ptr<LocalSearchProviderEngineImpl> engineImpl =
        std::shared_ptr<LocalSearchProviderEngineImpl>(new LocalSearchProviderEngineImpl(localSearchProvider));
    localSearchProvider->setEngineInterface(engineImpl);
    return engineImpl;
}

LocalSearchProviderEngineImpl::LocalSearchProviderEngineImpl(
    std::shared_ptr<aace::localNavigation::LocalSearchProvider> localSearchProvider) :
        m_localSearchProvider(localSearchProvider) {
}

std::string LocalSearchProviderEngineImpl::search(const std::string& searchRequest) {
    AACE_DEBUG(LX(TAG).sensitive("searchRequest", searchRequest));

    std::string currentRequestId;
    bool requestValid = getIdFromJSON(searchRequest, currentRequestId);
    if (!requestValid) {
        AACE_ERROR(LX(TAG).m("Search request missing ID"));
        return createErrorResponse("", "Invalid search request had no ID");
    }

    // Create new request in the active request map before invoking platform interface poiSearchRequest() in case
    // the response callback is invoked on this thread before poiSearchRequest() returns. Also do not hold the mutex
    // over the platform interface request so we do not deadlock in this same case.
    std::future<std::string> responseFuture = createNewRequest(currentRequestId);

    bool willHandleRequest = m_localSearchProvider->poiSearchRequest(searchRequest);
    if (!willHandleRequest) {
        AACE_ERROR(LX(TAG).m("LocalSearchProvider won't handle search request").d("request ID", currentRequestId));
        removeCompletedRequest(currentRequestId);
        return createErrorResponse(currentRequestId, "Provider won't handle request");
    }

    std::string response = waitForResponse(currentRequestId, std::move(responseFuture), SEARCH_TIMEOUT);
    removeCompletedRequest(currentRequestId);
    AACE_DEBUG(LX(TAG).m("Returning response").sensitive("response", response));
    return response;
}

std::string LocalSearchProviderEngineImpl::lookup(const std::string& lookupRequest) {
    AACE_DEBUG(LX(TAG).sensitive("lookupRequest", lookupRequest));

    std::string currentRequestId;
    bool requestValid = getIdFromJSON(lookupRequest, currentRequestId);
    if (!requestValid) {
        AACE_ERROR(LX(TAG).m("Lookup request missing ID"));
        return createErrorResponse("", "Invalid lookup request had no ID");
    }
    // Create new request in the active request map before invoking platform interface poiLookupRequest() in case
    // the response callback is invoked on this thread before poiLookupRequest() returns. Also do not hold the mutex
    // over the platform interface request so we do not deadlock in this same case.
    std::future<std::string> responseFuture = createNewRequest(currentRequestId);

    bool willHandleRequest = m_localSearchProvider->poiLookupRequest(lookupRequest);
    if (!willHandleRequest) {
        AACE_ERROR(LX(TAG).m("LocalSearchProvider won't handle lookup request").d("request ID", currentRequestId));
        removeCompletedRequest(currentRequestId);
        return createErrorResponse(currentRequestId, "Provider won't handle request");
    }

    std::string response = waitForResponse(currentRequestId, std::move(responseFuture), LOOKUP_TIMEOUT);
    removeCompletedRequest(currentRequestId);
    AACE_DEBUG(LX(TAG).m("Returning response").sensitive("response", response));
    return response;
}

std::future<std::string> LocalSearchProviderEngineImpl::createNewRequest(const std::string& requestId) {
    std::lock_guard<std::mutex> lock(m_requestMutex);
    std::shared_ptr<std::promise<std::string>> responsePromise = std::make_shared<std::promise<std::string>>();
    m_activeRequestsMap[requestId] = responsePromise;
    return responsePromise->get_future();
}

std::string LocalSearchProviderEngineImpl::waitForResponse(
    const std::string& requestId,
    std::future<std::string> responseFuture,
    std::chrono::milliseconds waitTimeout) {
    std::string response;
    auto status = responseFuture.wait_for(waitTimeout);
    if (status == std::future_status::timeout) {
        AACE_WARN(LX(TAG)
                      .m("Timed out waiting for response from LocalSearchProvider. Returning error response")
                      .d("request ID", requestId));
        response = createErrorResponse(requestId, "Provider timed out");
    } else if (status == std::future_status::ready) {
        // We do not check for exceptions set on the promise because we do not use them in the search callback
        response = responseFuture.get();
        AACE_DEBUG(LX(TAG).m("Received response with matching ID").d("request ID", requestId));
    }
    return response;
}

void LocalSearchProviderEngineImpl::onPoiSearchResponse(const std::string& response) {
    AACE_DEBUG(LX(TAG).sensitive("response", response));
    handleResponse(response);
}

void LocalSearchProviderEngineImpl::onPoiLookupResponse(const std::string& response) {
    AACE_DEBUG(LX(TAG).sensitive("response", response));
    handleResponse(response);
}

void LocalSearchProviderEngineImpl::handleResponse(const std::string& response) {
    std::lock_guard<std::mutex> lock(m_requestMutex);
    std::string requestId;
    bool responseValid = getIdFromJSON(response, requestId);
    if (!responseValid) {
        AACE_WARN(LX(TAG).m("Ignoring response without request ID"));
        return;
    }
    auto it = m_activeRequestsMap.find(requestId);
    if (m_activeRequestsMap.end() == it) {
        AACE_WARN(LX(TAG).m("No active request with matching ID").m("Ignoring response").d("request ID", requestId));
        return;
    }
    std::shared_ptr<std::promise<std::string>> responsePromise = it->second;
    responsePromise->set_value(response);
}

void LocalSearchProviderEngineImpl::removeCompletedRequest(const std::string& requestId) {
    AACE_DEBUG(LX(TAG).m("removing request ID from list of active requests").d("requestId", requestId));
    std::lock_guard<std::mutex> lock(m_requestMutex);
    m_activeRequestsMap.erase(requestId);
}

bool LocalSearchProviderEngineImpl::getIdFromJSON(const std::string& jsonStr, std::string& requestId) {
    json j;
    try {
        j = json::parse(jsonStr);
    } catch (json::parse_error& ex) {
        AACE_ERROR(LX(TAG).m("jsonStr is not valid JSON").d("exception", ex.what()));
        return false;
    }

    try {
        requestId = j.at(REQUEST_ID_KEY);
    } catch (json::exception& ex) {
        AACE_ERROR(LX(TAG).m(REQUEST_ID_KEY + " is missing or not a string").d("exception", ex.what()));
        return false;
    }
    return true;
}

std::string LocalSearchProviderEngineImpl::createErrorResponse(const std::string& id, const std::string& message) {
    // clang-format off
    json jResponse = {
        {REQUEST_ID_KEY, id},
        {"status", ERROR_RESPONSE_STATUS},
        {"error", {
            {"errorCode", ERROR_CODE_INTERNAL},
            {"errorMessage", message},
        }}
    };
    // clang-format on
    return jResponse.dump();
}

void LocalSearchProviderEngineImpl::shutdown() {
    AACE_DEBUG(LX(TAG));
    m_localSearchProvider->setEngineInterface(nullptr);
    m_localSearchProvider.reset();

    std::lock_guard<std::mutex> lock(m_requestMutex);
    for (auto& request : m_activeRequestsMap) {
        (request.second).reset();
    }
    m_activeRequestsMap.clear();
}

}  // namespace localNavigation
}  // namespace engine
}  // namespace aace
