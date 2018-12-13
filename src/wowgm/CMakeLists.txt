
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes
  # ...
)

find_package(ZLIB REQUIRED)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(WowGM
  ${PRIVATE_SOURCES}
)

set_target_properties(WowGM PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")

target_link_libraries(WowGM
  PRIVATE
    boost
    zlib
    storm
    extstd
    graphics
  PUBLIC
    ${OPENSSL_LIBRARIES}
)

add_dependencies(WowGM storm extstd graphics shared)

target_include_directories(WowGM
  PUBLIC
    ${PUBLIC_INCLUDES}
    ${Boost_INCLUDE_DIR})

add_definitions(-DWIN32_LEAN_AND_MEAN)

add_custom_command(TARGET WowGM POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:WowGM>/resources)

install(TARGETS WowGM DESTINATION "${CMAKE_INSTALL_PREFIX}")