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

#ifndef AACE_JNI_LOCAL_VOICE_CONTROL_LOCAL_VOICE_CONTROL_CONFIGURATION_BINDER_H
#define AACE_JNI_LOCAL_VOICE_CONTROL_LOCAL_VOICE_CONTROL_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/LocalVoiceControl/LocalVoiceControlConfiguration.h>

namespace aace {
namespace jni {
namespace localVoiceControl {

    // JSocketPermission

    class JSocketPermissionConfig : public EnumConfiguration<aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission> {
    public:
        using T = aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission;

        const char* getClassName() override {
            return "com/amazon/aace/localVoiceControl/config/LocalVoiceControlConfiguration$SocketPermission";
        }

        std::vector<std::pair<T,std::string>> getConfiguration() override {
            return {
                {T::OWNER,"OWNER"},
                {T::GROUP,"GROUP"},
                {T::ALL,"ALL"}
            };
        }
    };

    using JSocketPermission = JEnum<aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission,JSocketPermissionConfig>;

} // aace::jni::localVoiceControl
} // aace::jni
} // aace

#endif // AACE_JNI_LOCAL_VOICE_CONTROL_LOCAL_VOICE_CONTROL_CONFIGURATION_BINDER_H