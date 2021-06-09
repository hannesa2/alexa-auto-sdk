# AACE LocalNavigation Platform

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/LocalNavigation/LocalNavigationConfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/LocalNavigation/LocalSearchProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/LocalNavigation/LocalSearchProviderEngineInterface.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalNavigationPlatform SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalSearchProvider.cpp
)

target_include_directories(AACELocalNavigationPlatform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AACELocalNavigationPlatform
    PUBLIC
        AACECorePlatform
)

install(
    TARGETS AACELocalNavigationPlatform
    DESTINATION lib
    EXPORT AACELocalNavigation
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
