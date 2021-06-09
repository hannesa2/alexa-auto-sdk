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

#include <AACE/JNI/CoAssistant/CoAssistantBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.coassistant.CoAssistantBinder";

namespace aace {
namespace jni {
namespace coassistant {

    //
    // CoAssistantBinder
    //

    CoAssistantBinder::CoAssistantBinder( jobject obj ) {
        m_coassistantHandler = std::shared_ptr<CoAssistantHandler>( new CoAssistantHandler( obj ) );
    }

    //
    // CoAssistantHandler
    //

    CoAssistantHandler::CoAssistantHandler( jobject obj ) : m_obj( obj, "com/amazon/aace/coassistant/CoAssistant" ) {
    }

    void CoAssistantHandler::activeAgentChanged( const std::string& agent )
    {
        try_with_context
        {
            ThrowIfNot( m_obj.invoke<void>( "activeAgentChanged", "(Ljava/lang/String;)V", nullptr, JString(agent).get() ), "invokeFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"activeAgentChanged",ex.what());
        }
    }

} // aace::jni::coassistant
} // aace::jni
} // aace

#define COASSISTANT_BINDER(ref) reinterpret_cast<aace::jni::coassistant::CoAssistantBinder *>( ref )

extern "C"
{

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_coassistant_CoAssistant_createBinder( JNIEnv* env, jobject obj )  {
        return reinterpret_cast<long>( new aace::jni::coassistant::CoAssistantBinder( obj ) );
    }

    JNIEXPORT void JNICALL
    Java_com_amazon_aace_coassistant_CoAssistant_disposeBinder( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto coassistantBinder = COASSISTANT_BINDER(ref);
            ThrowIfNull( coassistantBinder, "invalidCoAssistantBinder" );
            delete coassistantBinder;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_coassistant_CoAssistant_disposeBinder",ex.what());
        }
    }
}

