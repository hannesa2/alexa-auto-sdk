cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

project(AACELocalVoiceControlAndroid LANGUAGES CXX)

set(AAC_HOME ${ANDROID_PLATFORM_DIR}/native-build/${ANDROID_ABI}-${ANDROID_PLATFORM}/opt/AAC)
set(CORE_MODULE_DIR ${ANDROID_PLATFORM_DIR}/modules/core)
string(TOLOWER ${CMAKE_BUILD_TYPE} NDK_BUILD_TYPE)

if(NOT EXISTS ${AAC_HOME})
    message(FATAL_ERROR "AAC build for ${ANDROID_ABI}/${ANDROID_PLATFORM} ${AAC_HOME} does not exist!")
endif()

### LVC MODULE ###
include(${AAC_HOME}/share/cmake/AACECore.cmake)
include(${AAC_HOME}/share/cmake/AACEAlexa.cmake)
include(${AAC_HOME}/share/cmake/AACELocalVoiceControl.cmake)

set(LOCALVOICECONTROL_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
set(LOCALVOICECONTROL_SOURCE
    ${LOCALVOICECONTROL_CPP_DIR}/src/LocalVoiceControl/LocalVoiceControlConfigurationBinder.cpp
)
set(LOCALVOICECONTROL_DEPENDENCIES
    AACELocalVoiceControlEngine
    ${CORE_MODULE_DIR}/build/intermediates/cmake/${NDK_BUILD_TYPE}/obj/${ANDROID_ABI}/libAACECoreAndroid.so
)

add_library(AACELocalVoiceControlAndroid SHARED
    ${LOCALVOICECONTROL_SOURCE}
)

target_include_directories(AACELocalVoiceControlAndroid PUBLIC
    ${LOCALVOICECONTROL_CPP_DIR}/include
    ${CORE_MODULE_DIR}/src/main/cpp/include
)

target_link_libraries(AACELocalVoiceControlAndroid
    ${LOCALVOICECONTROL_DEPENDENCIES}
    android
    log
)

# Copy all AAC dependencies
add_custom_command(
    TARGET AACELocalVoiceControlAndroid
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:AACELocalVoiceControlPlatform>
        $<TARGET_FILE:AACELocalVoiceControlEngine>
        ${AAC_HOME}/lib/libHybridProxy.so
        ${AAC_HOME}/lib/libahe_platform_interfaces_ipc.so
        ${AAC_HOME}/lib/libboost_chrono.so
        ${AAC_HOME}/lib/libboost_filesystem.so
        ${AAC_HOME}/lib/libboost_system.so
        ${AAC_HOME}/lib/libnanomsg.so
        ${AAC_HOME}/lib/libahe_logger.so
        ${AAC_HOME}/lib/libboost_atomic.so
        ${AAC_HOME}/lib/libboost_date_time.so
        ${AAC_HOME}/lib/libboost_program_options.so
        ${AAC_HOME}/lib/libboost_thread.so
        $<TARGET_FILE_DIR:AACELocalVoiceControlAndroid>
)
