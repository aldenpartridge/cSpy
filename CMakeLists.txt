cmake_minimum_required(VERSION 3.10)
project(cSpy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the cURL package
find_package(CURL REQUIRED)

# Include directories for nlohmann/json if you're using a package or header-only library.
# For example, if you have the json.hpp file in a subdirectory "include":
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(cSpy cSpy.cpp)

# Link cURL library
target_link_libraries(cSpy CURL::libcurl)
