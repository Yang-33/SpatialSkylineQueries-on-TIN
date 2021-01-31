# CGAL and its components
find_package( CGAL QUIET COMPONENTS  )
if ( NOT CGAL_FOUND )
  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()
endif()

# CGAL VIEWER
find_package(CGAL COMPONENTS Qt5)

# Boost and its components
find_package( Boost REQUIRED )
if ( NOT Boost_FOUND )
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()
endif()

# Binary
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_executable( create_fractal_points create_fractal_points.cc )
target_link_libraries( create_fractal_points PRIVATE CGAL::CGAL CGAL::CGAL_Qt5 )

find_package(Eigen3 3.1.0)
  if (EIGEN3_FOUND)
  include( ${EIGEN3_USE_FILE} )
endif()

add_executable( tin_reconstruction_from_points tin_reconstruction_from_points.cc )
target_link_libraries( tin_reconstruction_from_points PRIVATE CGAL::CGAL )

add_executable( tin_validator tin_validator.cc )
target_link_libraries( tin_validator PRIVATE CGAL::CGAL CGAL::CGAL_Qt5 glog)
