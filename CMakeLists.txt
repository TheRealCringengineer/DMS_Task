cmake_minimum_required(VERSION 3.15)

project(DMS_Task VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

# Copy compile_commands to project directory, since i am using it for my lsp
if (${CMAKE_GENERATOR} STREQUAL Ninja)
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${PROJECT_BINARY_DIR}/compile_commands.json
        ${PROJECT_SOURCE_DIR}
    )
endif()

add_executable(${PROJECT_NAME} main.cpp Input/Input.cpp CustomConditions.cpp)
configure_file(perception_results.txt ${CMAKE_BINARY_DIR}/perception_results.txt COPYONLY)
