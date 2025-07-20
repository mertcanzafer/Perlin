cmake_minimum_required(VERSION 3.20)
project(PerlinSDL)

# Set Language standards
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

set(SDL3IMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDL3IMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
set(SDL3IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL3IMAGE_JXL OFF CACHE BOOL "" FORCE) # Also disable JPEG XL
set(AVIF OFF CACHE BOOL "" FORCE) # Alternative flag name
set(JXL OFF CACHE BOOL "" FORCE) # Alternative flag name

# Add SDL_image (SDL3_image) subdirectory
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
set(GLM_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # optional but safe

# FetchContent for glm
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

add_subdirectory(src)
