# extstd.CMakeLists

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
  PRIVATE_INCLUDES
  # Excludes
  # ...
)

target_include_directories(extstd
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${PRIVATE_INCLUDES}
    ${Boost_INCLUDE_DIRS}
)

set(EXTSTD_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to src/extstd")

target_link_libraries(extstd
  PRIVATE
    boost
)

add_definitions(-DWIN32_LEAN_AND_MEAN)
