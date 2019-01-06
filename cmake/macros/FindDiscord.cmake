# - Try to find the Discord RPC library
# Once done this will define the following read-only variables:
#  DISCORD_FOUND - system has the DISCORD library
#  DISCORD_INCLUDE_DIR - the DISCORD include directory
#  DISCORD_STATIC_LIBRARY - The library needed to use DISCORD RPC
#  DISCORD_DYNAMIC_LIBRARY

#=============================================================================
# Copyright 2019 Vincent Piquet <vin.piquet@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

SET (DISCORD_VERSION "3.4.0-e6390c8")

IF (WIN32)

  IF (PLATFORM EQUAL 64)
    SET (DISCORD_ARCH "win64")
  ELSE()
    SET (DISCORD_ARCH "win32")
  ENDIF()
  
  SET (DISCORD_STATIC_NAME "discord-rpc.lib")
  SET (DISCORD_DYNAMIC_NAME "discord-rpc.dll")

ELSEIF (UNIX AND NOT APPLE)

  SET (DISCORD_ARCH "osx")
  SET (DISCORD_STATIC_NAME "libdiscord-rpc.dylib")

ELSEIF (APPLE)

  SET (DISCORD_ARCH "linux")
  SET (DISCORD_STATIC_NAME "libdiscord-rpc.so")

ELSE ()

  MESSAGE(WARNING "Platform not recognized, discord presence will be skipped")

ENDIF ()

SET (DISCORD_STATICLIB_EXISTS (EXISTS "${CMAKE_SOURCE_DIR}/dep/discord/${DISCORD_ARCH}/${DISCORD_STATIC_NAME}"))
SET (DISCORD_DYNLIB_EXISTS (EXISTS "${CMAKE_SOURCE_DIR}/dep/discord/${DISCORD_ARCH}/${DISCORD_DYNAMIC_NAME}"))

IF (DISCORD_STATICLIB_EXISTS)
  SET (DISCORD_STATIC_LIBRARY "${CMAKE_SOURCE_DIR}/dep/discord/${DISCORD_ARCH}/${DISCORD_STATIC_NAME}" CACHE FILEPATH "")
ENDIF ()

IF (DISCORD_DYNLIB_EXISTS)
  SET (DISCORD_DYNAMIC_LIBRARY "${CMAKE_SOURCE_DIR}/dep/discord/${DISCORD_ARCH}/${DISCORD_DYNAMIC_NAME}" CACHE FILEPATH "")
ENDIF()

SET (DISCORD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dep/discord" CACHE PATH "")

SET (DISCORD_FOUND (DISCORD_DYNLIB_EXISTS OR DISCORD_STATICLIB_EXISTS) CACHE BOOL "")
