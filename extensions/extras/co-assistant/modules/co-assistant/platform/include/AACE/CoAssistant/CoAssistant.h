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

#ifndef AACE_COASSISTANT_COASSISTANT_H
#define AACE_COASSISTANT_COASSISTANT_H

#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace coassistant {

class CoAssistant : public aace::core::PlatformInterface {

protected:
    CoAssistant() = default;

public:
    virtual ~CoAssistant();

    /**
    * Method that acts as an observer API when the active agent is changed.
    * @param [in] agent The agent to be used as the new active agent.
    */
    virtual void activeAgentChanged(const std::string& agent) = 0;

};
} // aace::coassistant
} // aace

#endif // AACE_COASSISTANT_COASSISTANT_H