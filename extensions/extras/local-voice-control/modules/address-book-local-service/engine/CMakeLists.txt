# AACE AACEAddressBookLocalService Engine

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/rapidjson.h
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_path(AVS_INCLUDE_DIRS AVSCommon)
find_library(AVS_AVS_COMMON_LIBRARY AVSCommon)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBookLocalService/AddressBookLocalService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/AddressBookLocalService/AddressBookLocalImpl.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACEAddressBookLocalServiceEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookLocalService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AddressBookLocalImpl.cpp

)

target_include_directories(AACEAddressBookLocalServiceEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${RAPIDJSON_INCLUDE_DIR}
)

target_link_libraries(AACEAddressBookLocalServiceEngine
    PUBLIC
        AACELocalSkillServiceEngine
        AACEAddressBookEngine
        AACECoreEngine
    PRIVATE
        ${AVS_AVS_COMMON_LIBRARY}
)

install(
    TARGETS AACEAddressBookLocalServiceEngine
    DESTINATION lib
    EXPORT AACEAddressBookLocalService
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

if(AAC_ENABLE_TESTS)
    add_subdirectory(test)
endif()