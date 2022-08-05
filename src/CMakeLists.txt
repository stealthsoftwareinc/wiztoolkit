cmake_minimum_required(VERSION 3.0)

find_package(OpenSSL REQUIRED)

add_subdirectory(deps)
add_subdirectory(main/cpp)
add_subdirectory(test/cpp)
