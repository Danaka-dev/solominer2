cmake_minimum_required(VERSION 3.5)
project(solominer)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS OFF)

#- info
message( STATUS "Source Dir : '${CMAKE_CURRENT_SOURCE_DIR}'" )
message( STATUS "Host System name : ${CMAKE_HOST_SYSTEM_NAME}" )
message( STATUS "System name: ${CMAKE_SYSTEM_NAME}" )
message( STATUS "System version: ${CMAKE_SYSTEM_VERSION}" )
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )
message( STATUS "C++ compiler : ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Project : ${CMAKE_PROJECT_NAME}" )
message( STATUS "user ; " $ENV{USER} )

#- modules
set( CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules
)

#- package
message( "#PACKAGES" )

find_package(UV REQUIRED)
find_package(X11 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

#- depends
message( "#DEPENDS" )

add_subdirectory(
    depends
)

include_directories(
    depends
    depends/tiny/
    depends/bitcoin-rpc/src
    depends/bitcoin-blk/ #src
    depends/xmrig-solo/src
)

#- directories
message( "#TARGET" )

include_directories(
    ./src
    ${ZLIB_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
)

#- executable
add_executable( ${CMAKE_PROJECT_NAME}
    ${DEPENDS_HEADER_FILES}
    ${DEPENDS_SOURCE_FILES}
)

target_compile_options( ${CMAKE_PROJECT_NAME}
    PUBLIC -DTINY_NAMESPACE_NAME=solominer
)

add_subdirectory(
    src
)

#- link
# set(CMAKE_EXE_LINKER_FLAGS "-static")

message( STATUS "ZLIB : " ${ZLIB_LIBRARY} )
message( STATUS "PNG : " ${PNG_LIBRARY} )

target_link_libraries( ${CMAKE_PROJECT_NAME}
#static
    # libpng.a libz.a

#shared
    ${PNG_LIBRARY}
    ${ZLIB_LIBRARY} # "z" #NB uncomment if getting InflateEnd undefined reference

#--
    ${X11_LIBRARIES}
    ${DEPENDS_LIB_FILES}
    curl
    uuid
    bitcoinrpc_static
    xmrig_solo
)

set_target_properties( ${CMAKE_PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin"
)

#eof