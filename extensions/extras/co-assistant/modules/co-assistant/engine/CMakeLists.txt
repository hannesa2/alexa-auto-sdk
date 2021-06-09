# AACE CoAssistant Engine

find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_KWD_LIBRARY KWD)
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CoAssistant/CoAssistantActiveAgentDelegateInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CoAssistant/CoAssistantCustomDirectiveCapabilityAgent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CoAssistant/CoAssistantEngineImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CoAssistant/CoAssistantEngineService.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECoAssistantEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoAssistantCustomDirectiveCapabilityAgent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoAssistantEngineImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoAssistantEngineService.cpp
)

target_include_directories(AACECoAssistantEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACECoAssistantEngine
    PUBLIC
        AACEAlexaEngine
        AACEAlexaPlatform
        AACECoreEngine
        AACECoAssistantPlatform
    PRIVATE
        ${AVS_KWD_LIBRARY}
        ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACECoAssistantEngine
    DESTINATION lib
    EXPORT AACECoAssistant
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
