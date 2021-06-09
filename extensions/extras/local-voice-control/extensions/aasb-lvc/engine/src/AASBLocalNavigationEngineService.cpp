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

#include <AASB/Engine/LocalNavigation/AASBLocalNavigationEngineService.h>
#include <AASB/Engine/LocalNavigation/AASBLocalSearchProvider.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aasb {
namespace engine {
namespace localNavigation {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.localNavigation.AASBLocalNavigationEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("3.1");

// register the service
REGISTER_SERVICE(AASBLocalNavigationEngineService);

AASBLocalNavigationEngineService::AASBLocalNavigationEngineService( const aace::engine::core::ServiceDescription& description ) :
        aace::engine::aasb::AASBHandlerEngineService( description, minRequiredVersion, {
                "LocalSearchProvider"
        }) {
}

bool AASBLocalNavigationEngineService::postRegister()
{
    try
    {
        auto aasbServiceInterface = getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>( "aace.aasb" );
        ThrowIfNull( aasbServiceInterface, "invalidAASBServiceInterface" );

        // LocalNavigation
        if( isInterfaceEnabled( "LocalSearchProvider" ) ) {
            auto localSearchProvider = AASBLocalSearchProvider::create( aasbServiceInterface->getMessageBroker() );
            ThrowIfNull( localSearchProvider, "invalidLocalSearchProviderHandler" );
            getContext()->registerPlatformInterface( localSearchProvider );
        }

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}


} // aasb::engine::localNavigation
} // aasb::engine
} // aasb
