
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes below
  # ...
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(extstd
  ${PRIVATE_SOURCES}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Excludes
  # ...
)

target_include_directories(extstd
  PUBLIC
    ${CMAKE_BINARY_DIR}
    ${PUBLIC_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIR}
)

set(EXTSTD_INCLUDE_DIRS ${CMAKE_INSTALL_FULL_INCLUDEDIR})

target_link_libraries(extstd
  PUBLIC
    boost
)

add_definitions(-DWIN32_LEAN_AND_MEAN)
