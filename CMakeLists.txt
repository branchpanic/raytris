cmake_minimum_required(VERSION 3.0.0)

project(raytris VERSION 0.0.0 LANGUAGES C)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/raylib)
include_directories(third_party/inih)

add_executable(raytris main.c third_party/inih/ini.c tetromino.c raytris.c settings.c graphics.c)

target_link_libraries(raytris raylib)

execute_process(
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
)
