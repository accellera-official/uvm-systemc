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

cmake_minimum_required (VERSION 3.16...3.31)

###############################################################################
# Parse version from the authoritative header
###############################################################################

set (_uvm_version_file
    "${CMAKE_CURRENT_SOURCE_DIR}/src/uvmsc/base/uvm_version.h")
if (NOT EXISTS "${_uvm_version_file}")
  message (FATAL_ERROR "Cannot find UVM-SystemC version header: ${_uvm_version_file}")
endif ()

file (READ "${_uvm_version_file}" _uvm_version_content)
string (REGEX MATCH
       "#define[ \t]+UVM_VERSION[ \t]+\"([^\"]+)\""
       _uvm_version_match "${_uvm_version_content}")
string (REGEX REPLACE ".*\"([^\"]+)\".*" "\\1"
       UVMSystemC_VERSION_STRING "${_uvm_version_match}")
string (REGEX MATCH
       "#define[ \t]+UVM_RELEASE_DATE[ \t]+\"([^\"]+)\""
       _uvm_release_match "${_uvm_version_content}")
string (REGEX REPLACE ".*\"([^\"]+)\".*" "\\1"
       UVMSystemC_RELEASE_DATE "${_uvm_release_match}")

if (NOT UVMSystemC_VERSION_STRING MATCHES "^([0-9]+)\\.([0-9]+)")
  message (FATAL_ERROR
          "Cannot parse UVM-SystemC version from ${_uvm_version_file}")
endif ()
set (UVMSystemC_VERSION_MAJOR "${CMAKE_MATCH_1}")
set (UVMSystemC_VERSION_MINOR "${CMAKE_MATCH_2}")
set (UVMSystemC_VERSION_PATCH 0)
set (UVMSystemC_VERSION
    "${UVMSystemC_VERSION_MAJOR}.${UVMSystemC_VERSION_MINOR}.${UVMSystemC_VERSION_PATCH}")
set (UVMSystemC_SOVERSION
    "${UVMSystemC_VERSION_MAJOR}.${UVMSystemC_VERSION_MINOR}")

unset (_uvm_version_content)
unset (_uvm_version_match)
unset (_uvm_release_match)

###############################################################################
# Project declaration
###############################################################################

project (UVMSystemC
  VERSION "${UVMSystemC_VERSION}"
  LANGUAGES CXX C)

if (NOT DEFINED PROJECT_IS_TOP_LEVEL)
  if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set (PROJECT_IS_TOP_LEVEL TRUE)
  else ()
    set (PROJECT_IS_TOP_LEVEL FALSE)
  endif ()
endif ()

