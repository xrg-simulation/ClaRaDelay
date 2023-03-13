cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(ClaRaDelay C)

set(PROJECT_NAME "Delay-V1")

add_library(${PROJECT_NAME} STATIC "claradelay.c")

set_property(TARGET ${PROJECT_NAME} PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

