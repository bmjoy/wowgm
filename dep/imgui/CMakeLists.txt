set(IMGUI_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/stb_rect_pack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/stb_textedit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/stb_truetype.h)


add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(imgui
  PROPERTIES
    FOLDER
    "dep"
)
