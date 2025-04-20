cmake_minimum_required (VERSION 3.0)

project ("POSIX-thread")

add_executable (POSIX-thread main.cpp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET POSIX-thread PROPERTY CXX_STANDARD 17)
endif()

if(LINUX)
	target_link_libraries(my_target PRIVATE pthread)
endif()