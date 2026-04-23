cmake_minimum_required(VERSION 3.16)

project(buniq LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install prefix" FORCE)
endif()

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

function(add_buniq_compare_test test_name case_name)
  add_test(NAME ${test_name}
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/compare_output.sh
      $<TARGET_FILE:buniq>
      ${case_name}
  )
endfunction()

add_buniq_compare_test(compare_empty empty)
add_buniq_compare_test(compare_small small)
add_buniq_compare_test(compare_cross_chunk cross_chunk)
add_buniq_compare_test(compare_no_final_newline no_final_newline)

function(add_buniq_error_test test_name case_name)
  add_test(NAME ${test_name}
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/error_test.sh
      $<TARGET_FILE:buniq>
      ${case_name}
  )
endfunction()

add_buniq_error_test(error_too_long to_be_determined)

install(TARGETS buniq
  RUNTIME DESTINATION bin
)
