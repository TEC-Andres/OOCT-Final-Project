cmake_minimum_required(VERSION 3.10)
project(main)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

file(GLOB MAIN_SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")
file(GLOB WINDOWHANDLER_SOURCES "${CMAKE_SOURCE_DIR}/windowHandler/*.cpp")
file(GLOB SCREENS_SOURCES "${CMAKE_SOURCE_DIR}/screens/*.cpp")
file(GLOB GLOBAL_VARS_SOURCES "${CMAKE_SOURCE_DIR}/*.h")

add_executable(main ${MAIN_SOURCES} ${WINDOWHANDLER_SOURCES} ${SCREENS_SOURCES} ${GLOBAL_VARS_SOURCES})
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
