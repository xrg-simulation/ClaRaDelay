cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(ClaRaDelay C)

set(PROJECT_NAME "Delay-V1")

add_library(${PROJECT_NAME} STATIC "claradelay.c")

set_property(TARGET ${PROJECT_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    if (UNIX)
        install(TARGETS ${PROJECT_NAME}
                DESTINATION ${PROJECT_SOURCE_DIR}/../ClaRaDelay/Resources/Library/linux64)
    else (UNIX)
        install(TARGETS ${PROJECT_NAME}
                DESTINATION ${PROJECT_SOURCE_DIR}/../ClaRaDelay/Resources/Library/win64)
    endif (UNIX)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    if (UNIX)
        install(TARGETS ${PROJECT_NAME}
                DESTINATION ${PROJECT_SOURCE_DIR}/../ClaRaDelay/Resources/Library/linux32)
    else (UNIX)
        install(TARGETS ${PROJECT_NAME}
                DESTINATION ${PROJECT_SOURCE_DIR}/../ClaRaDelay/Resources/Library/win32)
    endif (UNIX)
endif()


