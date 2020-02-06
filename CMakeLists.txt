cmake_minimum_required(VERSION 3.5)
project(cs270)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif ()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(ENABLE_TEST true)
set(ENABLE_FUSE false)

# http://rpg.ifi.uzh.ch/docs/glog.html

find_package(GLOG REQUIRED)
include_directories(${GLOG_INCLUDE_DIR})
set(ExtLibs ${ExtLibs} ${GLOG_LIBRARY})
message(${GLOG_INCLUDE_DIR})

if(${ENABLE_FUSE})
  find_package(FUSE)
  include_directories(${FUSE_INCLUDE_DIR})
  set(ExtLibs ${ExtLibs} ${FUSE_LIBRARY})
  set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64")
  message(${FUSE_INCLUDE_DIR})
endif()

if (${ENABLE_TEST})
  if (DEFINED ENV{GTEST_ROOT})
    set(GTEST_ROOT $ENV{GTEST_ROOT})
  endif ()
  find_package(GTest)
  if (GTEST_FOUND)
    message(STATUS "Found valid GTest version:")
    message(STATUS "  GTest root dir: ${GTEST_ROOT}")
    message(STATUS "  GTest include dir: ${GTEST_INCLUDE_DIRS}")
    message(STATUS "  GTest libraries: ${GTEST_BOTH_LIBRARIES}")
  endif ()
  include_directories(${GTEST_INCLUDE_DIRS})
  set(ExtTestLibs ${ExtLibs} ${GTEST_BOTH_LIBRARIES} pthread)
endif ()

include_directories(./src)
file(GLOB_RECURSE SourceFiles src/*.cpp)
add_library(Core STATIC ${SourceFiles})

if (${ENABLE_TEST})
file(GLOB_RECURSE TestFiles test/*.cpp)
add_executable(CoreTests ${TestFiles})
target_link_libraries(CoreTests Core ${ExtTestLibs})
endif ()

if (${ENABLE_FUSE})
file(GLOB_RECURSE FuseFiles fuse/*.cpp)
add_executable(solidFS ${FuseFiles})
target_link_libraries(solidFS Core ${ExtLibs} fuse)
endif()
