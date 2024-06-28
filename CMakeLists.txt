#[[
    Configure with preset:
        cmake --preset default
    Build with preset:
        cmake --build --preset default
    Clean with preset (delete the build directory):
        cmake --build ./build --target clean-all
]]
cmake_minimum_required(VERSION 3.25)
project(
    WebGPU
    VERSION 0.1.0
    LANGUAGES CXX C
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${SRC_DIR}/*.h")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")

add_executable(${PROJECT_NAME} ${APP_HEADERS} ${APP_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
target_link_options(${PROJECT_NAME} PRIVATE "-sUSE_WEBGPU=1" "-sUSE_GLFW=3")


add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${PROJECT_SOURCE_DIR}/emscripten/post-build.sh \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\"
    COMMENT "Running emscripten/post-build.sh..."
)


add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
)