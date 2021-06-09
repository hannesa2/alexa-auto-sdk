# AASB LOCAL NAVIGATION Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/LocalNavigation/AASBLocalNavigationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Engine/LocalNavigation/AASBLocalSearchProvider.h
)

source_group("Header Files" FILES ${HEADERS})

set(AASB_LOCAL_NAVIGATION_MESSAGES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/LocalNavigation/LocalSearchProvider/PoiLookupRequestMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/LocalNavigation/LocalSearchProvider/PoiLookupResponseMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/LocalNavigation/LocalSearchProvider/PoiSearchRequestMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB/Message/LocalNavigation/LocalSearchProvider/PoiSearchResponseMessage.h
)

source_group("LocalNavigation Message Headers" FILES ${AASB_LOCAL_NAVIGATION_MESSAGES})

add_library(AASBLocalNavigationEngine SHARED
    ${HEADERS}
    ${AASB_LOCAL_NAVIGATION_MESSAGES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBLocalNavigationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AASBLocalSearchProvider.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)

target_include_directories(AASBLocalNavigationEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
)

target_link_libraries(AASBLocalNavigationEngine
    PUBLIC
        AACEAASBEngine
        AACELocalNavigationPlatform
)

install(
    TARGETS AASBLocalNavigationEngine
    DESTINATION lib
    EXPORT AASBLocalNavigation
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AASB
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
