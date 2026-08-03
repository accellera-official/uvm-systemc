###############################################################################
#
# Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
# more contributor license agreements.  See the NOTICE file distributed
# with this work for additional information regarding copyright ownership.
# Accellera licenses this file to you under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.  See the License for the specific language governing
# permissions and limitations under the License.
#
###############################################################################
#
# tests/CMakeLists.txt -- discover and build UVM-SystemC regression tests
#
# Adapted from SystemC/tests/CMakeLists.txt and the SystemC AMS regression
# CMake flow.
#
###############################################################################

if (NOT ENABLE_REGRESSION)
  return()
endif()

cmake_minimum_required(VERSION 3.12...3.31) # FindPython3

if (NOT TARGET UVMSystemC::uvm-systemc)
  message (FATAL_ERROR
           "The UVM-SystemC tests must be configured from the project root.")
endif ()

set(TEST_CATEGORY tests)
include("${PROJECT_SOURCE_DIR}/cmake/UVMSystemCTesting.cmake")

###############################################################################

find_package(Python3 COMPONENTS Interpreter REQUIRED) # needed for run-test.py

###############################################################################

set(_UVM_TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(_UVM_TEST_COMMON_INCLUDE "${_UVM_TEST_ROOT}/include")

function(configure_and_add_regression_test TEST_PATH)
  string(REGEX REPLACE "^uvm-systemc/([^/]+).*$" "\\1" TEST_GROUP "${TEST_PATH}")
  get_filename_component(TEST_FOLDER "${TEST_PATH}" DIRECTORY)
  string(REPLACE "/" "-" TEST_NAME "${TEST_PATH}")
  # Strip leading "uvm-systemc-" prefix to keep target names short.
  string(REGEX REPLACE "^uvm-systemc-" "" TEST_NAME "${TEST_NAME}")

  # message(DEBUG "found ${TEST_GROUP} test: ${TEST_NAME} (${TEST_PATH})")
  add_executable(${TEST_NAME})
  set_target_properties(${TEST_NAME} PROPERTIES FOLDER "tests/${TEST_FOLDER}")
  add_dependencies(all-tests ${TEST_NAME})

  set(test_data_dir "${_UVM_TEST_ROOT}/${TEST_PATH}")
  set(test_source_dir "${test_data_dir}")
  if (TEST_PATH STREQUAL
      "uvm-systemc/examples/simple/registers/models/fifo_reg")
    # This compatibility copy has stale relative includes. Build the canonical
    # example while retaining the regression tree's golden/data directory.
    set(test_source_dir
        "${PROJECT_SOURCE_DIR}/examples/uvmsc/simple/registers/models/fifo_reg")
  endif()

  file(GLOB sources CONFIGURE_DEPENDS "${test_data_dir}/*.f")
  if(sources) # found a file list
    list(GET sources 0 source_list)
    get_filename_component(TEST_LEAFNAME ${source_list} NAME_WLE)
    file(STRINGS ${source_list} sources_list)
    set(sources)
    foreach(src ${sources_list})
      string(STRIP "${src}" src)
      if(src STREQUAL "" OR src MATCHES "^#")
        continue()
      endif()
      if(src MATCHES ".*tests/uvm-systemc/examples/(.*)$")
        set(src "${PROJECT_SOURCE_DIR}/examples/uvmsc/${CMAKE_MATCH_1}")
      else()
        # Legacy .f entries are relative to the copied leaf directory's parent.
        get_filename_component(src "${test_source_dir}/../${src}" ABSOLUTE)
      endif()
      list(APPEND sources ${src})
    endforeach()
  else(sources)
    file(GLOB sources CONFIGURE_DEPENDS "${test_source_dir}/*.cpp")
    if(NOT sources)
      message(FATAL_ERROR "No C++ source found for regression test ${TEST_PATH}")
    endif()
    list(GET sources 0 first_source)
    get_filename_component(TEST_LEAFNAME ${first_source} NAME_WLE)
  endif(sources)
  # message(TRACE "found ${TEST_NAME} sources: ${sources}")
  target_sources(${TEST_NAME} PRIVATE ${sources})

  target_include_directories(${TEST_NAME} PRIVATE
    "${test_source_dir}"
    "${_UVM_TEST_COMMON_INCLUDE}")
  target_link_libraries(${TEST_NAME} PRIVATE UVMSystemC::uvm-systemc)

  set(testname ${TEST_PATH}${TEST_SUFFIX})
  add_test(NAME ${testname}
           COMMAND ${Python3_EXECUTABLE}
                   ${PROJECT_SOURCE_DIR}/cmake/run-test.py
                   $<TARGET_FILE:${TEST_NAME}>)

  set(workdir ${CMAKE_CURRENT_BINARY_DIR}/Testing/${TEST_PATH})
  get_filename_component(leafdir ${TEST_PATH} NAME)
  file(MAKE_DIRECTORY ${workdir})
  if (UNIX OR MINGW)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${test_data_dir}
                    ${workdir}/${leafdir})
  else()
    # use mklink /J (junction) on Windows to avoid need for special privileges
    file(TO_NATIVE_PATH ${workdir}/${leafdir} _dstDir)
    file(TO_NATIVE_PATH ${test_data_dir} _srcDir)
    execute_process(COMMAND cmd.exe /c mklink /J "${_dstDir}" "${_srcDir}")
  endif()

  set(env
    SYSTEMC_REGRESSION=1
    SYSTEMC_ARCH=${UVMSystemC_TARGET_ARCH}
    TEST_FULLNAME=${TEST_PATH}
    TEST_LEAFNAME=${TEST_LEAFNAME}
    TEST_BUILD_TYPE=$<CONFIG>
    UVM_HEADER_FILE=${CMAKE_CURRENT_SOURCE_DIR}/headers/uvm-systemc.txt
  )

  if(DEFINED ENV{SYSTEMC_HOME})
    list(APPEND env "SYSTEMC_HOME=$ENV{SYSTEMC_HOME}")
  endif()

  # Windows might heuristically block some test executables when
  # their name contains certain keywords, e.g., "update".
  # Set requested execution level to "asInvoker" to disable heuristic analysis.
  if (WIN32)
    list(APPEND env "__COMPAT_LAYER=RunAsInvoker")
  endif()

  # Preserve the legacy DONTRUN/incomplete-golden behavior.
  set(test_skipped FALSE)
  if(EXISTS "${test_data_dir}/DONTRUN")
    set(test_skipped TRUE)
  endif()
  file(GLOB golden_files "${test_data_dir}/golden/*.log*")
  foreach(golden_file IN LISTS golden_files)
    file(STRINGS "${golden_file}" incomplete_golden
         REGEX "^Check with UVM-SV log before updating")
    if(incomplete_golden)
      set(test_skipped TRUE)
    endif()
  endforeach()
  if(test_skipped)
    list(APPEND env TEST_SKIPPED=yes)
  endif()

  set_tests_properties(${testname} PROPERTIES
      LABELS "${TEST_GROUP}"
      ENVIRONMENT "${env}"
      SKIP_RETURN_CODE 111
      TIMEOUT 300
      WORKING_DIRECTORY ${workdir}
  )
endfunction(configure_and_add_regression_test)

function(discover_regression_tests)
  # find all golden log directories
  file(GLOB_RECURSE tests
       LIST_DIRECTORIES TRUE
       RELATIVE "${_UVM_TEST_ROOT}"
       CONFIGURE_DEPENDS "${_UVM_TEST_ROOT}/golden"
  )
  foreach(test ${tests})
    if(IS_DIRECTORY "${_UVM_TEST_ROOT}/${test}/golden")
      configure_and_add_regression_test(${test})
    endif()
  endforeach()
endfunction()

discover_regression_tests()
