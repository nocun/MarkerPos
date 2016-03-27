cmake_minimum_required (VERSION 2.8)
project (MarkerPos)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_MARKERS")

# MarkerPos source
file(GLOB_RECURSE MARKER_POS_SRC
	"${CMAKE_SOURCE_DIR}/src/*.h"
	"${CMAKE_SOURCE_DIR}/src/*.cpp")

# Final target
add_executable(MarkerPos ${MARKER_POS_SRC})

# OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIRS})
	target_link_libraries(MarkerPos ${OPENGL_LIBRARIES})
endif()

# FreeGLUT
find_package(FreeGLUT REQUIRED)
if(FREEGLUT_FOUND)
	include_directories(${FREEGLUT_INCLUDE_DIRS})
	add_definitions(${FREEGLUT_DEFINITIONS})
	target_link_libraries(MarkerPos ${FREEGLUT_LIBRARIES})
endif()

# OpenCV
find_package(OpenCV REQUIRED core highgui imgproc calib3d)
if(OPENCV_FOUND)
	include_directories(${OpenCV_INCLUDE_DIRS})
	link_directories(${OpenCV_LIB_DIR})
	target_link_libraries(MarkerPos ${OpenCV_LIBS})
endif()
