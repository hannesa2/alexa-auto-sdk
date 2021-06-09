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

package com.amazon.alexaautoclientservice.lvc;

import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

import com.amazon.aace.localNavigation.LocalSearchProvider;

public class LocalSearchProviderHandler extends LocalSearchProvider {
    private static final String TAG = LocalSearchProviderHandler.class.getSimpleName();

    public LocalSearchProviderHandler() {}

    @Override
    public boolean poiSearchRequest(final String request) {
        String requestId = getRequestId(request);
        if (requestId == null) {
            return false;
        }
        String response = createErrorResponse(requestId);
        poiSearchResponse(response);
        return true;
    }

    @Override
    public boolean poiLookupRequest(final String request) {
        String requestId = getRequestId(request);
        if (requestId == null) {
            return false;
        }
        String response = createErrorResponse(requestId);
        poiLookupResponse(response);
        return true;
    }

    private String getRequestId(String request) {
        try {
            JSONObject requestJSON = new JSONObject(request);
            return requestJSON.getString("requestId");
        } catch (JSONException e) {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

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

