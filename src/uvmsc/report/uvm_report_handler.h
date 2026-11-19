//------------------------------------------------------------------------------
//   Copyright 2013-2016 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2013 NVIDIA Corporation
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
//------------------------------------------------------------------------------

#ifndef UVM_REPORT_HANDLER_H_
#define UVM_REPORT_HANDLER_H_

#include <string>
#include <map>

/*
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_root.h"
*/
#include "uvmsc/factory/uvm_object_registry.h"
#include "uvmsc/macros/uvm_object_defines.h"

namespace uvm {

// forward class declaration
class uvm_report_object;
class uvm_report_server;

//------------------------------------------------------------------------------
// CLASS: uvm_report_handler
//
//! The uvm_report_handler is the class to which most methods in
//! #uvm_report_object delegate. It stores the maximum verbosity, actions,
//! and files that affect the way reports are handled.
//!
//! The report handler is not intended for direct use. See #uvm_report_object
//! for information on the UVM reporting mechanism.
//!
//! The relationship between #uvm_report_object (a base class for #uvm_component)
//! and #uvm_report_handler is typically one to one, but it can be many to one
//! if several #uvm_report_objects are configured to use the same
//! #uvm_report_handler_object. See uvm_report_object::set_report_handler.
//!
//! The relationship between #uvm_report_handler and #uvm_report_server is many
//! to one.
//------------------------------------------------------------------------------

typedef std::map<std::string, uvm_action> uvm_id_actions_array;
typedef std::map<std::string, UVM_FILE> uvm_id_file_array;
typedef std::map<std::string, int> uvm_id_verbosities_array;
typedef std::map<uvm_severity, uvm_severity> uvm_sev_override_array;

class uvm_report_handler : public uvm_object
{
 public:
  friend class uvm_report_object;
  friend class uvm_report_catcher;

  UVM_OBJECT_UTILS(uvm_report_handler);

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  uvm_report_handler( const std::string name = "uvm_report_handler");

  void do_print( const uvm_printer& printer ) const override;

  //--------------------------------------------------------------------
  // Group: Message processing
  //--------------------------------------------------------------------

  void process_report_message(uvm_report_message* report_message);

  //--------------------------------------------------------------------
  // Group: Convenience methods
  //--------------------------------------------------------------------

  std::string format_action( uvm_action action ) const;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  int get_verbosity_level( uvm_severity severity = UVM_INFO,
                           const std::string& id = "" );

  uvm_action get_action( uvm_severity severity,
                         const std::string& id );

  UVM_FILE get_file_handle( uvm_severity severity,
                            const std::string& id );

  virtual void report( uvm_severity severity,
                       const std::string& name,
                       const std::string& id,
                       const std::string& message,
                       int verbosity_level = UVM_MEDIUM,
                       const std::string& filename = "",
                       int line = 0,
                       uvm_report_object* client = nullptr );

  ~uvm_report_handler() override;

 private:

  void initialize();

  UVM_FILE get_severity_id_file( uvm_severity severity,
                                 const std::string& id );

  void set_verbosity_level( int verbosity_level );

  void set_severity_action( uvm_severity severity,
                            uvm_action action );

  void set_id_action( const std::string& id,
                      uvm_action action );

  void set_severity_id_action( uvm_severity severity,
                               const std::string& id,
                               uvm_action action );

  void set_id_verbosity( const std::string& id,
                         int verbosity );

  void set_severity_id_verbosity( uvm_severity severity,
                                  const std::string& id,
                                  int verbosity );

  void set_default_file( UVM_FILE file );

  void set_severity_file( uvm_severity severity, UVM_FILE file );

  void set_id_file( const std::string& id, UVM_FILE file );

  void set_severity_id_file( uvm_severity severity,
                             const std::string& id,
                             UVM_FILE file );

  void set_severity_override( uvm_severity cur_severity,
                              uvm_severity new_severity );

  void set_severity_id_override( uvm_severity cur_severity,
                                 const std::string& id,
                                 uvm_severity new_severity );

  void dump_state();

  // internal variables

  int m_max_verbosity_level;

  typedef std::map<uvm_severity, uvm_action> severity_actions_mapt;
  severity_actions_mapt severity_actions;
  typedef severity_actions_mapt::const_iterator severity_actions_mapcitt;

  uvm_id_actions_array id_actions;
  typedef uvm_id_actions_array::const_iterator id_actions_mapcitt;

  typedef std::map<uvm_severity, uvm_id_actions_array> severity_id_actions_mapt;
  typedef severity_id_actions_mapt::const_iterator severity_id_actions_mapcitt;
  severity_id_actions_mapt severity_id_actions;

  // id verbosity settings : default and severity
  uvm_id_verbosities_array id_verbosities;
  typedef uvm_id_verbosities_array::const_iterator id_verbosities_mapcitt;

  typedef std::map<uvm_severity, uvm_id_verbosities_array> severity_id_verbosities_mapt;
  typedef severity_id_verbosities_mapt::const_iterator severity_id_verbosities_mapcitt;
  severity_id_verbosities_mapt severity_id_verbosities;

  // severity overrides
  uvm_sev_override_array sev_overrides;
  typedef uvm_sev_override_array::const_iterator sev_overrides_mapcitt;

  typedef std::map<std::string, uvm_sev_override_array> sev_id_overrides_mapt;
  typedef sev_id_overrides_mapt::const_iterator sev_id_overrides_mapcitt;
  sev_id_overrides_mapt sev_id_overrides;

  // file handles : default, severity, action, (severity,id)
  UVM_FILE default_file_handle;

  typedef std::map<uvm_severity, UVM_FILE> severity_file_handles_mapt;
  typedef severity_file_handles_mapt::const_iterator severity_file_handles_mapcitt;
  severity_file_handles_mapt severity_file_handles;

  uvm_id_file_array id_file_handles;
  typedef uvm_id_file_array::const_iterator id_file_handles_mapcitt;

  typedef std::map<uvm_severity, uvm_id_file_array> severity_id_file_handles_mapt;
  typedef severity_id_file_handles_mapt::const_iterator severity_id_file_handles_mapcitt;
  severity_id_file_handles_mapt severity_id_file_handles;

}; // class uvm_report_handler

} // namespace uvm


#endif // UVM_REPORT_HANDLER_H_

