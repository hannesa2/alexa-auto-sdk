# AACE LocalVoiceControl Engine

# Find AVS Includes
# Reason: AACEAlexaEngine expose AVS headers via public API

find_path(AVS_INCLUDE_DIR ACL AVSCommon)
find_path(AHE_INCLUDE_DIR alexa_hybrid)
find_library(AVS_COMMON_LIBRARY AVSCommon)
find_library(AVS_ACL_LIBRARY ACL)
find_library(AVS_ENDPOINTS_LIBRARY Endpoints)
find_library(AHE_PLATFORM_INTERFACES_LIB ahe_platform_interfaces_ipc)
find_library(HYBRID_PROXY_LIB HybridProxy)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(AVS_INCLUDE_DIRS ${AVS_INCLUDE_DIR})

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/LocalVoiceControlEngineService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/AuthImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/ClockImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/PlatformServicesRunner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/ConfigurationImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/ConnectivityImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/DummyAuthDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/MetricsEmitterImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/LocaleImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE/Engine/LocalVoiceControl/EndpointImpl.h
)

source_group("Header Files" FILES ${HEADERS})

add_library(AACELocalVoiceControlEngine SHARED
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalVoiceControlEngineService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ClockImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlatformServicesRunner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConnectivityImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DummyAuthDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocalVoiceControlConfigurationImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MetricsEmitterImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LocaleImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EndpointImpl.cpp
)

target_include_directories(AACELocalVoiceControlEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${AVS_INCLUDE_DIRS}
        ${AVS_INCLUDE_DIRS}/../avs_shared
        ${AHE_INCLUDE_DIR}
)

target_link_libraries(AACELocalVoiceControlEngine
#    PUBLIC
#       AACELocalVoiceControlPlatform
    PRIVATE
        AACELocalSkillServiceEngine
        AACEAlexaEngine
        AACECoreEngine
        AACELocalVoiceControlPlatform
        ${AVS_ACL_LIBRARY}
        ${AVS_COMMON_LIBRARY}
        ${AVS_ENDPOINTS_LIBRARY}
        ${AHE_PLATFORM_INTERFACES_LIB}
        ${HYBRID_PROXY_LIB}
)

install(
    TARGETS AACELocalVoiceControlEngine
    DESTINATION lib
    EXPORT AACELocalVoiceControl
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/AACE
    DESTINATION include
)
