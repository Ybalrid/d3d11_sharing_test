cmake_minimum_required(VERSION 3.17)
project(dx11sharetest)

set(SDL_STATIC on CACHE BOOL "thing" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/third-party/SDL")

file(GLOB my_sources CONFIGURE_DEPENDS src/*)
add_executable(dx11sharetest ${my_sources})
target_link_libraries(dx11sharetest SDL3::SDL3-static)