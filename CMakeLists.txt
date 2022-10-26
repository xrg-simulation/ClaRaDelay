cmake_minimum_required(VERSION 3.9)

project(ClaRaDelay C)

set(PROJECT_NAME "Delay-V1")

add_library(${PROJECT_NAME} STATIC "claradelay.c")
