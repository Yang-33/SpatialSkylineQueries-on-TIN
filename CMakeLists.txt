cmake_minimum_required(VERSION 3.1...3.15)
set( CXX_STANDARD c++17 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -std=${CXX_STANDARD} -pthread -lpthread" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -ftrapv -DYANG33" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native -march=native -mfpmath=both -s -DNDEBUG -O3" )
project( tin-skyline )

include_directories( ./ )
add_subdirectory( source )
