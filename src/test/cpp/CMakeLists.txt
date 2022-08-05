project(wiztoolkit_test)

include_directories(${GTEST_INCLUDE_DIRS})

include_directories(
  ../../main/cpp
  ../../deps/logging
)

add_executable(wtk-test
  wtk/utils/is_prime.test.cpp
  wtk/utils/SkipList.test.cpp
  wtk/utils/IRTreeUtils.test.cpp
)

target_link_libraries(wtk-test
  gtest
  gtest_main
  wiztoolkit
)
