# graphics.CMakeLists

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Excludes below
  # ...
)

# find_package(glm REQUIRED)
# find_package(GLI REQUIRED)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(graphics
  ${PRIVATE_SOURCES}
)

target_include_directories(graphics
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${SHARED_INCLUDE_DIRS} # src/extstd
    ${EXTSTD_INCLUDE_DIRS} # src/shared

    ${IMGUI_INCLUDE_DIR}
	
    # Memory allocator
    ${CMAKE_SOURCE_DIR}/dep/vma
	
    # Shader reflection
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/glslang
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/SPIRV-cross
)

target_link_libraries(graphics
  PRIVATE
    Vulkan::Vulkan
    shared
    extstd
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

add_dependencies(graphics shared extstd)

# add_dependencies(graphics glfw)

set(GRAPHICS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to src/graphics")

if( WIN32 )
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()