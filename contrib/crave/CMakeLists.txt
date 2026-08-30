project ( crave2uvm )
cmake_minimum_required(VERSION 2.8)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#SystemC
find_package(SystemC REQUIRED)

#CRAVE
find_package(CRAVE REQUIRED)

#UVM-SystemC
find_package(UVM-SystemC REQUIRED)

#Includes setzen
include_directories (${SystemC_INCLUDE_DIRS} ${CRAVE_INCLUDE_DIRS} ${UVM_SystemC_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src/include)

#Compiler flags
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread -g -ggdb")

add_subdirectory (examples)