# Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
)

find_package(GLM REQUIRED)
find_package(GLI REQUIRED)
find_package(ZLIB REQUIRED)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_executable(WowGM
  ${PRIVATE_SOURCES}
)

set_target_properties(WowGM PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")
target_link_libraries(WowGM
  ${OPENSSL_LIBRARIES}
  boost
  glfw
  Vulkan::Vulkan
  zlib
  storm
)

add_dependencies(WowGM storm)

target_include_directories(WowGM
  PUBLIC
    ${PUBLIC_INCLUDES}
    ${GLM_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${glfw_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src)

add_definitions(-DWIN32_LEAN_AND_MEAN)

add_custom_command(TARGET WowGM POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:WowGM>/resources)

install(TARGETS WowGM DESTINATION "${CMAKE_INSTALL_PREFIX}")