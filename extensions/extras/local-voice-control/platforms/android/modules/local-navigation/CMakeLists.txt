cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACELocalNavigationAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACELocalNavigation.cmake)

set(LOCALNAVIGATION_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(LOCALNAVIGATION_SOURCE
    ${LOCALNAVIGATION_CPP_DIR}/src/LocalNavigation/LocalNavigationConfigurationBinder.cpp
    ${LOCALNAVIGATION_CPP_DIR}/src/LocalNavigation/LocalSearchProviderBinder.cpp
)
set(LOCALNAVIGATION_DEPENDENCIES
    AACELocalNavigationEngine
    AACELocalNavigationPlatform
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACELocalNavigationAndroid SHARED
    ${LOCALNAVIGATION_SOURCE}
)

target_include_directories(AACELocalNavigationAndroid PUBLIC
    ${LOCALNAVIGATION_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACELocalNavigationAndroid
    ${LOCALNAVIGATION_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACELocalNavigationAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACELocalNavigationPlatform>
        $<TARGET_FILE:AACELocalNavigationEngine>
        ${AAC_HOME}/lib/libAlexaHybridPOIERService.so
        ${AAC_HOME}/lib/libAlexaHybridPOISearchLibrary.so
        ${AAC_HOME}/lib/libahe_external_er_client.so
        ${AAC_HOME}/lib/libahe_external_er_common.so
        ${AAC_HOME}/lib/libahe_external_er_server.so
        $<TARGET_FILE_DIR:AACELocalNavigationAndroid>
)
