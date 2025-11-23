cmake_minimum_required(VERSION 3.17)
project(dx11sharetest)

set(SDL_STATIC on CACHE BOOL "thing" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/third-party/SDL")

file(GLOB my_sources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*" "${CMAKE_SOURCE_DIR}/include/*")

message(STATUS "sources : ${my_sources}")

add_executable(dx11sharetest ${my_sources})

target_link_libraries(dx11sharetest SDL3::SDL3-static)
target_include_directories(dx11sharetest PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_features(dx11sharetest PRIVATE cxx_std_17)