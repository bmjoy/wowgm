# shared.CMakeLists

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes
  # ...
)

# find_package(...)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(shared
  ${PRIVATE_SOURCES}
)

target_include_directories(shared
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${EXTSTD_INCLUDE_DIRS}   # src/extstd
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src
)

set(SHARED_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to src/shared")

target_link_libraries(shared
  PRIVATE
    boost
    openssl
    extstd
    fmt
    storm
)

add_dependencies(shared openssl boost extstd fmt storm)

if( WIN32 )
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()
