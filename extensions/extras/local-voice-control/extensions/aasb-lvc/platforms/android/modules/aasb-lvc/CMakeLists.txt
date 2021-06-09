cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AASBLocalNavigationAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### AASB LOCAL NAVIGATION MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACELocalNavigation.cmake)
include(${AAC_HOME}/share/cmake/AACELocalSkillService.cmake)
include(${AAC_HOME}/share/cmake/AACEAASB.cmake)
include(${AAC_HOME}/share/cmake/AASBCore.cmake)
include(${AAC_HOME}/share/cmake/AASBLocalNavigation.cmake)

set(AASB_LOCAL_NAVIGATION_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(AASB_LOCAL_NAVIGATION_SOURCE
    ${AASB_LOCAL_NAVIGATION_CPP_DIR}/dummy.cpp
)

set(AASB_LOCAL_NAVIGATION_DEPENDENCIES
    AASBLocalNavigationEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AASBLocalNavigationAndroid SHARED
    ${AASB_LOCAL_NAVIGATION_SOURCE}
)

target_include_directories(AASBLocalNavigationAndroid PUBLIC
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AASBLocalNavigationAndroid
    ${AASB_LOCAL_NAVIGATION_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AASBLocalNavigationAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AASBLocalNavigationEngine>
        $<TARGET_FILE_DIR:AASBLocalNavigationAndroid>
)
