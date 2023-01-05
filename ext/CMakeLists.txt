
add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC imgui)
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
