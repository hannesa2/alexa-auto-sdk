# AACE LocalNavigation Engine

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalNavigation/LocalNavigationEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalNavigation/LocalSearchProviderEngineImpl.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalNavigationEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalNavigationConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalNavigationEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalSearchProviderEngineImpl.cpp
)

find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp
    CMAKE_FIND_ROOT_PATH_BOTH
)
find_path(LOCAL_SEARCH_INCLUDE_DIR localsearch)

target_include_directories(AACELocalNavigationEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${NLOHMANN_INCLUDE_DIR}
        ${LOCAL_SEARCH_INCLUDE_DIR}
)

find_library(POI_ER_SERVICE_LIB AlexaHybridPOIERService)
find_library(POI_SEARCH_LIB AlexaHybridPOISearchLibrary)

target_link_libraries(AACELocalNavigationEngine
    PUBLIC
        AACECoreEngine
        AACELocalNavigationPlatform
    PRIVATE
        AACELocalSkillServiceEngine
        ${POI_ER_SERVICE_LIB}
        ${POI_SEARCH_LIB}
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    TARGETS AACELocalNavigationEngine
    DESTINATION lib
    EXPORT AACELocalNavigation
)