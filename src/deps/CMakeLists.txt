cmake_minimum_required(VERSION 3.1)

if(${ENABLE_GTEST} EQUAL 1)
  add_subdirectory(gtest EXCLUDE_FROM_ALL)
endif()

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

if(${ENABLE_FLATBUFFER} EQUAL 1)
  add_subdirectory(flatbuffer)
endif()
