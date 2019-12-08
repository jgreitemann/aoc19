cmake_minimum_required(VERSION 3.8)
project(aoc19 CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS -stdlib=libc++)
add_compile_options(-fcoroutines-ts)

include_directories(include)
include_directories(ranges-v3/include)

file(GLOB days "day*")
foreach(day ${days})
	add_subdirectory(${day})
endforeach()