get_property (GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

message (STATUS "")
message (STATUS "UVM-SystemC ${UVMSystemC_VERSION_STRING}")

if (PROJECT_IS_TOP_LEVEL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if (DEFINED ENV{UVM_SYSTEMC_HOME} AND IS_ABSOLUTE "$ENV{UVM_SYSTEMC_HOME}")
    set (_uvm_default_prefix "$ENV{UVM_SYSTEMC_HOME}")
  elseif (UNIX)
    set (_uvm_default_prefix "/opt/uvm-systemc")
  elseif (WIN32)
    file (TO_CMAKE_PATH "$ENV{ProgramFiles}" _program_files)
    if (_program_files)
      set (_uvm_default_prefix "${_program_files}/UVM-SystemC")
    else ()
      set (_uvm_default_prefix "${CMAKE_INSTALL_PREFIX}/uvm-systemc")
    endif ()
    unset (_program_files)
  else ()
    set (_uvm_default_prefix "${CMAKE_INSTALL_PREFIX}/uvm-systemc")
  endif ()
  set (CMAKE_INSTALL_PREFIX "${_uvm_default_prefix}" CACHE PATH
      "Installation root directory" FORCE)
  unset (_uvm_default_prefix)
endif ()

###############################################################################
# Build options
###############################################################################

if (NOT (WIN32 OR CYGWIN))
  option (BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
else ()
  option (BUILD_SHARED_LIBS "Build shared instead of static libraries." OFF)
endif ()

if (BUILD_SHARED_LIBS AND (WIN32 OR CYGWIN))
  if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  endif ()
endif ()

option (ENABLE_DEBUG "Include debug symbols." OFF)
option (ENABLE_OPTIMIZE "Enable compiler optimizations." ON)
set (ENABLE_PROFILING "no" CACHE STRING
    "Generate profiling instrumentation: no, prof, gprof, or instrument. Implies ENABLE_DEBUG.")
set_property (CACHE ENABLE_PROFILING PROPERTY STRINGS no prof gprof instrument)

set (UVM_SYSTEMC_REGEX "posix" CACHE STRING
    "Regular expression backend: c++11, pcre, or posix.")
set_property (CACHE UVM_SYSTEMC_REGEX PROPERTY STRINGS c++11 pcre posix)

option (UVM_SYSTEMC_SYSTEMC_COMPAT_CHECK
       "Compile-check that SystemC is IEEE 1666-2011 or newer." ON)

# Install layout options (mirroring the SystemC conventions so that a UVM
# installation can share the same prefix as a SystemC installation).
option (INSTALL_TO_LIB_BUILD_TYPE_DIR
       "Install libraries to lib-<build-type>." OFF)
if (NOT INSTALL_TO_LIB_BUILD_TYPE_DIR)
  option (INSTALL_TO_LIB_TARGET_ARCH_DIR
         "Install libraries to lib-<target-arch>." OFF)
else ()
  unset (INSTALL_TO_LIB_TARGET_ARCH_DIR CACHE)
endif ()
if (CMAKE_HOST_UNIX AND NOT INSTALL_TO_LIB_TARGET_ARCH_DIR)
  option (INSTALL_LIB_TARGET_ARCH_SYMLINK
         "Create a lib-<target-arch> symlink to the library directory." OFF)
else ()
  unset (INSTALL_LIB_TARGET_ARCH_SYMLINK CACHE)
endif ()

mark_as_advanced (
  ENABLE_DEBUG
  ENABLE_OPTIMIZE
  ENABLE_PROFILING
  UVM_SYSTEMC_SYSTEMC_COMPAT_CHECK)

###############################################################################
# Build-type default (single-config generators only)
###############################################################################

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING
       "Build type (Release, Debug, RelWithDebInfo, MinSizeRel)." FORCE)
endif ()

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

###############################################################################
# Compile options — directory scope
###############################################################################

string (TOLOWER "${ENABLE_PROFILING}" ENABLE_PROFILING)
if (NOT ENABLE_PROFILING MATCHES "^(no|prof|gprof|instrument)$")
  message (FATAL_ERROR
          "Invalid ENABLE_PROFILING value '${ENABLE_PROFILING}'. "
          "Choose no, prof, gprof, or instrument.")
endif ()
if (NOT ENABLE_PROFILING STREQUAL "no")
  if (MSVC)
    message (FATAL_ERROR "ENABLE_PROFILING is not supported with MSVC.")
  endif ()
  set (ENABLE_DEBUG ON CACHE BOOL "Forced ON by ENABLE_PROFILING." FORCE)
endif ()

if (ENABLE_DEBUG)
  if (MSVC)
    add_compile_options (/Zi)
  else ()
    add_compile_options (-g)
  endif ()
endif ()
if (ENABLE_PROFILING STREQUAL "prof")
  add_compile_options (-p)
  add_link_options (-p)
elseif (ENABLE_PROFILING STREQUAL "gprof")
  add_compile_options (-pg)
  add_link_options (-pg)
elseif (ENABLE_PROFILING STREQUAL "instrument")
  add_compile_options (-finstrument-functions)
endif ()
if (NOT ENABLE_OPTIMIZE)
  if (MSVC)
    add_compile_options (/Od)
  else ()
    add_compile_options (-O0)
  endif ()
endif ()

###############################################################################
# Dependencies
###############################################################################

include (GNUInstallDirs)

if (NOT SystemCLanguage_FOUND AND NOT TARGET SystemC::systemc)
  if (DEFINED SystemCLanguage_DIR)
    find_package (
      SystemCLanguage 2.3 CONFIG REQUIRED
      PATHS "${SystemCLanguage_DIR}"
      NO_DEFAULT_PATH)
  else ()
    # Treat an explicitly selected SystemCLanguage_DIR as authoritative: only
    # use SYSTEMC_HOME-derived guesses when no explicit selection was made.
    if (DEFINED ENV{SYSTEMC_HOME})
      list (APPEND CMAKE_PREFIX_PATH "$ENV{SYSTEMC_HOME}")

      # Prefer the libdir matching the current UVM-SystemC configuration.
      if (NOT IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
        set (_systemc_preferred_libdir
            "$ENV{SYSTEMC_HOME}/${CMAKE_INSTALL_LIBDIR}")
        if (IS_DIRECTORY "${_systemc_preferred_libdir}")
          list (APPEND CMAKE_PREFIX_PATH "${_systemc_preferred_libdir}")
          set (_systemc_configdir
              "${_systemc_preferred_libdir}/cmake/SystemCLanguage")
          if (IS_DIRECTORY "${_systemc_configdir}")
            list (APPEND CMAKE_PREFIX_PATH "${_systemc_configdir}")
          endif ()
        endif ()
      endif ()

      # Fall back to all available SystemC lib directories.
      file (GLOB _systemc_lib_subdirs LIST_DIRECTORIES TRUE
            "$ENV{SYSTEMC_HOME}/lib*")
      list (REMOVE_ITEM _systemc_lib_subdirs "${_systemc_preferred_libdir}")

      foreach (_systemc_lib_subdir IN LISTS _systemc_lib_subdirs)
        if (IS_DIRECTORY "${_systemc_lib_subdir}")
          list (APPEND CMAKE_PREFIX_PATH "${_systemc_lib_subdir}")
          set (_systemc_configdir
              "${_systemc_lib_subdir}/cmake/SystemCLanguage")
          if (IS_DIRECTORY "${_systemc_configdir}")
            list (APPEND CMAKE_PREFIX_PATH "${_systemc_configdir}")
          endif ()
        endif ()
      endforeach ()

      unset (_systemc_preferred_libdir)
      unset (_systemc_configdir)
      unset (_systemc_lib_subdirs)
      unset (_systemc_lib_subdir)
    endif ()

    find_package (SystemCLanguage 2.3 REQUIRED CONFIG)
  endif ()
endif ()
message (STATUS "Using SystemC ${SystemCLanguage_VERSION} (${SystemCLanguage_DIR})")

if (UVM_SYSTEMC_SYSTEMC_COMPAT_CHECK)
  include (CheckCXXSourceCompiles)
  get_target_property (_uvm_systemc_includes
                      SystemC::systemc INTERFACE_INCLUDE_DIRECTORIES)
  set (CMAKE_REQUIRED_INCLUDES ${_uvm_systemc_includes})
  set (CMAKE_REQUIRED_LIBRARIES SystemC::systemc)
  check_cxx_source_compiles ([=[
    #include <systemc>
    #if !defined(IEEE_1666_SYSTEMC) || IEEE_1666_SYSTEMC < 201101L
    # error "UVM-SystemC requires an IEEE 1666-2011 compatible SystemC"
    #endif
    int sc_main(int, char**) { return 0; }
    int main() { return sc_main(0, 0); }
  ]=] UVM_SYSTEMC_SYSTEMC_COMPATIBLE)
  unset (CMAKE_REQUIRED_INCLUDES)
  unset (CMAKE_REQUIRED_LIBRARIES)
  unset (_uvm_systemc_includes)
  if (NOT UVM_SYSTEMC_SYSTEMC_COMPATIBLE)
    message (FATAL_ERROR
            "The selected SystemC installation is not IEEE 1666-2011 "
            "compatible, or was built with incompatible compiler options.")
  endif ()
endif ()

###############################################################################
# Regular expression backend (mirrors configure.ac --with-regex)
###############################################################################

string (TOLOWER "${UVM_SYSTEMC_REGEX}" UVM_SYSTEMC_REGEX)
if (UVM_SYSTEMC_REGEX STREQUAL "cxx11")
  set (UVM_SYSTEMC_REGEX "c++11")
endif ()
set (UVM_SYSTEMC_REGEX_LIBRARIES)
set (UVM_SYSTEMC_PCRE_ENABLED 0)

include (CheckCXXSourceCompiles)
if (UVM_SYSTEMC_REGEX STREQUAL "c++11")
  check_cxx_source_compiles ([=[
    #include <regex>
    int main() {
      std::regex rx("[(ab)]+", std::regex::extended);
      return std::regex_match("abababab", rx) ? 0 : 1;
    }
  ]=] UVM_SYSTEMC_HAVE_CXX11_REGEX)
  if (NOT UVM_SYSTEMC_HAVE_CXX11_REGEX)
    message (FATAL_ERROR "The compiler does not provide a usable C++ regex implementation.")
  endif ()
  set (HAVE_CXX11_REGEX 1)
elseif (UVM_SYSTEMC_REGEX STREQUAL "pcre")
  find_package (PkgConfig REQUIRED)
  pkg_check_modules (PCRE REQUIRED IMPORTED_TARGET libpcreposix libpcre)
  set (UVM_SYSTEMC_REGEX_LIBRARIES PkgConfig::PCRE)
  set (UVM_SYSTEMC_PCRE_ENABLED 1)
  set (HAVE_PCRE_REGEX 1)
elseif (UVM_SYSTEMC_REGEX STREQUAL "posix")
  set (_uvm_posix_regex_source [=[
    #include <regex.h>
    int main() {
      regex_t rx;
      if (regcomp(&rx, "[(ab)]+", REG_EXTENDED) != 0) return 1;
      int result = regexec(&rx, "abababab", 0, 0, 0);
      regfree(&rx);
      return result;
    }
  ]=])
  check_cxx_source_compiles ("${_uvm_posix_regex_source}"
                            UVM_SYSTEMC_HAVE_POSIX_REGEX)
  if (NOT UVM_SYSTEMC_HAVE_POSIX_REGEX)
    foreach (_uvm_regex_library regex rx posix)
      unset (UVM_SYSTEMC_HAVE_POSIX_REGEX_WITH_LIBRARY CACHE)
      set (CMAKE_REQUIRED_LIBRARIES "${_uvm_regex_library}")
      check_cxx_source_compiles (
        "${_uvm_posix_regex_source}"
        UVM_SYSTEMC_HAVE_POSIX_REGEX_WITH_LIBRARY)
      unset (CMAKE_REQUIRED_LIBRARIES)
      if (UVM_SYSTEMC_HAVE_POSIX_REGEX_WITH_LIBRARY)
        set (UVM_SYSTEMC_HAVE_POSIX_REGEX TRUE)
        set (UVM_SYSTEMC_REGEX_LIBRARIES "${_uvm_regex_library}")
        break ()
      endif ()
    endforeach ()
  endif ()
  unset (_uvm_posix_regex_source)
  unset (_uvm_regex_library)
  if (NOT UVM_SYSTEMC_HAVE_POSIX_REGEX)
    message (FATAL_ERROR "A POSIX regular expression implementation was not found.")
  endif ()
  set (HAVE_POSIX_REGEX 1)
else ()
  message (FATAL_ERROR
          "Invalid UVM_SYSTEMC_REGEX value '${UVM_SYSTEMC_REGEX}'. "
          "Choose c++11, pcre, or posix.")
endif ()

###############################################################################
# Install-layout overrides
###############################################################################

if (NOT UVMSystemC_TARGET_ARCH)
  if (DEFINED SystemC_TARGET_ARCH)
    set (UVMSystemC_TARGET_ARCH "${SystemC_TARGET_ARCH}" CACHE STRING
        "Target architecture (defaults to SystemC_TARGET_ARCH).")
  elseif (DEFINED ENV{SYSTEMC_TARGET_ARCH})
    set (UVMSystemC_TARGET_ARCH "$ENV{SYSTEMC_TARGET_ARCH}" CACHE STRING
        "Target architecture.")
  elseif (DEFINED ENV{SYSTEMC_ARCH})
    set (UVMSystemC_TARGET_ARCH "$ENV{SYSTEMC_ARCH}" CACHE STRING
        "Target architecture.")
  else ()
    string (TOLOWER "${CMAKE_SYSTEM_NAME}${CMAKE_SYSTEM_PROCESSOR}"
           _uvm_default_arch)
    set (UVMSystemC_TARGET_ARCH "${_uvm_default_arch}" CACHE STRING
        "Target architecture.")
    unset (_uvm_default_arch)
  endif ()
endif ()

if (INSTALL_TO_LIB_BUILD_TYPE_DIR)
  string (TOLOWER "lib-${CMAKE_BUILD_TYPE}" _uvm_libdir)
  set (CMAKE_INSTALL_LIBDIR "${_uvm_libdir}" CACHE PATH
      "Library install directory." FORCE)
  unset (_uvm_libdir)
elseif (INSTALL_TO_LIB_TARGET_ARCH_DIR)
  set (CMAKE_INSTALL_LIBDIR "lib-${UVMSystemC_TARGET_ARCH}" CACHE PATH
      "Library install directory." FORCE)
endif ()

if (INSTALL_LIB_TARGET_ARCH_SYMLINK AND CMAKE_HOST_UNIX)
  install (CODE
    "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${CMAKE_INSTALL_LIBDIR}\" \"lib-${UVMSystemC_TARGET_ARCH}\" WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\")")
endif ()

set (CMAKE_INSTALL_DOCDIR "share/doc/uvm-systemc" CACHE PATH
    "Documentation install directory." FORCE)
# A dependency may have populated a normal variable with the same name before
# GNUInstallDirs creates its cache entry. Keep both scopes consistent.
set (CMAKE_INSTALL_DOCDIR "share/doc/uvm-systemc")
if (INSTALL_TO_LIB_BUILD_TYPE_DIR OR INSTALL_TO_LIB_TARGET_ARCH_DIR)
  set (UVMSystemC_INSTALL_CMAKEDIR "share/cmake/UVMSystemC" CACHE PATH
      "CMake package config install directory.")
else ()
  set (UVMSystemC_INSTALL_CMAKEDIR
      "${CMAKE_INSTALL_LIBDIR}/cmake/UVMSystemC" CACHE PATH
      "CMake package config install directory.")
endif ()
mark_as_advanced (UVMSystemC_INSTALL_CMAKEDIR)

###############################################################################
# Convenience header-install macro (used by src-tree CMakeLists files)
###############################################################################

macro (install_headers)
  string (REPLACE "${PROJECT_SOURCE_DIR}/src" "" _rel_dir
          "${CMAKE_CURRENT_SOURCE_DIR}")
  install (FILES ${ARGV}
           DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}${_rel_dir}"
           COMPONENT dev)
endmacro ()

###############################################################################
# Source tree
###############################################################################

string (TIMESTAMP UVMSystemC_CONFIGURE_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h")

add_subdirectory (src)

###############################################################################
# Package export — build-tree registration
###############################################################################

export (EXPORT UVMSystemCTargets
       NAMESPACE UVMSystemC::
       FILE "${CMAKE_CURRENT_BINARY_DIR}/UVMSystemCTargets.cmake")
export (PACKAGE UVMSystemC)

###############################################################################
# Package export — install-tree config / version files
###############################################################################

include (CMakePackageConfigHelpers)

configure_package_config_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UVMSystemCConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/UVMSystemCConfig.cmake"
  INSTALL_DESTINATION "${UVMSystemC_INSTALL_CMAKEDIR}")
write_basic_package_version_file (
  "${CMAKE_CURRENT_BINARY_DIR}/UVMSystemCConfigVersion.cmake"
  VERSION "${UVMSystemC_VERSION}"
  COMPATIBILITY AnyNewerVersion)

install (EXPORT UVMSystemCTargets
        NAMESPACE UVMSystemC::
        DESTINATION "${UVMSystemC_INSTALL_CMAKEDIR}"
        COMPONENT dev)
install (FILES
          "${CMAKE_CURRENT_BINARY_DIR}/UVMSystemCConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/UVMSystemCConfigVersion.cmake"
        DESTINATION "${UVMSystemC_INSTALL_CMAKEDIR}"
        COMPONENT dev)

###############################################################################
# pkg-config
###############################################################################

set (prefix "${CMAKE_INSTALL_PREFIX}")
set (exec_prefix "\${prefix}")
set (libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set (includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set (PACKAGE_VERSION "${UVMSystemC_VERSION_STRING}")
set (PACKAGE_URL "https://accellera.org")
set (LIBS)
if (UVM_SYSTEMC_PCRE_ENABLED)
  set (LIBS "-lpcreposix -lpcre")
else ()
  foreach (_uvm_regex_link_item IN LISTS UVM_SYSTEMC_REGEX_LIBRARIES)
    string (APPEND LIBS " -l${_uvm_regex_link_item}")
  endforeach ()
endif ()
string (STRIP "${LIBS}" LIBS)
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uvm-systemc.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/uvm-systemc.pc"
  @ONLY)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/uvm-systemc.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
        COMPONENT dev)

###############################################################################
# Documentation
###############################################################################

install (FILES
          AUTHORS.md ChangeLog CONTRIBUTING.md INSTALL.md LICENSE NEWS NOTICE
          README.md RELEASENOTES.md
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT doc)
install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT doc
        PATTERN "Makefile.am" EXCLUDE
        PATTERN "Makefile.in" EXCLUDE
        PATTERN "Doxyfile.in" EXCLUDE)

option (INSTALL_EXAMPLES "Install the UVM-SystemC examples." ON)

if (INSTALL_EXAMPLES)
  install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/uvmsc/"
          DESTINATION "${CMAKE_INSTALL_DOCDIR}/examples"
          COMPONENT examples
          PATTERN "Makefile.am" EXCLUDE
          PATTERN "Makefile.in" EXCLUDE
          PATTERN "test.am" EXCLUDE
          PATTERN "msvc*" EXCLUDE)
