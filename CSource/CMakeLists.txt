cmake_minimum_required(VERSION 3.15)

project(ClaRaDelay C)

set(PROJECT_NAME "Delay-V1")

add_library(${PROJECT_NAME} STATIC "claradelay.c")

if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

# Detect target platform
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if(MSYS OR MINGW)
        if("$ENV{MSYSTEM}" STREQUAL "UCRT64")
            set(TARGET_SYSTEM_NAME "ucrt")
        elseif("$ENV{MSYSTEM}" STREQUAL "MINGW64" OR "$ENV{MSYSTEM}" STREQUAL "MINGW32")
            set(TARGET_SYSTEM_NAME "mingw")
        else()
            set(TARGET_SYSTEM_NAME "win")
        endif()
    else(MSYS OR MINGW)
        set(TARGET_SYSTEM_NAME "win")
    endif(MSYS OR MINGW)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(TARGET_SYSTEM_NAME "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(TARGET_SYSTEM_NAME "darwin")
else()
  message(FATAL_ERROR "Unknown target system: ${CMAKE_SYSTEM_NAME}")
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(TARGET_PLATFORM "${TARGET_SYSTEM_NAME}64")
else()
  set(TARGET_PLATFORM "${TARGET_SYSTEM_NAME}32")
endif()

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${PROJECT_SOURCE_DIR}/../ClaRaDelay/Resources/Library/${TARGET_PLATFORM})
