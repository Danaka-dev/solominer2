CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(libbitcoin-rpc)

# Set version number
SET(MAJOR_VERSION 1)
SET(MINOR_VERSION 1)
SET(PATCH_VERSION 0)
SET(SO_VERSION    0)

# Set compiler settings
SET( CMAKE_CXX_FLAGS "-std=c++11 -g -Wall" )

set( CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
)

# Find packages
    #NB if using CLion and JSONCPP is not found, you may install it via vcpkg plugin :
    # (if not already present add vcpkg plugin from market place)
    # in main menu go to : view->Tool Windows->Vcpkg
    # in the vcpkg tool window : Packages -> vcpkg -> find:jsoncpp -> install
    # add "-DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake -DCMAKE_PAK=VCPKG" to cmake options :
        # menu -> File -> Settings -> Build,Execution,Deployment -> Cmake .. "CMake options:" textbox in the right pane
        # e.g. <vcpkg-root> will probably be "/home/<username>/.vcpkg-clion/" on Linux

# add_compile_definitions(CMAKE_PAK=${CMAKE_PAK})
# target_compile_definitions(bitcoinrpc PRIVATE CMAKE_PAK=${CMAKE_PAK} )


if( ${CMAKE_PAK} MATCHES "VCPKG" )
 FIND_PACKAGE(jsoncpp CONFIG REQUIRED)
 set( JSONCPP_LIBRARY JsonCpp::JsonCpp )
else()
 FIND_PACKAGE(JSONCPP REQUIRED)
endif()

FIND_PACKAGE(CURL REQUIRED)

message( STATUS " JSONCPP : " ${JSONCPP_LIBRARY} )
message( STATUS " CURL : " ${CURL_LIBRARY} )

# Find header and source files
FILE( GLOB bitcoinrpc_header ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h )
FILE( GLOB bitcoinrpc_source ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp )

#[[ include_directories(
    ../jsoncpp
) ]]

# Set target libraries
ADD_LIBRARY( bitcoinrpc SHARED ${bitcoinrpc_source} )
ADD_LIBRARY( bitcoinrpc_static STATIC ${bitcoinrpc_source} )

# Set output name of static library
SET_TARGET_PROPERTIES( bitcoinrpc_static PROPERTIES OUTPUT_NAME bitcoinrpc )

if( ${CMAKE_PAK} MATCHES "VCPKG" )
    target_compile_options( bitcoinrpc PUBLIC -DCMAKE_PAK=1 )
    target_compile_options( bitcoinrpc_static PUBLIC -DCMAKE_PAK=1 )
endif()

# Link the library with depends
TARGET_LINK_LIBRARIES( bitcoinrpc
    ${CURL_LIBRARY}
    ${JSONCPP_LIBRARY}
)

TARGET_LINK_LIBRARIES( bitcoinrpc_static
    ${CURL_LIBRARY}
    ${JSONCPP_LIBRARY}
)

# Set version settings
SET( VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION} )

SET_TARGET_PROPERTIES( bitcoinrpc bitcoinrpc_static
    PROPERTIES
    VERSION "${VERSION_STRING}"
    SOVERSION "${SO_VERSION}"
)

#eof