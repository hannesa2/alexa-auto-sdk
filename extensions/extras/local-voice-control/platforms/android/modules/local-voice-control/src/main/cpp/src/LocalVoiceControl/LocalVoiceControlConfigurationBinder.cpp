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
#include <AACE/JNI/LocalVoiceControl/LocalVoiceControlConfigurationBinder.h>

// String to identify log entries orignating from this file.
static const char TAG[] = "aace.jni.localVoiceControl.config.LocalVoiceControlConfigurationBinder";

// type aliases
using SocketPermission = aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission;
// JNI
extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_localVoiceControl_config_LocalVoiceControlConfiguration_createFileConfigBinder( JNIEnv * env, jobject obj, jstring configFilePath )
    {
        try 
        {
            auto config = aace::localVoiceControl::config::LocalVoiceControlConfiguration::createFileConfig( JString(configFilePath).toStdStr() );
            ThrowIfNull( config, "createLocalVoiceControlConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_localVoiceControl_config_LocalVoiceControlConfiguration_createFileConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_localVoiceControl_config_LocalVoiceControlConfiguration_createIPCConfigBinder( JNIEnv * env, jobject obj, jstring controllerSocketDirectory, 
            jobject controllerSocketPermission, jstring platformSocketDirectory, jstring platformSocketPermission, jstring address, jstring messageRouterSocketDirectory )
    {
        try
        {
            SocketPermission controllerSocketPermObj;
            ThrowIfNot( aace::jni::localVoiceControl::JSocketPermission::checkType( controllerSocketPermission, &controllerSocketPermObj ), "invalidSocketPermission" );

            SocketPermission platformSocketPermObj;
            ThrowIfNot( aace::jni::localVoiceControl::JSocketPermission::checkType( platformSocketPermission, &platformSocketPermObj ), "invalidSocketPermission" );

            auto config = aace::localVoiceControl::config::LocalVoiceControlConfiguration::createIPCConfig( JString(controllerSocketDirectory).toStdStr(), controllerSocketPermObj,
                JString(platformSocketDirectory).toStdStr(), platformSocketPermObj, JString(address).toStdStr(), JString(messageRouterSocketDirectory).toStdStr() );

            ThrowIfNull( config, "createLocalVoiceControlConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) ); 
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_localVoiceControl_config_LocalVoiceControlConfiguration_createIPCConfigBinder",ex.what());
            return 0;
        }
    }
}
