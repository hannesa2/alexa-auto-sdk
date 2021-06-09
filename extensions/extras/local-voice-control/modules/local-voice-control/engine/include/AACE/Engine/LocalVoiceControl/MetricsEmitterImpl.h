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

#ifndef AACE_LOCAL_VOICE_CONTROL_METRICS_EMITTER_IMPL_H
#define AACE_LOCAL_VOICE_CONTROL_METRICS_EMITTER_IMPL_H

#include <memory>
#include <rapidjson/document.h>
#include "AACE/Engine/Metrics/MetricEvent.h"
#include "alexa_hybrid/sdk/IMetricsEmitter.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class MetricsEmitterImpl :
    public AlexaHybrid::SDK::IMetricsEmitter,
    public std::enable_shared_from_this<MetricsEmitterImpl> {

public:

    /**
     * Default constructor.
     */
    MetricsEmitterImpl();

    /**
     * Emit a metric.
     *
     * @param programName Name of the program emitting the metric.
     * @param programSource Source of the program emitting the metric.
     * @param metricName Metric name.
     * @param metricValue Metric value.
     * @param type Type of metric being emitted.
     * @return Whether the metric was recorded successfully.
     */
    bool emit(const std::string &programName, const std::string &programSource,
              const std::string &metricName, const int64_t metricValue,
              const AlexaHybrid::SDK::IMetricsEmitter::Type type, AlexaHybrid::SDK::IMetricsEmitter::Priority priority) const override;

    /**
     * Emit a metric with metadata.
     *
     * @param programName Name of the program emitting the metric.
     * @param programSource Source of the program emitting the metric.
     * @param metricName Metric name.
     * @param metricValue Metric value.
     * @param type Type of metric being emitted.
     * @param metadata Metric metadata.
     * @return Whether the metric was recorded successfully.
     */
    bool emit(const std::string &programName, const std::string &programSource,
              const std::string &metricName, const int64_t metricValue, const AlexaHybrid::SDK::IMetricsEmitter::Type type,
              const std::unordered_map<std::string, std::string> &metadata, AlexaHybrid::SDK::IMetricsEmitter::Priority priority) const override;


    /**
     * Emit a metric from JSON document.
     *
     * @param request JSON payload. Format described hereunder.
     * @return Whether the metric was recorded successfully.
     * 
     * Payload format contains an array of metrics:
     * 
     * [
     *     {
     *         "program": "AlexaAuto_Speech_Gamma",
     *         "source": "VoiceRequest",
     *         "priority": "LOW|HIGH",                          // default is "LOW"
     *         "dataPoints": [
     *              {  
     *                  "name":"PlaybackStartedTimestamp",
     *                  "value":1550001498647,
     *                  "type":"TIMER"
     *              },
     *              {  
     *                  "name":"StopCaptureTimestamp",
     *                  "value":1550001497801,
     *                  "type":"TIMER"
     *              },
     *              {  
     *                  "name":"DialogRequestId",
     *                  "value":"4ac66229-7f24-4654-9ee9-0a0cdceba343",
     *                  "type":"DISCRETE_VALUE"
     *              }
     *          ],
     *         "metadata": {                                    // Optional
     *             "key1": "value1",
     *             "key2": "value2"
     *         }
     *     }
     * ]
     */
    bool emitHandler(const rapidjson::Value& request);
    bool emitMetricHandler(const rapidjson::Value& metric);

private:
    /**
     * Convert priority value.
     */
    aace::engine::metrics::MetricEvent::MetricPriority getMetricPriority(AlexaHybrid::SDK::IMetricsEmitter::Priority priority) const;

    /**
     * Record metrics. 
     */
    bool record(const std::string &programName, const std::string &programSource,
                const std::string &metricName, const int64_t metricValue, const AlexaHybrid::SDK::IMetricsEmitter::Type type,
                AlexaHybrid::SDK::IMetricsEmitter::Priority priority, const std::unordered_map<std::string, std::string> &metadata ) const;
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_METRICS_EMITTER_IMPL_H