
CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
)

find_package(GLM REQUIRED)
find_package(GLI REQUIRED)
find_package(ZLIB REQUIRED)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES)

add_executable(WowGM
  ${PRIVATE_SOURCES}
)

set_target_properties(WowGM PROPERTIES LINK_FLAGS "${worldserver_LINK_FLAGS}")
target_link_libraries(WowGM
  PUBLIC ${OPENSSL_LIBRARIES}
  PUBLIC Vulkan::Vulkan
  PRIVATE boost
  PRIVATE glfw
  PRIVATE zlib
  PRIVATE storm
  PRIVATE glslang
  PRIVATE OGLCompiler
  PRIVATE OSDependent
  PRIVATE SPIRV
  PRIVATE SPVRemapper
  PRIVATE spirv-cross-core
  PRIVATE spirv-cross-glsl
)

add_dependencies(WowGM storm)

target_include_directories(WowGM
  PUBLIC
    ${PUBLIC_INCLUDES}
    ${GLM_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${glfw_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/StormLib/src
    ${CMAKE_SOURCE_DIR}/dep/vma
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/glslang
    ${CMAKE_SOURCE_DIR}/dep/spirv-compiler/SPIRV-cross)

add_definitions(-DWIN32_LEAN_AND_MEAN)

add_custom_command(TARGET WowGM POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:WowGM>/resources)

install(TARGETS WowGM DESTINATION "${CMAKE_INSTALL_PREFIX}")