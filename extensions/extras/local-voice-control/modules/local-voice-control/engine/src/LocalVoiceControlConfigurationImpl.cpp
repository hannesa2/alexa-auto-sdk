

/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/LocalVoiceControl/LocalVoiceControlConfiguration.h"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/pointer.h>

namespace aace {
namespace localVoiceControl {
namespace config {

std::shared_ptr<aace::core::config::EngineConfiguration> LocalVoiceControlConfiguration::createFileConfig(const std::string& configFilePath) {
    // Create new document
    auto doc = rapidjson::Document();

    // Create top level Json object
    rapidjson::Pointer("/aace.localVoiceControl").Create(doc);

    // Set object properties
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/configFilePath", configFilePath );

    // Convert document to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // return Stream configuration object
    return aace::core::config::StreamConfiguration::create( std::make_shared<std::stringstream>( buffer.GetString() )); 
}

std::shared_ptr<aace::core::config::EngineConfiguration> LocalVoiceControlConfiguration::createIPCConfig(
    const std::string& controllerSocketDirectory, SocketPermission controllerSocketPermissions, 
    const std::string& platformSocketDirectory,   SocketPermission platformSocketPermissions,
    const std::string& address, const std::string& messageRouterSocketDirectory ) {

    // Create new document
    auto doc = rapidjson::Document();

    // Create top level Json object
    rapidjson::Pointer("/aace.localVoiceControl").Create(doc);

    // Set object properties
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/controllerSocketDirectory",    controllerSocketDirectory                       );
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/controllerSocketPermissions",  permissionToString(controllerSocketPermissions) );
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/platformSocketDirectory",      platformSocketDirectory                         );
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/platformSocketPermissions",    permissionToString(platformSocketPermissions)   );
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/address",                      address                                         );
    rapidjson::SetValueByPointer(doc, "/aace.localVoiceControl/messageRouterSocketDirectory", messageRouterSocketDirectory                    );

    // Convert document to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // return Stream configuration object
    return aace::core::config::StreamConfiguration::create( std::make_shared<std::stringstream>( buffer.GetString() ));
}

std::string LocalVoiceControlConfiguration::permissionToString(SocketPermission permission) {
    switch(permission) {
        case SocketPermission::OWNER:
            return "OWNER";
        case SocketPermission::GROUP:
            return "GROUP";
        case SocketPermission::ALL:
            return "ALL";
        default:
            throw std::runtime_error("Invalid SocketPermission value");
    }
}

} // aace::localVoiceControl::config
} // aace::localVoiceControl
} // aace