//----------------------------------------------------------------------
//   Copyright 2018 COSEDA Technologies GmbH
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//----------------------------------------------------------------------

#ifndef UVM_GLOBALS_H_
#define UVM_GLOBALS_H_

#define UVM_POUND_ZERO_COUNT 1 // used for NBA wait

#include <systemc>

#include <string>
#include <ostream>

#include "uvmsc/base/uvm_object_globals.h"

//////////////

namespace uvm {

//forward declarations
//class uvm_root;
class uvm_object;
class uvm_component;
class uvm_printer;

//------------------------------------------------------------------------------
//
// Global helper functions to use some uvm_root methods in sc_main
//
//------------------------------------------------------------------------------

void run_test( const std::string& test_name = "" );

void print_topology( uvm_printer* printer = nullptr );

//------------------------------------------------------------------------------
//
// Configuration
//
//! non-standard functions
//! These are temporary functions, which should be replaced by the commandline
//! interface options in the future
//------------------------------------------------------------------------------

void uvm_set_config_int( const std::string& inst_name,
                         const std::string& field_name,
                         int value );

void uvm_set_config_string( const std::string& inst_name,
                            const std::string& field_name,
                            const std::string& value );

//------------------------------------------------------------------------------
//
// Function to control additional debugging message generation during 
// get_config_xxx() calls. 
//
//------------------------------------------------------------------------------

void uvm_print_config_matches(bool b);

///////////////

//------------------------------------------------------------------------------
//
// Global factory interface functions.
//
//------------------------------------------------------------------------------

uvm_object* uvm_create_object(
  const std::string& type_name,
  const std::string& inst_path = "",
  const std::string& name = "",
  bool no_overrides = false
);

uvm_component* uvm_create_component(
  const std::string& type_name,
  const std::string& inst_path = "",
  const std::string& name = ""
);

// global override functions follow the API as used in uvm_component class
void uvm_set_type_override(
  const std::string& original_type_name,
  const std::string& override_type_name,
  bool replace = true
);

void uvm_set_inst_override(
  const std::string& original_type_name,
  const std::string& override_type_name,
  const std::string& full_inst_path
);

// TODO temporary set verbosity level. Should be moved to
// command line interface

void uvm_set_verbosity_level(int verbosity_level);

//------------------------------------------------------------------------------
//
// Global reporting
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function: uvm_report_enabled
//
// Returns true if the configured verbosity at the UVM root is greater than
// ~verbosity~ and the action associated with the given ~severity~ and ~id~
// is not UVM_NO_ACTION, else returns false.
//------------------------------------------------------------------------------

bool uvm_report_enabled( const int& verbosity,
                         const uvm_severity& severity = UVM_INFO,
                         const std::string& id = "" );

void uvm_report( uvm_severity severity,
                 const std::string& id,
                 const std::string& message,
                 int verbosity = -1, // TODO check default
                 const std::string& filename = "",
                 int line = 0,
                 const std::string& context_name = "",
                 bool report_enabled_checked = false );

void uvm_report_info( const std::string& id,
                      const std::string& message,
                      int verbosity = UVM_NONE,
                      const std::string& filename = "",
                      int line = 0,
                      const std::string& context_name = "",
                      bool report_enabled_checked = false );

void uvm_report_warning( const std::string& id,
                         const std::string& message,
                         int verbosity = UVM_NONE,
                         const std::string& filename = "",
                         int line = 0,
                         const std::string&  context_name = "",
                         bool report_enabled_checked = false );

void uvm_report_error( const std::string& id,
                       const std::string& message,
                       int verbosity = UVM_NONE,
                       const std::string& filename = "",
                       int line = 0,
                       const std::string&  context_name = "",
                       bool report_enabled_checked = false );

void uvm_report_fatal( const std::string& id,
                       const std::string& message,
                       int verbosity = UVM_NONE,
                       const std::string& filename = "",
                       int line = 0,
                       const std::string&  context_name = "",
                       bool report_enabled_checked = false );

//------------------------------------------------------------------------------
// Other utilities
//------------------------------------------------------------------------------

void uvm_wait_for_nba_region();

bool uvm_is_match ( const std::string& expr, const std::string& str );

bool uvm_has_wildcard ( const std::string& arg);

std::string uvm_glob_to_re( const std::string& s );

const char* uvm_glob_to_re_char(const char *glob);

bool uvm_re_match( const std::string& re,  const std::string& str);

int uvm_re_match_char(const char * re, const char *str);

int uvm_extract_path_index(const std::string& path, std::string& objname, int& begin, int& end );

std::vector<std::string> uvm_re_match2(const std::string& expr, const std::string& str);

} // namespace uvm

#endif /* UVM_GLOBALS_H_ */
