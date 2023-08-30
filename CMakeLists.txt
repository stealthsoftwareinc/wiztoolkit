cmake_minimum_required(VERSION 3.1)
project(wiztoolkit VERSION 2.0.0)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG -O2 -Werror ")
set(CMAKE_CXX_FLAGS_DEBUG " -O0 -g -DLOG_OPT_FILE_LINE " )
set(CMAKE_CXX_FLAGS_CI " -O1 -Werror ")


list(APPEND wtk_cxx_flags
  -Wall
  -Wextra
  -Wconversion
  -Wsign-conversion
  -Wcast-align
)

if(${ENABLE_FLATBUFFER} EQUAL 1)
  list(APPEND wtk_cxx_flags -Wno-sign-conversion)
endif()

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

find_package(OpenSSL REQUIRED)

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

add_subdirectory(src/deps)
add_subdirectory(src/main/cpp)

if(${ENABLE_GTEST} EQUAL 1)
  add_subdirectory(src/test/cpp)
endif()
