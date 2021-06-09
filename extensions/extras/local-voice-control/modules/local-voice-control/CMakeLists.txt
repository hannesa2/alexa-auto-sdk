# AACE Local Voice Control 

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(AACELocalVoiceControl VERSION 0.99.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("No build type specified, defaulting to DEBUG.")
endif()

# Select flags.
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Wall -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -DAACE_DEBUG_LOG_ENABLED -Wall -g")

if (AAC_EMIT_SENSITIVE_LOGS)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
    if (BUILD_TYPE_UPPER STREQUAL DEBUG)
        message("WARNING: Logging of sensitive information enabled!")
        add_definitions(-DAAC_EMIT_SENSITIVE_LOGS)
    else()
        message(FATAL_ERROR "FATAL_ERROR: AAC_EMIT_SENSITIVE_LOGS=ON in non-DEBUG build.")
    endif()
endif()

# LocalVoiceControl module depends on Core, Alexa, Local Skill Service modules
if(AAC_HOME)
    include(${AAC_HOME}/share/cmake/AACECore.cmake)
    include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
    include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
    if(COASSISTANT)
        include(${AAC_HOME}/share/cmake/AACECoAssistant.cmake)
        add_definitions(-DCOASSISTANT)
    endif()
endif()

add_subdirectory(platform)
add_subdirectory(engine)

install(
  EXPORT AACELocalVoiceControl
    DESTINATION share/cmake
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/config-local-voice-control.json
    DESTINATION etc
)
