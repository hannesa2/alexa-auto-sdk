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

#ifndef AACE_ENGINE_AMAZONLITE_AMAZONLITE_ENGINE_SERVICE_H
#define AACE_ENGINE_AMAZONLITE_AMAZONLITE_ENGINE_SERVICE_H

#include <AVSCommon/Utils/Threading/Executor.h>

#include "AACE/Alexa/AlexaProperties.h"

#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyListenerInterface.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineService.h"
#include "AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h"

#include "AmazonLiteWakewordEngineAdapter.h"

namespace aace {
namespace engine {
namespace amazonLite {

class AmazonLiteEngineService
        : public aace::engine::core::EngineService
        , public aace::engine::propertyManager::PropertyListenerInterface
        , public std::enable_shared_from_this<AmazonLiteEngineService> {
public:
    DESCRIBE(
        "aace.amazonLite",
        VERSION("1.0"),
        DEPENDS(aace::engine::alexa::AlexaEngineService),
        DEPENDS(aace::engine::propertyManager::PropertyManagerEngineService))

private:
    AmazonLiteEngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~AmazonLiteEngineService() = default;

    // PropertyListenerInterface
    virtual void propertyChanged(const std::string& key, const std::string& newValue) override;

protected:
    bool initialize() override;
    bool setup() override;
    bool start() override;
    bool stop() override;

    /**
     * @code {.json}
     * {
     *      "aace.amazonLite" : {
     *          "rootPath" : "Top level file system path where the model files (and other assets) are stored"
     *          "models" : [
     *              {
     *                  "locale" : "locale",
     *                  "path" : "relative path to the locale Model file"
     *              }
     *          ]
     *      }
     * }
     * @endcode
     */
    bool configure( std::shared_ptr<std::istream> configuration ) override;

private:
    void executeUpdateLocale(const std::string& value);

private:
    /// Adapter object reference
    std::shared_ptr<AmazonLiteWakewordEngineAdapter> m_primaryAdapter;
    std::shared_ptr<AmazonLiteWakewordEngineAdapter> m_secondaryAdapter;

    /// configuration elements provided to the EngineService
    std::string m_rootPath;
    std::map<std::string,std::string> m_modelLocaleMap;

    // executor
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

} // aace::engine::amazonLite
} // aace::engine
} // aace

#endif // AACE_ENGINE_AMAZONLITE_AMAZONLITE_ENGINE_SERVICE_H