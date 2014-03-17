project(nkdhny.richquery)
cmake_minimum_required(VERSION 2.6)

include( CheckIncludeFiles )
include( CTest )

if(DEBUG)
	add_definitions(-DDEBUG)
endif(DEBUG)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.hpp *.h)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS *.cpp)

list(REMOVE_ITEM SOURCES templatefunctionaltest.cpp queryfunctionaltest.cpp pooltest.cpp poolactionsfunctionaltest.cpp)

add_library(richquery ${HEADERS} ${SOURCES})
target_link_libraries(richquery pq)

#unit
add_executable(pooltest pooltest.cpp)
target_link_libraries(pooltest richquery gtest pthread)
add_test(pooltest ${EXECUTABLE_OUTPUT_PATH}/pooltest)

#functional
add_executable(templatefunctionaltest templatefunctionaltest.cpp)
target_link_libraries(templatefunctionaltest richquery gtest pthread)

add_executable(queryfunctionaltest queryfunctionaltest.cpp)
target_link_libraries(queryfunctionaltest richquery gtest pthread)

add_executable(poolactionsfunctionaltest poolactionsfunctionaltest.cpp)
target_link_libraries(poolactionsfunctionaltest richquery gtest pthread)
