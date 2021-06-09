# AACE CarControl Engine

find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)
find_library(sqlite3-lib sqlite3)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/CarControlLocalService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Endpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/JSONUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/CapabilityInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/PowerController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/PrimitiveController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/ModeController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/RangeController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/CarControlLocalService/Capabilities/ToggleController.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACECarControlLocalServiceEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CarControlLocalService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Endpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JSONUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/CapabilityInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/PowerController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/PrimitiveController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/ModeController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/RangeController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities/ToggleController.cpp
)

target_include_directories(AACECarControlLocalServiceEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
		${RAPIDJSON_INCLUDE_DIR}
        ${AVS_INCLUDE_DIRS}
)

target_link_libraries(AACECarControlLocalServiceEngine
    PUBLIC
		AACELocalSkillServiceEngine
		AACECarControlEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
        ${sqlite3-lib}
)

install(
    TARGETS AACECarControlLocalServiceEngine
    DESTINATION lib
    EXPORT AACECarControlLocalService
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()
