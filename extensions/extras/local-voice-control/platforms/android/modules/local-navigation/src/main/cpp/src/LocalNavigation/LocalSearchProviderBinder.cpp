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

#include <AACE/JNI/LocalNavigation/LocalSearchProviderBinder.h>

/// String to identify log entries originating from this file.
static const char TAG[] = "LocalSearchProviderBinder";

namespace aace {
namespace jni {
namespace localNavigation {

LocalSearchProviderBinder::LocalSearchProviderBinder(jobject obj) {
    m_localSearchProviderHandler = std::shared_ptr<LocalSearchProviderHandler>(new LocalSearchProviderHandler(obj));
}

LocalSearchProviderHandler::LocalSearchProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/localNavigation/LocalSearchProvider") {
}

bool LocalSearchProviderHandler::poiSearchRequest(const std::string& request) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("poiSearchRequest", "(Ljava/lang/String;)Z", &result, JString(request).get()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return false;
    }
}

bool LocalSearchProviderHandler::poiLookupRequest(const std::string& request) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("poiLookupRequest", "(Ljava/lang/String;)Z", &result, JString(request).get()),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return false;
    }
}

}  // namespace localNavigation
}  // namespace jni
}  // namespace aace

#define LOCAL_SEARCH_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::localNavigation::LocalSearchProviderBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_localNavigation_LocalSearchProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::localNavigation::LocalSearchProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_localNavigation_LocalSearchProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto localSearchProviderBinder = LOCAL_SEARCH_PROVIDER_BINDER(ref);
        ThrowIfNull(localSearchProviderBinder, "nullLocalSearchProviderBinder");
        delete localSearchProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_localNavigation_LocalSearchProvider_poiSearchResponse(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring response) {
    try {
        auto localSearchProviderBinder = LOCAL_SEARCH_PROVIDER_BINDER(ref);
        ThrowIfNull(localSearchProviderBinder, "nullLocalSearchProviderBinder");

        localSearchProviderBinder->getLocalSearchProviderHandler()->poiSearchResponse(JString(response).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_localNavigation_LocalSearchProvider_poiLookupResponse(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring response) {
    try {
        auto localSearchProviderBinder = LOCAL_SEARCH_PROVIDER_BINDER(ref);
        ThrowIfNull(localSearchProviderBinder, "nullLocalSearchProviderBinder");

        localSearchProviderBinder->getLocalSearchProviderHandler()->poiLookupResponse(JString(response).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}