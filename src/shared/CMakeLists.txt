
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes below
  # ...
)

# find_package(...)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(shared
  ${PRIVATE_SOURCES}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Excludes
  # ...
)

target_include_directories(shared
  PUBLIC
    # Provide the binary dir for all child targets
    ${CMAKE_BINARY_DIR}
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

# target_include_directories(shared
#   PUBLIC
#     ${CMAKE_BINARY_DIR}
#     ${PUBLIC_INCLUDES}
#     ${OPENSSL_INCLUDE_DIR}
#     ${Boost_INCLUDE_DIR}
#   PRIVATE
#     ${CMAKE_CURRENT_BINARY_DIR}
# )

set(SHARED_INCLUDE_DIRS ${CMAKE_INSTALL_FULL_INCLUDEDIR})

target_link_libraries(shared
  PUBLIC
    boost
    openssl
)

if( WIN32 )
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()