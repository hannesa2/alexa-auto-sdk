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

#ifndef AACE_LOCAL_VOICE_CONTROL_CLOCKIMPL_H
#define AACE_LOCAL_VOICE_CONTROL_CLOCKIMPL_H

#include "alexa_hybrid/sdk/IClock.h"

namespace aace {
namespace engine {
namespace localVoiceControl {

class ClockImpl : 
    public AlexaHybrid::SDK::IClock {

public:
    ClockImpl();
    ~ClockImpl(){};
    /**
     * Get the current local time that the physical device is in.
     * This function will only be invoked if getCapability() returns TimeZoneCapability::NONE
     * 
     * @param[out] timeOutput       Output parameter which this function will write the time to.
     *                              The output time will be in RFC 3339 format, but the offset
     *                              must be -00:00, as specified by RFC3339 section 4.3.
     *                              This parameter is only valid when the function 
     *                              returns TimeResponseStatus::SUCCESS.
     *                              Example:
     *                              1996-12-19T16:39:57-00:00
     * 
     * @return                      The @c TimeResponseStatus indicating if this function 
     *                              succeeded or failed and why.
     */
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> getLocalTime(std::string &timeOutput) override;

    /**
     * Get the current time, shifted by an offset.
     * This function will only be invoked if getCapability() returns 
     * TimeZoneCapability::OFFSET_SUPPORTED or TimeZoneCapability::OLSON_TIME_ZONE_AND_OFFSET_SUPPORTED
     * 
     * @param[in]  timeOffset       Offset to shift the reported time by.  Formatted in the manner
     *                              specified by the time-offset field in RFC 3339.
     * 
     * @param[out] timeOutput       Output parameter which this function will write the time to.
     *                              The output time will be in RFC 3339 format in the offset
     *                              specified by @c timeOffset.                      
     *                              This parameter is only valid when the function 
     *                              returns TimeResponseStatus::SUCCESS.
     *                              Examples:
     *                              1996-12-19T16:39:57-08:00
     *                              This represents 39 minutes and 57 seconds after the 16th hour of
     *                              December 19th, 1996 with an offset of -08:00 from UTC (Pacific
     *                              Standard Time)
     *                              1996-12-20T00:39:57Z
     *                              This represents the same as above, but in UTC
     * 
     * @return                      The @c TimeResponseStatus indicating if this function 
     *                              succeeded or failed and why.
     */
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> getTimeWithOffset(const std::string &timeOffset, std::string &timeOutput) override;

    /**
     * Get the current time, in the requested time zone.
     * This function will only be invoked if getCapability() returns TimeZoneCapability::OLSON_TIME_ZONE_AND_OFFSET_SUPPORTED
     * 
     * @param[in]  olsenTimeZone    Time zone to report the time in.  Time zone is in olsen format
     * 
     * @param[out] timeOutput       Output parameter which this function will write the time to.
     *                              The output time will be in RFC 3339 format and it will be 
     *                              in the requested time zone.
     *                              This parameter is only valid when the function 
     *                              returns TimeResponseStatus::SUCCESS.
     *                              Examples:
     *                              1996-12-19T16:39:57-08:00
     *                              This represents 39 minutes and 57 seconds after the 16th hour of
     *                              December 19th, 1996 with an offset of -08:00 from UTC (Pacific
     *                              Standard Time)
     *                              1996-12-20T00:39:57Z
     *                              This represents the same as above, but in UTC
     * 
     * @return                      The @c TimeResponseStatus indicating if this function 
     *                              succeeded or failed and why.
     */
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> getTimeInTimeZone(const std::string &olsenTimeZone, std::string &timeOutput) override;

    /**
     * Get the ClockCapability of the device's clock.
     *
     * @return                      what capabilities the device supports
     */
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::IClock::ClockCapability> getCapability() override;
    
private:
    /**
     * Get the time that the physical device is in. This is a private helper function for the public getTime methods.
     * 
     * @param[out] timeOutput       Output parameter which this function will write the time to.
     *                              The output time will be in RFC 3339 format, but the offset
     *                              must be -00:00, as specified by RFC3339 section 4.3.
     *                              This parameter is only valid when the function 
     *                              returns TimeResponseStatus::SUCCESS.
     *                              Example:
     *                              1996-12-19T16:39:57-00:00
     * 
     * @return                      The @c TimeResponseStatus indicating if this function 
     *                              succeeded or failed and why.
     */
    AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> getTime(std::string &timeOutput, const std::string &timeOffset);

    /**
     * Add a colon in the center of the standard offset formatting from -hhmm to -hh:mm
     * 
     * @param[out] offset           Output parameter which this function will write to.
     *                              
     *                              Example:
     *                              -10:00
     */
    void addColonToOffset(std::string &offset);
};

} // aace::engine::localVoiceControl
} // aace::engine
} // aace

#endif // AACE_LOCAL_VOICE_CONTROL_CLOCKIMPL_H