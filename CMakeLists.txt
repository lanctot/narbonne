project(narbonne)

set(CMAKE_CXX_COMPILER "g++")

# default compile & link options
set(CMAKE_CXX_FLAGS "-O2 -DLINUX -Wall")
set(CMAKE_SHARED_LINKER_FLAGS "-O2")

# add debug symbols for valgrind/gdb and profiling symbols for gprof 
#set(CMAKE_CXX_FLAGS "-g -pg -O2 -DLINUX -Wall")
#set(CMAKE_SHARED_LINKER_FLAGS "-g -pg -O2")

# no debug flags and highest optimized version
#set(CMAKE_CXX_FLAGS "-O3 -DLINUX -DNDEBUG -Wall")
#set(CMAKE_SHARED_LINKER_FLAGS "-O3 -DNDEBUG")

add_executable (server server.cpp rules.cpp tiles.cpp util.cpp search.cpp)
target_link_libraries(server m)

add_executable (simple simple.cpp rules.cpp tiles.cpp util.cpp search.cpp)
target_link_libraries(simple m)

add_executable (expm_ai expm_ai.cpp rules.cpp search.cpp tiles.cpp util.cpp expectimax.cpp)
target_link_libraries(expm_ai m)

