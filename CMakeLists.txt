cmake_minimum_required (VERSION 3.0)
project (ambilight)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(ambilight main.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

find_package(libftdi REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LIBFTDI_INCLUDE_DIR})
target_link_libraries(ambilight ${LIBFTDI_LIBRARY})

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})
target_link_libraries(ambilight ${X11_LIBRARIES})
