cmake_minimum_required(VERSION 3.5)
project(solominer)

set(CMAKE_CXX_STANDARD 14)

#- files
file( GLOB_RECURSE HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

file( GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#- exceptions
# list( REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platform-win32.cpp)
# list( REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/platform-linux.cpp)

#- target
target_sources( ${CMAKE_PROJECT_NAME}
    PRIVATE
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

#eof