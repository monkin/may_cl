project(may-cl)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
if(BUILD_DOCUMENTATION)
	FIND_PACKAGE(Doxygen)
	add_custom_target(docs COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
		SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif()

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_BUILD_TYPE Debug)
add_subdirectory(src)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(test)
