cmake_minimum_required(VERSION 3.5)
project(solominer)

set(CMAKE_CXX_STANDARD 11)

### depends

# bitcoin-rpc
message( STATUS "BUILDING dependency : bitcoin-rpc " )
add_subdirectory( bitcoin-rpc )

#bitcoin-blk
message( STATUS "BUILDING dependency : bitcoin-blk " )

file( GLOB_RECURSE BITCOIN_BLK_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-blk/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-blk/*.hpp
)

file( GLOB_RECURSE BITCOIN_BLK_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-blk/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-blk/*.cpp
)

# tiny
message( STATUS "BUILDING dependency : tiny " )

file( GLOB TINY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-app/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-gui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-gui/controls/*.h

if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/OS/linux/*.h
endif()
)

file( GLOB TINY_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/common/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-app/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-gui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/x-gui/controls/*.cpp

if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/OS/linux/tiny-linux-os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tiny/OS/linux/tiny-linux-x11.c
endif()
)

# xmrig-solo
message( STATUS "BUILDING dependency : xmrig-solo " )
add_subdirectory( xmrig-solo )

###

# assemble (parent scope)
message( STATUS "BUILDING dependency : others " )

set( DEPENDS_HEADER_FILES
    ${TINY_HEADERS}
    ${BITCOIN_BLK_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-blk/bitcoin-blk.h #?? why
    PARENT_SCOPE
)

set( DEPENDS_SOURCE_FILES
    ${TINY_SOURCE}
    ${BITCOIN_BLK_SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/xmrig-solo/src/base/crypto/sha3.cpp #?? why
    PARENT_SCOPE
)

#eof