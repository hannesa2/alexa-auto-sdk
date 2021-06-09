/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AACE/LocalSkillService/LocalSkillServiceConfiguration.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.localSkillService.config.LocalSkillServiceConfigurationBinder";

// JNI
extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_localSkillService_config_LocalSkillServiceConfiguration_createLocalSkillServiceConfigBinder( 
        JNIEnv * env, jobject obj, jstring lssSocketPath, jstring lmbSocketPath )
    {
        try
        {
            auto config = aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig( 
                JString(lssSocketPath).toStdStr(), JString(lmbSocketPath).toStdStr() );
            ThrowIfNull( config, "createLocalSkillServiceConfigFailed");

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_localSkillService_config_LocalSkillServiceConfiguration_createLocalSkillServiceConfigBinder",ex.what());
            return 0;
        }
    }
}