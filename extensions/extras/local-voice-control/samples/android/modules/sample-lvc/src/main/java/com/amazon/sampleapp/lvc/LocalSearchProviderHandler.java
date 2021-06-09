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

package com.amazon.sampleapp.lvc;

import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

import com.amazon.aace.localNavigation.LocalSearchProvider;

/**
 * This sample implementation of LocalSearchProvider provides sample error
 * responses to search and lookup requests from the Engine. In order for offline
 * POI search and navigation features to work, your implementation must delegate
 * search and lookup requests to a navigation provider with offline search
 * functionality.
 */
public class LocalSearchProviderHandler extends LocalSearchProvider {
    private static final String TAG = LocalSearchProviderHandler.class.getSimpleName();

    @Override
    public boolean poiSearchRequest(final String request) {
        Log.i(TAG, "Handling POI search request");
        String requestId = getRequestId(request);
        if (requestId == null) {
            Log.e(TAG, "Could not extract requestId from search request");
            return false;
        }
        // Your implementation should delegate the request to a navigation provider, return from
        // this method immediately, and call the poiSearchResponse() callback when the async result
        // is available from the provider. This sample produces an error response instead.
        String response = createErrorResponse(requestId);
        Log.i(TAG, "Using error response: " + response);
        poiSearchResponse(response);

        return true;
    }

    @Override
    public boolean poiLookupRequest(final String request) {
        Log.i(TAG, "Handling POI lookup request");
        String requestId = getRequestId(request);
        if (requestId == null) {
            Log.e(TAG, "Could not extract requestId from search request");
            return false;
        }
        // Your implementation should delegate the request to a navigation provider, return from
        // this method immediately, and call the poiLookupResponse() callback when the async result
        // is available from the provider. This sample produces an error response instead.
        String response = createErrorResponse(requestId);
        Log.i(TAG, "Using error response: " + response);
        poiLookupResponse(response);

        return true;
    }

    /**
     * Utility method to get the request ID from the specified search or lookup request.
     *
     * @param request A search or lookup request in LocalSearchProvider JSON format.
     * @return @Nullable The request ID from the specified request.
     */
    private String getRequestId(String request) {
        try {
            JSONObject requestJSON = new JSONObject(request);
            return requestJSON.getString("requestId");
        } catch (JSONException e) {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

    /**
     * Utility method to create a sample error response for a request with the specified request ID.
     *
     * @param requestId The request ID for which to generate a sample error response.
     * @return An error response in the format required by LocalSearchProvider.
     */
    private String createErrorResponse(String requestId) {
        JSONObject errorResponse = new JSONObject();
        try {
            errorResponse.put("requestId", requestId);
            errorResponse.put("status", "FAIL");
            JSONObject errorDetails = new JSONObject();
            errorDetails.put("errorCode", "INTERNAL_ERROR");
            errorDetails.put("errorMessage", "Default sample does not implement search");
            errorResponse.put("error", errorDetails);
        } catch (JSONException e) {
            Log.e(TAG, e.getMessage());
        }
        return errorResponse.toString();
    }
}