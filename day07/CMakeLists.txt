add_executable(thrusters thrusters.cpp ../src/intcode_computer.cpp)
target_link_libraries(thrusters ${CMAKE_THREAD_LIBS_INIT})
