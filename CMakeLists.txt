cmake_minimum_required(VERSION 3.28)
project(TinySDL3 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(SDL_TEST_LIBRARY OFF)
set(SDL_EXAMPLES OFF)

include(FetchContent)
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.14
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS CONFIG
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.9b # Add "-docking" for dockable windows.
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(
  dear_imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(dear_imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(dear_imgui PUBLIC SDL3::SDL3)

add_executable(TinyGame "src/main.cpp")
target_link_libraries(TinyGame PRIVATE SDL3::SDL3 dear_imgui)

if(MSVC)
  target_compile_options(TinyGame PRIVATE /W4)
else()
  target_compile_options(TinyGame PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

add_custom_target(
  sync_assets
  ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR}/textures
    $<TARGET_FILE_DIR:TinyGame>/textures
  VERBATIM
)
add_dependencies(TinyGame sync_assets)

if(WIN32)
  add_custom_command(
    TARGET TinyGame
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:TinyGame> $<TARGET_FILE_DIR:TinyGame>
    COMMAND_EXPAND_LISTS
  )
endif()
