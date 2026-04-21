cmake_minimum_required(VERSION 3.16)

project(buniq LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(buniq
  src/buniq.cpp
)

target_include_directories(buniq PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(buniq PRIVATE cxx_std_17)

find_package(Threads REQUIRED)
target_link_libraries(buniq PRIVATE Threads::Threads)
