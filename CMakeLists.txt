set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
include(CMakeLists_inc_cotire)

set(TESTMODE false CACHE BOOL "Enable test mode")

project(repodbsync)
cmake_minimum_required(VERSION 2.8)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wconversion -Wnon-virtual-dtor -Wswitch-default")
endif()

aux_source_directory(. SRC_LIST)
if(TESTMODE)
  aux_source_directory(tests SRC_LIST)
  file(GLOB INC_LIST "*.h" "tests/*.h")
  list(REMOVE_ITEM "SRC_LIST" ./main.cpp)
  list(REMOVE_ITEM "INC_LIST" ${CMAKE_CURRENT_SOURCE_DIR}/main.h)
else()
  file(GLOB INC_LIST "*.h")
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  set(resources README.md)

  foreach(resource IN LISTS resources)
    message(STATUS ${resource})
      add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${resource}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${resource}" "${CMAKE_CURRENT_BINARY_DIR}/${resource}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${resource}"
      )
  endforeach()

  add_custom_target(resources-target ALL DEPENDS ${resources})
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

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_SOURCE_DIR};${Boost_INCLUDE_DIR}/boost/test/")
cotire(${PROJECT_NAME})
