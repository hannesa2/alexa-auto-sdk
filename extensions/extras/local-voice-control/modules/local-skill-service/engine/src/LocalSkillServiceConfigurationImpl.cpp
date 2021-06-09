/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "AACE/LocalSkillService/LocalSkillServiceConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace localSkillService {
namespace config {

std::shared_ptr<aace::core::config::EngineConfiguration> LocalSkillServiceConfiguration::createLocalSkillServiceConfig( const std::string& lssSocketPath, const std::string& lmbSocketPath ) {
    rapidjson::Document document(rapidjson::kObjectType);
    auto& allocator = document.GetAllocator();
    rapidjson::Value lssElement(rapidjson::kObjectType);
    
    lssElement.AddMember( "lssSocketPath", rapidjson::Value().SetString( lssSocketPath.c_str(), lssSocketPath.length() ), allocator );

    if ( !lmbSocketPath.empty() ) {
        lssElement.AddMember( "lmbSocketPath", rapidjson::Value().SetString( lmbSocketPath.c_str(), lmbSocketPath.length() ), allocator );
    }
    
    document.AddMember( "aace.localSkillService", lssElement, allocator );

    // create string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer( buffer );

    document.Accept( writer );
    
    return aace::core::config::StreamConfiguration::create( std::make_shared<std::stringstream>( buffer.GetString() ) );
}

} // aace::localSkillService::config
} // aace::localSkillService
} // aace
