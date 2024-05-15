# -*- cmake -*-
# Finds JSONCPP include directory and library

# This module defines
#  JSONCPP_INCLUDE_DIR : directory where find json.h is located.
#  JSONCPP_LIBRARY : library needed to use jsoncpp.
#  JSONCPP_FOUND : "YES" if jsoncpp was found, "NO" if not found and project should not try to jsoncpp.

# find directory
FIND_PATH( JSONCPP_INCLUDE_DIR json.h
    /usr/include
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/win32-deps/include
    PATH_SUFFIXES jsoncpp/json jsoncpp
)

# find library
FIND_LIBRARY( JSONCPP_LIBRARY NAMES jsoncpp
    HINTS /usr/lib /usr/local/lib ${CMAKE_SOURCE_DIR}/win32-deps/lib
)

# set result variables
IF( JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR )
    SET( JSONCPP_LIBRARY ${JSONCPP_LIBRARY} )
    SET( JSONCPP_FOUND "YES" )
ELSE (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
    SET( JSONCPP_FOUND "NO" )
ENDIF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)

# report
IF( JSONCPP_FOUND )
    IF( NOT JSONCPP_FIND_QUIETLY )
        MESSAGE( STATUS "Found JSONCPP: ${JSONCPP_LIBRARY}" )
    ENDIF (NOT JSONCPP_FIND_QUIETLY)
ELSE (JSONCPP_FOUND)
    IF( JSONCPP_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "Could not find JSONCPP include or library : ${JSONCPP_INCLUDE_DIR}, lib: ${JSONCPP_LIBRARY}" )
    ENDIF (JSONCPP_FIND_REQUIRED)
ENDIF (JSONCPP_FOUND)

# advance
MARK_AS_ADVANCED(
    JSONCPP_INCLUDE_DIR
    JSONCPP_LIBRARY
)

#eof