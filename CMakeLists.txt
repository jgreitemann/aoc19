cmake_minimum_required(VERSION 3.8)
project(aoc19 CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-fconcepts)

include_directories(cmcstl2/include)

file(GLOB days "day*")
foreach(day ${days})
	add_subdirectory(${day})
endforeach()
