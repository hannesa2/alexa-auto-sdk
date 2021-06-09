# AACE Local Skill Service Engine

# Find AVSCommon headers & library
# Reason: LSS engine has direct dependency on AVSCommon
find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h)
add_definitions(-DRAPIDJSON_HAS_STDSTRING)

find_package(CURL REQUIRED)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalSkillService/HTTP.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalSkillServiceEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalSkillServiceConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalSkillServiceEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTP.cpp
)

target_include_directories(AACELocalSkillServiceEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${RAPIDJSON_INCLUDE_DIR}
        ${AVS_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIR}
)

target_link_libraries(AACELocalSkillServiceEngine
    PUBLIC
        AACELocalSkillServicePlatform
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${CURL_LIBRARY}
)

install(
    TARGETS AACELocalSkillServiceEngine
    DESTINATION lib
    EXPORT AACELocalSkillService
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
