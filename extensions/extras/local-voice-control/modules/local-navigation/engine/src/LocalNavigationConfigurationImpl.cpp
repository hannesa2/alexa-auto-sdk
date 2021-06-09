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
#include <AACE/LocalNavigation/LocalNavigationConfiguration.h>

#include <istream>
#include <nlohmann/json.hpp>

namespace aace {
namespace localNavigation {
namespace config {

using json = nlohmann::json;

std::shared_ptr<aace::core::config::EngineConfiguration> LocalNavigationConfiguration::createLocalSearchConfig(
    const std::string& navigationPOISocketPath,
    const std::string& poiEERSocketPath) {
    // clang-format off
    json config = {
        {"aace.localNavigation", {
            {"localSearch", {
                {"navigationPOISocketPath", navigationPOISocketPath},
                {"poiEERSocketPath", poiEERSocketPath}
            }}
        }}
    };
    // clang-format on
    auto stream = std::make_shared<std::stringstream>();
    *stream << config;
    return aace::core::config::StreamConfiguration::create(stream);
}

}  // namespace config
}  // namespace localNavigation
}  // namespace aace
