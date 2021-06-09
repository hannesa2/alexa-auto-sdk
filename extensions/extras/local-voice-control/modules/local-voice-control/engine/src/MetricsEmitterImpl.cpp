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

#include "AACE/Engine/Metrics/MetricEvent.h"
#include "AACE/Engine/LocalVoiceControl/MetricsEmitterImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.MetricsEmitterImpl");

MetricsEmitterImpl::MetricsEmitterImpl() {
}

bool MetricsEmitterImpl::emit( const std::string &programName, const std::string &programSource,
                               const std::string &metricName, const int64_t metricValue,
                               const AlexaHybrid::SDK::IMetricsEmitter::Type type, AlexaHybrid::SDK::IMetricsEmitter::Priority priority ) const {
    std::unordered_map<std::string, std::string> metadata;
    return record(programName, programSource, metricName, metricValue, type, priority, metadata);
}

bool MetricsEmitterImpl::emit(const std::string &programName, const std::string &programSource,
                              const std::string &metricName, const int64_t metricValue, const AlexaHybrid::SDK::IMetricsEmitter::Type type,
              const std::unordered_map<std::string, std::string> &metadata, AlexaHybrid::SDK::IMetricsEmitter::Priority priority) const {
    return record(programName, programSource, metricName, metricValue, type, priority, metadata);
}

bool MetricsEmitterImpl::emitHandler(const rapidjson::Value& request) {
    try {
        if (request.IsArray()) {
            for (auto& itr : request.GetArray()) {
                ThrowIfNot(emitMetricHandler(itr), "emitMetricFailed");
            }
        }
        else if (request.IsObject()) {
            ThrowIfNot(emitMetricHandler(request), "emitMetricFailed");
        }
        else {
            return false;
        }
        return true;
    }
    catch (std::exception &ex) {
        AACE_ERROR(LX(TAG, "emitHandler").d("reason", ex.what()));
        return false;
    }
}

bool MetricsEmitterImpl::emitMetricHandler(const rapidjson::Value& metric) {
    try {
        ThrowIfNot(metric.HasMember("program") && metric["program"].IsString(), "No program");
        ThrowIfNot(metric.HasMember("source") && metric["source"].IsString(), "No source");
        std::string priorityString = "LOW";
        if (metric.HasMember("priority") && metric["priority"].IsString()) {
            priorityString = std::string(metric["priority"].GetString());
        }
        
        std::string program = std::string(metric["program"].GetString());
        std::string source = std::string(metric["source"].GetString());
        aace::engine::metrics::MetricEvent::MetricPriority priority = aace::engine::metrics::MetricEvent::MetricPriority::NR;
        if (priorityString == "LOW") {
            priority = aace::engine::metrics::MetricEvent::MetricPriority::NR;
        }
        else if (priorityString == "HIGH") {
            priority = aace::engine::metrics::MetricEvent::MetricPriority::HI;
        }

        // Create metric event
        aace::engine::metrics::MetricEvent currentMetric(program, source, priority);

        ThrowIfNot(metric.HasMember("dataPoints") && metric["dataPoints"].IsArray(), "No dataPoints");

        for (auto& dataPoint : metric["dataPoints"].GetArray()) {
            ThrowIfNot(dataPoint.HasMember("name") && dataPoint["name"].IsString(), "No name");
            ThrowIfNot(dataPoint.HasMember("value"), "No value");
            ThrowIfNot(dataPoint.HasMember("type") && dataPoint["type"].IsString(), "No type");
            std::string name = std::string(dataPoint["name"].GetString());
            std::string type = std::string(dataPoint["type"].GetString());
            auto& value = dataPoint["value"];
            if ((type == "COUNTER") && (value.IsUint())) {
                currentMetric.addCounter(name, value.GetUint());
            }
            else if ((type == "DISCRETE_VALUE") && (value.IsString())) {
                currentMetric.addString(name, std::string(value.GetString()));
            }
            else if ((type == "TIMER") && (value.IsUint64())) {
                currentMetric.addTimer(name, value.GetUint64());
            }
            else {
                continue;
            }
        }

        // Add meta data strings if available
        if (metric.HasMember("metadata") && metric["metadata"].IsObject()) {
            for (auto& property : metric["metadata"].GetObject()) {
                if (property.value.IsString()) {
                    currentMetric.addString(property.name.GetString(), property.value.GetString());
                }
            }
        }

        // Record metric event
        currentMetric.record();
        return true;
    }
    catch (std::exception &ex) {
        AACE_ERROR(LX(TAG, "emitMetricHandler").d("reason", ex.what()));
        return false;
    }
}

bool MetricsEmitterImpl::record(const std::string &programName, const std::string &programSource,
                                const std::string &metricName, const int64_t metricValue, const AlexaHybrid::SDK::IMetricsEmitter::Type type,
                                AlexaHybrid::SDK::IMetricsEmitter::Priority priority, const std::unordered_map<std::string, std::string> &metadata ) const {
    AACE_DEBUG(LX(TAG,"metric received").d("programName",   programName)
                                        .d("programSource", programSource)
                                        .d("metricName",    metricName)
                                        .d("metricValue",   metricValue));
    bool result = false;
    try {
        // Create metric event
        aace::engine::metrics::MetricEvent currentMetric(programName, programSource, getMetricPriority(priority));

        // Add data to metric event
        switch (type)
        {
        case AlexaHybrid::SDK::IMetricsEmitter::Type::COUNTER:
            currentMetric.addCounter(metricName, metricValue);
            break;
        case AlexaHybrid::SDK::IMetricsEmitter::Type::DISCRETE_VALUE:
            currentMetric.addString(metricName, std::to_string(metricValue));
            break;
        case AlexaHybrid::SDK::IMetricsEmitter::Type::TIMER:
            currentMetric.addTimer(metricName, metricValue);
            break;
        default:
            AACE_WARN(LX(TAG, "Unknown metric type"));
            return false;
        }

        std::string metadataStr;

        // Add meta data strings if available
        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
            metadataStr += it->first + ": " + it->second + ",";
            currentMetric.addString(it->first, it->second);
        }

        // Log metadata
        AACE_DEBUG(LX(TAG, "metric metadata").d("metadata", metadataStr));

        // Record metric event
        currentMetric.record();
        result = true;
    } catch (std::invalid_argument& e) {
        
        result = false;
    }

    return result;
}

aace::engine::metrics::MetricEvent::MetricPriority MetricsEmitterImpl::getMetricPriority(AlexaHybrid::SDK::IMetricsEmitter::Priority priority) const {
    switch (priority) {
        case AlexaHybrid::SDK::IMetricsEmitter::Priority::LOW:
            return aace::engine::metrics::MetricEvent::MetricPriority::NR;
        case AlexaHybrid::SDK::IMetricsEmitter::Priority::HIGH:
            return aace::engine::metrics::MetricEvent::MetricPriority::HI;
    }

    throw std::invalid_argument( "Unknown priority" );
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace