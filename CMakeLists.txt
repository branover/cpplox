cmake_minimum_required(VERSION 3.10)

# set the project name
project(cpplox VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wc99-designator")

# sources
file(GLOB source_glob src/*.cc)
file(GLOB header_glob src/*.h include/*.h)
set(SOURCES ${source_glob} ${header_glob})

# add the executable
add_executable(cpplox ${SOURCES})

# includes
target_include_directories(cpplox PRIVATE include)

set_property(TARGET cpplox PROPERTY CXX_STANDARD 20)
# set_property(TARGET cpplox PROPERTY C_STANDARD 99)
