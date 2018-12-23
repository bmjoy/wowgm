include(CheckSymbolExists)
if (WIN32)
  check_symbol_exists(open io.h HAVE_OPEN)
else ()
  check_symbol_exists(open fcntl.h HAVE_OPEN)
endif ()

set(FMT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/chrono.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/color.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/core.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/format.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/format.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/format-inl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/locale.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/ostream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/printf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/ranges.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/time.h)

if (HAVE_OPEN)
  set(FMT_SOURCES ${FMT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/posix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/posix.cc)
endif()

add_library(fmt STATIC ${FMT_SOURCES})

target_include_directories(fmt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(fmt
  PUBLIC
    -DFMT_USE_OVERRIDE
    -DFMT_USE_VARIADIC_TEMPLATES
    -DFMT_USE_RVALUE_REFERENCES
    -DFMT_USE_DELETED_FUNCTIONS
    -DFMT_USE_EXTERN_TEMPLATES)

set_target_properties(fmt
  PROPERTIES
    FOLDER
    "dep"
)