endif ()

###############################################################################
# Regression tests (optional, in-tree)
###############################################################################

option (ENABLE_REGRESSION
        "Enable the in-tree UVM-SystemC regression tests." OFF)

if (ENABLE_REGRESSION)
  include (CTest)
  add_subdirectory (tests)
endif ()

###############################################################################
# Configuration summary
###############################################################################

message (STATUS "")
message (STATUS "UVM-SystemC configuration summary:")
message (STATUS "  Version                         = ${UVMSystemC_VERSION_STRING}")
message (STATUS "  Release date                    = ${UVMSystemC_RELEASE_DATE}")
message (STATUS "  Target architecture             = ${UVMSystemC_TARGET_ARCH}")
message (STATUS "  Build type                      = ${CMAKE_BUILD_TYPE}")
message (STATUS "  BUILD_SHARED_LIBS               = ${BUILD_SHARED_LIBS}")
message (STATUS "  ENABLE_DEBUG                    = ${ENABLE_DEBUG}")
message (STATUS "  ENABLE_OPTIMIZE                 = ${ENABLE_OPTIMIZE}")
message (STATUS "  ENABLE_PROFILING                = ${ENABLE_PROFILING}")
message (STATUS "  UVM_SYSTEMC_REGEX               = ${UVM_SYSTEMC_REGEX}")
message (STATUS "  SystemC compatibility check     = ${UVM_SYSTEMC_SYSTEMC_COMPAT_CHECK}")
message (STATUS "  ENABLE_REGRESSION               = ${ENABLE_REGRESSION}")
message (STATUS "  INSTALL_TO_LIB_BUILD_TYPE_DIR   = ${INSTALL_TO_LIB_BUILD_TYPE_DIR}")
message (STATUS "  INSTALL_TO_LIB_TARGET_ARCH_DIR  = ${INSTALL_TO_LIB_TARGET_ARCH_DIR}")
message (STATUS "  INSTALL_LIB_TARGET_ARCH_SYMLINK = ${INSTALL_LIB_TARGET_ARCH_SYMLINK}")
message (STATUS "  CMAKE_INSTALL_PREFIX            = ${CMAKE_INSTALL_PREFIX}")
message (STATUS "  CMAKE_INSTALL_LIBDIR            = ${CMAKE_INSTALL_LIBDIR}")
message (STATUS "  UVMSystemC_INSTALL_CMAKEDIR     = ${UVMSystemC_INSTALL_CMAKEDIR}")
message (STATUS "")
