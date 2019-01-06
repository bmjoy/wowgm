
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes
  # ...
)

find_package(ZLIB REQUIRED)
find_package(DISCORD)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(WowGM
  ${PRIVATE_SOURCES}
)

set_target_properties(WowGM PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

target_link_libraries(WowGM
  PRIVATE
    boost
    zlib
    extstd
    graphics
    shared
    imgui
    ${DISCORD_STATIC_LIBRARY}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_INCLUDES
  # Excludes
  # ...
)

target_include_directories(WowGM
  PUBLIC
    ${CMAKE_BINARY_DIR}
  PRIVATE
    ${PRIVATE_INCLUDES}
    ${SHARED_INCLUDE_DIRS}
    ${GRAPHICS_INCLUDE_DIRS}
    ${DISCORD_INCLUDE_DIR}
)

add_dependencies(WowGM extstd graphics shared)

add_definitions(-DWIN32_LEAN_AND_MEAN)

if (DISCORD_FOUND)
  add_definitions(-DDISCORD_PRESENCE)
endif ()

add_custom_command(TARGET WowGM POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:WowGM>/resources
)

add_custom_command(TARGET WowGM POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${DISCORD_DYNAMIC_LIBRARY} $<TARGET_FILE_DIR:WowGM>
)

install(TARGETS WowGM DESTINATION "${CMAKE_INSTALL_PREFIX}")