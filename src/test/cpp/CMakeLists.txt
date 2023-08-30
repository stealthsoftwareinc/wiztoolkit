cmake_minimum_required(VERSION 3.1)

add_compile_options(${wtk_cxx_flags})

include_directories(${GTEST_INCLUDE_DIRS})

include_directories(
  ../../main/cpp
  ../../deps/logging
)

add_executable(wtk-test
  wtk/utils/SkipList.test.cpp
  wtk/utils/CharMap.test.cpp
)

target_link_libraries(wtk-test
  gtest
  gtest_main
  wiztoolkit
)
