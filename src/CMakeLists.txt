###############################################################################
#
# Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
# more contributor license agreements.  See the NOTICE file distributed
# with this work for additional information regarding copyright ownership.
# Accellera licenses this file to you under the Apache License, Version 2.0.
#
###############################################################################
# Build rules for UVM-SystemC library
#
# Component CMakeLists files register their sources directly on this final
# target with target_sources(), following SystemC's final-target aggregation
# model without the legacy intermediate static collector targets used by AMS.
###############################################################################

add_library (uvm-systemc)
add_library (UVMSystemC::uvm-systemc ALIAS uvm-systemc)

target_include_directories (uvm-systemc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    "${PROJECT_BINARY_DIR}")

target_compile_features (uvm-systemc PUBLIC cxx_std_17)
target_compile_options (uvm-systemc
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
      -Wall -pedantic -Wno-long-long -Wno-variadic-macros>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4244 /wd4267 /wd4996>)
target_link_libraries (uvm-systemc
  PUBLIC SystemC::systemc
  PRIVATE ${UVM_SYSTEMC_REGEX_LIBRARIES})

set_target_properties (uvm-systemc PROPERTIES
  VERSION "${UVMSystemC_VERSION}"
  SOVERSION "${UVMSystemC_SOVERSION}"
  OUTPUT_NAME "uvm-systemc"
  WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory (uvmsc)

install (TARGETS uvm-systemc
        EXPORT UVMSystemCTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lib
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lib)

install (FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/uvm"
          "${CMAKE_CURRENT_SOURCE_DIR}/uvm.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT dev)
