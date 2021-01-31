set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_subdirectory( skyline )
add_subdirectory( tin )

## test

set( EXECUTE_BINARY skyline_tin_all_tests )

file( GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES true *test.h *test.cc )
set( TEST_SOURCES ${TEST_SOURCES} )

add_executable( ${EXECUTE_BINARY} ${TEST_SOURCES} )
target_link_libraries( ${EXECUTE_BINARY} gtest gtest_main glog gflags CGAL::CGAL )
