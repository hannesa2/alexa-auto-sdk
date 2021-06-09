/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_INTERFACE_H
#define AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_INTERFACE_H

#include <rapidjson/document.h>
#include <string>

namespace aace {
namespace engine {
namespace localSkillService {

typedef std::function<bool ( std::shared_ptr<rapidjson::Document> request, std::shared_ptr<rapidjson::Document> response )> RequestHandler;
typedef std::function<bool ( std::shared_ptr<rapidjson::Document> request )> PublishRequestHandler;
typedef std::function<bool ( std::shared_ptr<rapidjson::Document> response )> PublishResponseHandler;

class LocalSkillServiceInterface {
public:
    // Destructor
    virtual ~LocalSkillServiceInterface() = default;

    /// Provide registration with specific request path and Request Handler
    virtual void registerHandler(const std::string& path, RequestHandler handler) = 0;

    /// Provide registration with Subscribe Handler, Request Handler and Response Handler
    virtual bool registerPublishHandler(const std::string& id, RequestHandler subscribeHandler = nullptr, PublishRequestHandler requestHandler = nullptr, PublishResponseHandler responseHandler = nullptr) = 0;

    /// Publish message with specific path registered with handler and payload
    virtual bool publishMessage(const std::string& id, std::shared_ptr<rapidjson::Document> message = nullptr) = 0;

    /// Getter for LSS Socket Path instance
    virtual std::string getLSSSocketPath() = 0;
};

}  // namespace localSkillService
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOCAL_SKILL_SERVICE_LOCAL_SKILL_SERVICE_INTERFACE_H
