
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes below
  # ...
)

find_package(GLM REQUIRED)
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
    ${GLM_INCLUDE_DIR}
    ${glfw_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/vma
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/glslang
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/SPIRV-cross
)

target_link_libraries(graphics
  PUBLIC
    extstd
    Vulkan::Vulkan
  PRIVATE
    extstd
    glfw
    glslang
    OGLCompiler
    OSDependent
    SPIRV
    SPVRemapper
    spirv-cross-core
    spirv-cross-glsl
)

add_dependencies(graphics shared)

add_definitions(-DWIN32_LEAN_AND_MEAN)
