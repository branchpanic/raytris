cmake_minimum_required(VERSION 3.0.0)

project(raytris VERSION 0.0.0 LANGUAGES C)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(raylib)

add_executable(raytris main.c tetromino.c)
target_link_libraries(raytris raylib)

add_custom_command(
    TARGET
        raytris
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/colors.txt
        ${CMAKE_CURRENT_BINARY_DIR}/colors.txt
)

execute_process(
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
)
