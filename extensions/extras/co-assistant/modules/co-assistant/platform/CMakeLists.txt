# AACE CoAssistant Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/CoAssistant/CoAssistant.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECoAssistantPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoAssistant.cpp
)

target_include_directories(AACECoAssistantPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACECoAssistantPlatform
    AACECorePlatform
)

install(
    TARGETS AACECoAssistantPlatform
    DESTINATION lib
    EXPORT AACECoAssistant
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
