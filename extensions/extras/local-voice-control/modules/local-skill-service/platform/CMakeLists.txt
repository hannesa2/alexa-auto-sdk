# AACE Local Skill Service Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/LocalSkillService/LocalSkillServiceConfiguration.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalSkillServicePlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalSkillService.cpp
)

target_include_directories(AACELocalSkillServicePlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACELocalSkillServicePlatform
    AACECorePlatform
)

install(
    TARGETS AACELocalSkillServicePlatform
    DESTINATION lib
    EXPORT AACELocalSkillService
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
