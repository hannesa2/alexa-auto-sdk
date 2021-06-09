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

#ifndef AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_H
#define AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_H

#include <memory>
#include <string>
#include <AACE/Core/PlatformInterface.h>
#include "LocalSearchProviderEngineInterface.h"

/** @file */

namespace aace {
namespace localNavigation {

/**
 * Extend the @c LocalSearchProvider platform interface to allow the user to search for and navigate to points of
 * interest (POIs), such as categories (e.g., "Italian restaurants"), chains (e.g., "Starbucks"), and entities
 * (e.g., "Golden Gate Bridge"), when the head unit is not connected to the internet. When the user makes a request to
 * Alexa, Alexa generates search queries based on the request and depends on the @c LocalSearchProvider implementation
 * to use the offline geo-catalog of an onboard navigation provider to return results relevant to the query.
 */
class LocalSearchProvider : public aace::core::PlatformInterface {
protected:
    LocalSearchProvider() = default;

public:
    virtual ~LocalSearchProvider();

    /**
     * Use the specified search query to find specific points of interest relevant to a user request.
     * The format of @a request matches the following example:
     * @code{.json}
     * {
     *    "requestId": "4da2419b-6596-4a51-9707-80fbd6960e3b",
     *    "query": "Coffee Shops",
     *    "queryType": "CATEGORY",
     *    "rankingStrategy": "RELEVANCE",
     *    "numOfResults": 8,
     *    "geolocation": [
     *        37.8043514,
     *        -122.2711639
     *    ],
     *    "locale": "en-US",
     *    "searchLocation": {
     *        "city": "San Francisco"
     *    }
     * }
     * @endcode
     *
     * See the Local Navigation module documentation for complete details of the request schema.
     *
     * @note This is an asynchronous request, and the implementation should return immediately. After returning @c true,
     * call @c poiSearchResponse() with the search results and the same @c requestId when the search response is
     * available.
     *
     * @param request A search request in the specified format.
     * @return @c true if the implmentation will attempt to fulfill the search request, false otherwise.
     */
    virtual bool poiSearchRequest(const std::string& request) = 0;

    /**
     * Asynchronously respond to a @c poiSearchRequest() query with results of the search. The @c requestId corresponds
     * to the ID from the original search request.
     *
     * The format of @a response matches the following example:
     * @code{.json}
     * {
     *    "requestId": "4da2419b-6596-4a51-9707-80fbd6960e3b",
     *    "status": "SUCCESS",
     *    "data": {
     *        "totalNumResults": 2,
     *        "results": [
     *            {
     *                "type": "ENTITY",
     *                "score": 100,
     *                "confidence": "HIGH",
     *                "provider": "TomTom",
     *                "id": "4111",
     *                "address": {
     *                    "city": "San Francisco",
     *                    "addressLine1": "525 Golden Gate Ave",
     *                    "stateOrRegion": "CA",
     *                    "countryCode": "USA",
     *                    "postalCode": "94102"
     *                },
     *                "poi": {
     *                    "name": "Hayes Valley Bakeworks",
     *                    "phoneNumber": "+1 415 6587185"
     *                },
     *                "routingInfo": {
     *                    "travelDistance": {
     *                        "straightLineDistanceInMeters": 13298.17
     *                    }
     *                },
     *                "navigationPosition": [
     *                    37.78101,
     *                    -122.41925
     *                ]
     *            },
     *            {
     *                "type": "ENTITY",
     *                "score": 100,
     *                "confidence": "HIGH",
     *                "provider": "TomTom",
     *                "id": "4112",
     *                "address": {
     *                    "city": "San Francisco",
     *                    "addressLine1": "601 Van Ness Ave",
     *                    "stateOrRegion": "CA",
     *                    "countryCode": "USA",
     *                    "postalCode": "94102"
     *                },
     *                "poi": {
     *                    "name": "Peet's Coffee & Tea",
     *                    "phoneNumber": "+1 415 5932661"
     *                },
     *                "routingInfo": {
     *                    "travelDistance": {
     *                        "straightLineDistanceInMeters": 13476.75
     *                    }
     *                },
     *                "navigationPosition": [
     *                    37.78139,
     *                    -122.42141
     *                ]
     *            }
     *        ]
     *    }
     * }
     * @endcode
     *
     * See the Local Navigation module documentation for complete details of the response schema.
     *
     * @note The Engine expects calls to this method to happen on a separate thread from @c poiSearchRequest(); however,
     * it is thread safe to call this before returning from @c poiSearchRequest() if the response is synchronous,
     * such as in an error condition.
     *
     * @param response A search response in the specified format.
     */
    void poiSearchResponse(const std::string& response);

