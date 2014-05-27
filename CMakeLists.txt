set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

set(TESTMODE false CACHE BOOL "Enable test mode")

project(repodbsync)
cmake_minimum_required(VERSION 2.8)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

aux_source_directory(. SRC_LIST)
file(GLOB_RECURSE INC_LIST "*.h")

if(TESTMODE)
  list(REMOVE_ITEM "SRC_LIST" ./main.cpp)
  list(REMOVE_ITEM "INC_LIST" ./main.h)
  aux_source_directory(tests SRC_LIST)
  file(GLOB_RECURSE INC_LIST "tests/*.h")
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST} ${INC_LIST})

find_package(Boost COMPONENTS system filesystem REQUIRED)
target_link_libraries(${PROJECT_NAME}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

if(TESTMODE)
  find_package(Boost COMPONENTS unit_test_framework REQUIRED)
  target_link_libraries(${PROJECT_NAME}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  )
  ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
endif()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  cotire(${PROJECT_NAME})
endif()
