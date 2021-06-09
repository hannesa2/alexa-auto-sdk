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

#ifndef AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_INTERFACE_H
#define AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_INTERFACE_H

#include <string>

/** @file */

namespace aace {
namespace localNavigation {

/**
 * @c LocalSearchProviderEngineInterface is an interface invoked by the @c LocalSearchProvider to provide search and
 * lookup responses to the Engine.
 */
class LocalSearchProviderEngineInterface {
public:
    virtual ~LocalSearchProviderEngineInterface() = default;

    /**
     * Handle an asynchronous response to @c poiSearchRequest() from the @c LocalSearchProvider implementation.
     * The format of @a response is as follows:
     *    TODO
     * The @c requestID corresponds to the ID from the original search request.
     *
     * @note The @c LocalSearchProvider may call this method on the calling thread of
     * @c LocalSearchProvider::poiSearchRequest(). The implementation of this method must not deadlock.
     *
     * @param response A search response in the specified format.
     */
    virtual void onPoiSearchResponse(const std::string& response) = 0;

    /**
     * Handle an asynchronous response to @c poiLookupRequest() from the
     * @c LocalSearchProvider implementation.
     * The format of @a response is as follows:
     *    TODO
     * The @c requestId corresponds to the ID from the original lookup request.
     *
     * @note The @c LocalSearchProvider may call this method on the calling thread of
     * @c LocalSearchProvider::poiLookupRequest(). The implementation of this method must not deadlock.
     *
     * @param response A lookup response in the specified format.
     */
    virtual void onPoiLookupResponse(const std::string& response) = 0;
};

}  // namespace localNavigation
}  // namespace aace

#endif  // AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_ENGINE_INTERFACE_H