    /**
     * Use the specified lookup query to provide additional details about a POI returned in @c poiSearchResponse().
     * This request is a followup action from @c poiSearchRequest() used to query additional details about a POI
     * selected by the user when such details have a high computation time, such as navigation time and distance.
     *
     * The format of @a request matches the following example:
     * @code{.json}
     * {
     *    "requestId": "alexahybrid.ask.request.8a81089a-4ab9-4336-b31a-de92b28317e5",
     *    "lookupIds": [
     *        "4112"
     *    ]
     * }
     * @endcode
     *
     * See the Local Navigation module documentation for complete details of the request schema.
     *
     * @note This is an asynchronous request, and the implementation should return immediately. After returning @c true,
     * call @c poiLookupResponse() with the lookup results and the same @c requestId when the lookup response is
     * available.
     *
     * @param request A lookup request in the specified format.
     * @return @c true if the implmentation will attempt to fulfill the lookup request, false otherwise.
     */
    virtual bool poiLookupRequest(const std::string& request) = 0;

    /**
     * Asynchronously respond to a @c poiLookupRequest() query with results of the lookup. The @c requestId corresponds
     * to the ID from the original lookup request.
     *
     * The format of @a response matches the following example:
     * @code{.json}
     * {
     *    "requestId": "alexahybrid.ask.request.8a81089a-4ab9-4336-b31a-de92b28317e5",
     *    "status": "SUCCESS",
     *    "data": {
     *        "results": [
     *            {
     *                "type": "ENTITY",
     *                "score": 100,
     *                "confidence": "HIGH",
     *                "provider": "TomTom",
     *                "id": "4112",
     *                "address": {
     *                    "city": "San Francisco",
     *                    "addressLine1": "601 Van Ness Ave",
     *                    "stateOrRegion": "CA",
     *                    "countryCode": "USA",
     *                    "postalCode": "94102"
     *                },
     *                "poi": {
     *                    "name": "Peet's Coffee & Tea",
     *                    "phoneNumber": "+1 415 5932661"
     *                },
     *                "routingInfo": {
     *                    "travelDistance": {
     *                        "straightLineDistanceInMeters": 13476.75,
     *                        "navigationDistanceInMeters": 19297
     *                    },
     *                    "travelTime": {
     *                        "predicted": "2020-12-07T10:06:49.400-0800"
     *                    }
     *                },
     *                "navigationPosition": [
     *                    37.78139,
     *                    -122.42141
     *                ]
     *            }
     *        ]
     *    }
     * }
     * @endcode
     *
     * See the Local Navigation module documentation for complete details of the response schema.
     *
     * @note The Engine expects calls to this method to happen on a separate thread from @c poiLookupRequest(); however,
     * it is thread safe to call this before returning from @c poiLookupRequest() if the response is synchronous, such
     * as in an error condition.
     *
     * @param response A lookup response in the specified format.
     */
    void poiLookupResponse(const std::string& response);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::localNavigation::LocalSearchProviderEngineInterface> engineInterface);

private:
    std::shared_ptr<aace::localNavigation::LocalSearchProviderEngineInterface> m_engineInterface;
};

}  // namespace localNavigation
}  // namespace aace

#endif  // AACE_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_H