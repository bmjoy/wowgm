
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes below
  # ...
)

find_package(glm REQUIRED)
find_package(GLI REQUIRED)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(graphics
  ${PRIVATE_SOURCES}
)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Excludes
  # ...
)

target_include_directories(graphics
  PUBLIC
    ${CMAKE_BINARY_DIR}
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SHARED_INCLUDE_DIRS}
    # The graphics stuff.
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/vma
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/glslang
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/SPIRV-cross
)

target_link_libraries(graphics
  PUBLIC
    extstd
    shared
  PRIVATE
    Vulkan::Vulkan
    glfw
    glm
    glslang
    OGLCompiler
    OSDependent
    SPIRV
    SPVRemapper
    spirv-cross-core
    spirv-cross-glsl
)

add_dependencies(graphics extstd shared)

add_definitions(-DWIN32_LEAN_AND_MEAN)
