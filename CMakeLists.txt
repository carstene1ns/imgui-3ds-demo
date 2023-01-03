cmake_minimum_required(VERSION 3.18)

project(imgui_3ds_example VERSION 0.0.1 LANGUAGES CXX)

add_subdirectory(ext)

add_executable(ex
  src/main.cpp
)
target_compile_features(ex PUBLIC cxx_std_17)
target_link_libraries(ex PUBLIC
  imgui_citro3d
  $<IF:$<CONFIG:Debug>,citro3dd,citro3d>
)

#dkp_add_asset_target(romfs ${CMAKE_CURRENT_BINARY_DIR}/romfs)
#dkp_install_assets(romfs
#  DESTINATION gfx
#  TARGETS imgui_vsh
#)
ctr_generate_smdh(ex.smdh
  NAME ex
  AUTHOR c1
)
ctr_create_3dsx(ex
  SMDH ex.smdh
#  ROMFS romfs
)
