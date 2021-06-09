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

#include "AACE/Engine/LocalVoiceControl/ClockImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <chrono>
#include <sstream>
#include <ctime>
#include <iostream>
#include <time.h>

namespace aace {
namespace engine {
namespace localVoiceControl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.localVoiceControl.ClockImpl");

// Size of buffer to hold date and time values.
static const int DATE_AND_TIME_STRING_SIZE = 80;

// Size of buffer to hold time offset
static const int TIME_OFFSET_SIZE = 6;

// Format string for strftime() to produce date and time in the format "YYYY-MM-DDThh:mm:ss".
static const char* STRFTIME_FORMAT_STRING = "%Y-%m-%dT%T";

// Format string for strftime() to produce time offset in the format "+-hh:mm".
static const char* STRFTIME_FORMAT_OFFSET = "%z";

// Overwrite flag for setenv()
static const int OVERWRITE = 1;

// Number of times to insert char
static const int INSERT_COUNT = 1;

// Index to insert char into
static const int INSERT_INDEX = 3;

ClockImpl::ClockImpl() {
}

AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> ClockImpl::getLocalTime( std::string &timeOutput ) {
    AACE_INFO(LX(TAG, "Getting local time"));
    return getTime(timeOutput, "");
}

AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> ClockImpl::getTimeWithOffset( const std::string &timeOffset, std::string &timeOutput ) {
    AACE_INFO(LX(TAG, "Getting time with offset ").d("timeOffset", timeOffset));
    return getTime(timeOutput, timeOffset);
}

AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> ClockImpl::getTimeInTimeZone( const std::string &olsenTimeZone, std::string &timeOutput ) {
    AACE_INFO(LX(TAG, "Getting time for timezone specified ").d("timezone", olsenTimeZone));
    
    // Setting timezone to olsenTimeZone parameter
    setenv("TZ", olsenTimeZone.c_str(), OVERWRITE);
    tzset();
    auto result = getTime(timeOutput, "");

    // Unset timezone after time has been calculated
    unsetenv("TZ");
    return result;
}

AlexaHybrid::SDK::Result<AlexaHybrid::SDK::IClock::ClockCapability> ClockImpl::getCapability() {
    AACE_INFO(LX(TAG, "Getting capability"));
    AlexaHybrid::SDK::IClock::ClockCapability clockCapability;
    clockCapability.timeZoneCapability = AlexaHybrid::SDK::TimeZoneCapability::OLSON_TZ_AND_OFFSET;
    return AlexaHybrid::SDK::Result<AlexaHybrid::SDK::IClock::ClockCapability>::create(clockCapability);
}

AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum> ClockImpl::getTime(std::string &timeOutput, const std::string &timeOffsetInput) {
    try {
        // Get current time from device and convert
        auto nowTimePoint = std::chrono::system_clock::now();
        auto rawTimeAsTime_t = std::chrono::system_clock::to_time_t( nowTimePoint );
        struct tm* timeAsTmPtr;

        // If time offset is not given then use localtime, otherwise use GM time
        if(timeOffsetInput == "") {
            timeAsTmPtr = std::localtime( &rawTimeAsTime_t );
        } else {
            timeAsTmPtr = std::gmtime( &rawTimeAsTime_t );
        }

        // Create char buffers to store formatted time output
        char dateTimeChars[DATE_AND_TIME_STRING_SIZE];
        char timeOffsetChars[TIME_OFFSET_SIZE];

        // Format the tm to string and check that formatting returned valid values 
        if (!timeAsTmPtr || 0 == strftime(dateTimeChars, sizeof(dateTimeChars), STRFTIME_FORMAT_STRING, timeAsTmPtr) || 0 == strftime(timeOffsetChars, sizeof(timeOffsetChars), STRFTIME_FORMAT_OFFSET, timeAsTmPtr)) {
            return AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum>::create(AlexaHybrid::SDK::TimeResponseStatus::INTERNAL_FAILURE);
        }

        // If time offset is not given, then add colon to offset string, otherwise set offset string = time offset
        std::string timeOffset;
        if(timeOffsetInput == "") {
            timeOffset = timeOffsetChars;
            addColonToOffset(timeOffset);
        } else {
            timeOffset = timeOffsetInput;
        }

        // Set output and append offset string to the output
        timeOutput = dateTimeChars;
        timeOutput.append(timeOffset);
        AACE_INFO(LX(TAG, "Time is ").d("time", timeOutput));
        return AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum>::create(AlexaHybrid::SDK::TimeResponseStatus::SUCCESS);
    } 
    catch (std::exception &ex) {
        AACE_ERROR(LX(TAG, "getTime").d("reason", ex.what()));
    }

    return AlexaHybrid::SDK::Result<AlexaHybrid::SDK::TimeResponseStatus::Enum>::create(AlexaHybrid::SDK::TimeResponseStatus::INTERNAL_FAILURE);
}

void ClockImpl::addColonToOffset( std::string &offset ) {
    // Insert 1 ":" at index 3
    offset.insert(INSERT_INDEX, INSERT_COUNT, ':');
}

} // aace::engine::localVoiceControl
} // aace::engine
} // aace