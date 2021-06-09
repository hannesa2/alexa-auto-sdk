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

#ifndef AACE_ENGINE_COASSISTANT_COASSISTANT_OBSERVER_INTERFACE_H
#define AACE_ENGINE_COASSISTANT_COASSISTANT_OBSERVER_INTERFACE_H

namespace aace {
namespace engine {
namespace coassistant {

// Interface is required to delegate the active agent information from
// the capabilityagent.
class CoAssistantActiveAgentDelegateInterface
{
public:
    virtual ~CoAssistantActiveAgentDelegateInterface() = default;

    virtual void activeAgentChanged( const std::string& agent ) = 0;
};

} // aace::engine::navigation
} // aace::engine
} // aace

#endif // AACE_ENGINE_COASSISTANT_COASSISTANT_OBSERVER_INTERFACE_H
