#[[
    Configure with preset:
        cmake --preset default
    Build with preset:
        cmake --build --preset default
    Clean with preset (delete the build directory):
        cmake --build ./build --target clean-all
]]
cmake_minimum_required(VERSION 3.21)

project(
    WebGPU
    LANGUAGES CXX C
)

set(CMAKE_BUILD_TYPE Debug)

# Maybe for clangd?
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Corrects cpptools failing to query em++ for and falling back to defailt c++ language version.
add_compile_options(--target=wasm32-unknown-emscripten)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build and output directories
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Set source files from glob
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${SRC_DIR}/*.hpp")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")

#Get GLM from GitHub
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glm)

add_executable(${PROJECT_NAME} ${APP_HEADERS} ${APP_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

#For clangd to understand emscripten include directories.
execute_process(COMMAND em++ --cflags OUTPUT_VARIABLE EM_CFLAGS)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${EM_CFLAGS}")

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
target_link_options(${PROJECT_NAME} PRIVATE
    -fsanitize=undefined -sSAFE_HEAP=1 -sASSERTIONS=1 -Wno-limited-postlink-optimizations
    -g -gsource-map --source-map-base http://localhost:3000/build/out/
    -sUSE_WEBGPU=1
    -sUSE_GLFW=3
    -sASYNCIFY
    --shell-file=${CMAKE_SOURCE_DIR}/template/shell.html
    --preload-file=${CMAKE_SOURCE_DIR}/assets@assets/
)


add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/WebGPU.html"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.html"
    COMMENT "Renaming WebGPU.html to index.html..."
)


# Define a custom target to clean the build directory
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Deleting ${CMAKE_BINARY_DIR}..."
)
