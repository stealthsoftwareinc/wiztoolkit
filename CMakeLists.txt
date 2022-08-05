cmake_minimum_required(VERSION 3.0)
project(wiztoolkit)

if(${ENABLE_ANTLR} EQUAL 0)
  set(ANTLR_CXX_FLAGS "")
else()
  set(ANTLR_CXX_FLAGS " \
    -Wno-conversion \
    -Wno-unknown-pragmas \
    -Wno-attributes \
    -Wno-implicit-fallthrough")
endif()

if(${ENABLE_FLATBUFFER} EQUAL 0)
  set(FLATBUFFER_CXX_FLAGS "")
else()
  set(FLATBUFFER_CXX_FLAGS " \
    -Wno-conversion")
endif()

set(CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG -O2 -Werror ")
set(CMAKE_CXX_FLAGS_PROFILE " -DNDEBUG -pg -O2 -Werror ")
set(CMAKE_CXX_FLAGS_CI " -Werror ")
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} \
  --std=c++11 \
  -Wall \
  -Wextra \
  -Wconversion \
  -Wcast-align \
  ${ANTLR_CXX_FLAGS} \
  ${FLATBUFFER_CXX_FLAGS} \
  -DENABLE_ANTLR=${ENABLE_ANTLR} \
  -DENABLE_FLATBUFFER=${ENABLE_FLATBUFFER} \
  ")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if(NOT DEFINED OPENSSL_ROOT_DIR)
		IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
			# M1 Apple
			set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl")
			message(STATUS "OPENSSL_ROOT_DIR set to default: ${OPENSSL_ROOT_DIR}")
		ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)")
			# Intel Apple
			set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
			message(STATUS "OPENSSL_ROOT_DIR set to default: ${OPENSSL_ROOT_DIR}")
		ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "(aarch64)|(arm64)" )
	endif()
endif()


if(${ENABLE_ANTLR} EQUAL 1)
  FILE(GLOB gen_antlr_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/target/generated/wtk/antlr/*.cpp
  )

  FILE(GLOB gen_antlr_h
    ${CMAKE_CURRENT_SOURCE_DIR}/target/generated/wtk/antlr/*.h
  )
endif()

FILE(GLOB gen_irregular_cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/target/generated/wtk/irregular/*.cpp
)

FILE(GLOB gen_irregular_h
  ${CMAKE_CURRENT_SOURCE_DIR}/target/generated/wtk/irregular/*.h
)

if(${ENABLE_FLATBUFFER} EQUAL 1)
  FILE(GLOB gen_flatbuffer_h
    ${CMAKE_CURRENT_SOURCE_DIR}/target/generated/wtk/flatbuffer/*.h
  )
endif()

add_subdirectory(src)
