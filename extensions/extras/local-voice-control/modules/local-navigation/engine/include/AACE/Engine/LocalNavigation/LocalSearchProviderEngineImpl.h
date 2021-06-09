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

#ifndef AACE_ENGINE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_IMPL_H

#include <future>
#include <mutex>
#include <string>
#include <unordered_map>

#include <localsearch/IPOISearch.h>

#include <AACE/LocalNavigation/LocalSearchProvider.h>
#include <AACE/LocalNavigation/LocalSearchProviderEngineInterface.h>

namespace aace {
namespace engine {
namespace localNavigation {

/**
 * @c LocalSearchProviderEngineImpl implements both the @c LocalSearchProviderEngineInterface and @c IPOISearch
 * interfaces. The implementation is responsible for translating synchronous @c IPOISearch queries from the local
 * POI service to asynchronous @c LocalSearchProvider platform interface method invocations and callbacks.
 *
 * @note This class is thread safe.
 */
class LocalSearchProviderEngineImpl
        : public aace::localNavigation::LocalSearchProviderEngineInterface
        , public localSearch::IPOISearch {
public:
    /**
     * Create a @c LocalSearchProviderEngineImpl.
     *
     * @param localSearchProvider The @c LocalSearchProvider platform interface instance.
     */
    static std::shared_ptr<LocalSearchProviderEngineImpl> create(
        std::shared_ptr<aace::localNavigation::LocalSearchProvider> localSearchProvider);

    /// @name @c IPOISearch methods
    /// @{
    std::string search(const std::string& searchRequest) override;
    std::string lookup(const std::string& lookupRequest) override;
    /// @}

    /// @name @c LocalSearchProviderEngineInterface methods
    /// @{
    void onPoiSearchResponse(const std::string& response) override;
    void onPoiLookupResponse(const std::string& response) override;
    /// @}

    /**
     * Shut down the @c LocalSearchProviderEngineImpl.
     */
    void shutdown();

private:
    /// LocalSearchProviderEngineImpl constructor.
    LocalSearchProviderEngineImpl(std::shared_ptr<aace::localNavigation::LocalSearchProvider> localSearchProvider);

    /**
     * Utility method to extract a request ID from a request or response JSON.
     *
     * @param [in] jsonStr The request or response JSON.
     * @param [out] requestId The string reference to update with the request ID.
     * @return @c true if @a id was updated, false otherwise.
     */
    static bool getIdFromJSON(const std::string& jsonStr, std::string& requestId);

    /**
     * Utility method to add a search or lookup request with the specified ID and a corresponding response promise to
     * @c m_activeRequestsMap.
     *
     * @note Do not call this method on a thread already holding @c m_requestMutex.
     *
     * @param [in] requestId The ID of the new active request.
     * @return A @c future that holds the JSON response to the request after the promise in @c m_activeRequestsMap is
     *         updated by the response thread.
     */
    std::future<std::string> createNewRequest(const std::string& requestId);

    /**
     * Utility method to wait for a response to the request corresponding to the specified request ID. This method
     * waits for the promise in @c m_activeRequestsMap to be updated with a response or times out after the specified
     * timeout duration. The return value of this method is a JSON response in the format expected by @c IPOISearch,
     * and the value is either the response from the search provider or an error constructed on its behalf.
     *
     * @note Do not call this method on a thread already holding @c m_requestMutex.
     *
     * @param [in] requestId The ID of the request for which to await a response.
     * @param [in] responseFuture The @c future awaiting a response via an update to its corresponding promise in
     *             @c m_activeRequestsMap.
     * @param [in] waitTimeout The duration to wait before timing out and returning an error response.
     * @return A JSON response for @a requestId.
     */
    std::string waitForResponse(
        const std::string& requestId,
        std::future<std::string> responseFuture,
        std::chrono::milliseconds waitTimeout);

    /**
     * Utility method to process search and lookup responses.
     * This utility method exists because logic to handle search responses and lookup responses is identical.
     *
     * @note Do not call this method on a thread already holding @c m_requestMutex.
     */
    void handleResponse(const std::string& response);

    /**
     * Utility method to construct an error response JSON for a search or lookup request with the specified request ID
     * and message.
     *
     * @param [in] id The request ID.
     * @param [in] message The error message.
     * @return The error response in the JSON format expected by @c IPOISearch.
     */
    static std::string createErrorResponse(const std::string& id, const std::string& message = "Unknown error");

    /**
     * Utility method to clean up a completed request by removing the specified request ID from @c m_activeRequestsMap.
     *
     * @note Do not call this method on a thread already holding @c m_requestMutex.
     *
     * @param [in] requestId The ID of the completed request to remove.
     */
    void removeCompletedRequest(const std::string& requestId);

    /// The @c LocalSearchProvider platform interface instance.
    std::shared_ptr<aace::localNavigation::LocalSearchProvider> m_localSearchProvider;

    /**
     * A map of active request IDs to their future responses. Access is serialzed with @c m_requestMutex.
     *
     * @note An entry in this map only exists while its corresponding @c search() or @c lookup() is waiting, and the
     * entry is deleted before returning from the method in which the promise was created.
     */
    std::unordered_map<std::string, std::shared_ptr<std::promise<std::string>>> m_activeRequestsMap;

    /// The mutex used to serialize access to @c m_activeRequestsMap.
    std::mutex m_requestMutex;
};

}  // namespace localNavigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_IMPL_H
