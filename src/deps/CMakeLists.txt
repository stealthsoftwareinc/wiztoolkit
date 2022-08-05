cmake_minimum_required(VERSION 3.0)

if(${ENABLE_ANTLR} EQUAL 1)
  # This is sort of a hack, because the top-level CMake file for ANTLR has
  # a lot of confusing "compiler-detection-restrictions"
  set(OLD_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
  set(PROJECT_SOURCE_DIR "${OLD_PROJECT_SOURCE_DIR}/src/deps/antlr4")
  set(ANTLR_VERSION 4.8)

  add_subdirectory(antlr4/runtime)

  unset(ANTLR_VERSION)
  set(PROJECT_SOURCE_DIR "${OLD_PROJECT_SOURCE_DIR}")
endif()

if(${ENABLE_FLATBUFFER} EQUAL 1)
  add_subdirectory(flatbuffer)
endif()

add_subdirectory(gtest EXCLUDE_FROM_ALL)
add_subdirectory(logging)

include_directories(
  sst_bignum/include
)

add_library(sst_bignum
  sst_bignum/src/sst/bignum.cpp
)

target_link_libraries(sst_bignum
  OpenSSL::Crypto
)
