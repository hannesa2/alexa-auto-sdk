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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/LocalNavigation/LocalNavigationConfiguration.h>

/// String to identify log entries originating from this file.
static const char TAG[] = "LocalNavigationConfigurationBinder";

extern "C" {
JNIEXPORT jlong JNICALL
Java_com_amazon_aace_localNavigation_config_LocalNavigationConfiguration_createLocalSearchConfigBinder(
    JNIEnv* env,
    jobject obj,
    jstring navigationPOISocketPath,
    jstring poiEERSocketPath) {
    try {
        auto config = aace::localNavigation::config::LocalNavigationConfiguration::createLocalSearchConfig(
            JString(navigationPOISocketPath).toStdStr(), JString(poiEERSocketPath).toStdStr());
        ThrowIfNull(config, "createLocalSearchConfig failed");

        return reinterpret_cast<long>(new aace::jni::core::config::EngineConfigurationBinder(config));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return 0;
    }
}
}