
include(FetchContent)
FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_compile_definitions(imgui PUBLIC
  IMGUI_DISABLE_OBSOLETE_KEYIO
  IMGUI_DISABLE_OBSOLETE_FUNCTIONS
)

enable_language(ASM)
ctr_add_shader_library(vshader imgui_citro3d/vshader.v.pica)
dkp_add_embedded_binary_library(shader vshader)
add_library(imgui_citro3d STATIC
  imgui_citro3d/imgui_citro3d.cpp
  imgui_citro3d/imgui_citro3d.h
  imgui_citro3d/imgui_ctru.cpp
  imgui_citro3d/imgui_ctru.h
)
target_include_directories(imgui_citro3d PUBLIC imgui_citro3d)
target_link_libraries(imgui_citro3d imgui shader)
