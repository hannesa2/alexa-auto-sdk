# AACE LocalVoiceControl Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/LocalVoiceControl/LocalVoiceControlConfiguration.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalVoiceControlPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalVoiceControlService.cpp
)

target_include_directories(AACELocalVoiceControlPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACELocalVoiceControlPlatform
    AACECorePlatform
)

install(
    TARGETS AACELocalVoiceControlPlatform
    DESTINATION lib
    EXPORT AACELocalVoiceControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
