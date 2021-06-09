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

#ifndef AACE_JNI_COASSISTANT_COASSISTANT_BINDER_H
#define AACE_JNI_COASSISTANT_COASSISTANT_BINDER_H

#include <AACE/CoAssistant/CoAssistant.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace coassistant {

    // CoAssistantHandler

class CoAssistantHandler : public aace::coassistant::CoAssistant {
    public:
        CoAssistantHandler( jobject obj );

        // aace::coassistant::CoAssistant
        void activeAgentChanged(const std::string& agent) override;

    private:
        JObject m_obj;
};

    // CoAssistantBinder

class CoAssistantBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    CoAssistantBinder (jobject obj );

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
            return m_coassistantHandler;
        }
    
    private:
        std::shared_ptr<CoAssistantHandler> m_coassistantHandler;
};

} // aace::jni::coassistant
} // aace::jni
} // aace

#endif // AACE_JNI_COASSISTANT_COASSISTANT_BINDER_H