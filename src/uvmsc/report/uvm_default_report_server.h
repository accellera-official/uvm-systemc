//----------------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2014 NVIDIA Corporation
//   Copyright 2016 NXP B.V.
//   Copyright 2018 Intel Corp.
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
//----------------------------------------------------------------------------

#ifndef UVM_DEFAULT_REPORT_SERVER_H_
#define UVM_DEFAULT_REPORT_SERVER_H_

#include <map>
#include <string>
#include <vector>

#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/report/uvm_report_server.h"

namespace uvm {

// forward class declarations
//class uvm_tr_stream;
class uvm_object_server;
class uvm_report_message;

//----------------------------------------------------------------------------
// CLASS: uvm_default_report_server
//
// Default implementation of the UVM report server.
//----------------------------------------------------------------------------

class uvm_default_report_server : public uvm_report_server
{
 public:

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  uvm_default_report_server( const std::string& name = "uvm_report_server" );

  void do_print( const uvm_printer& printer ) const override;

  //--------------------------------------------------------------------------
  // Group: Quit Count
  //--------------------------------------------------------------------------

  int get_max_quit_count() const override;

  void set_max_quit_count( int count, bool overridable = true ) override;

  int get_quit_count() const override;

  void set_quit_count( int quit_count ) override;

  void incr_quit_count();

  void reset_quit_count();

  bool is_quit_count_reached();

  //--------------------------------------------------------------------------
  // Group: Severity Count
  //--------------------------------------------------------------------------

  int get_severity_count( uvm_severity severity ) const override;

  void set_severity_count( uvm_severity severity, int count ) override;

  void incr_severity_count( uvm_severity severity );

  void reset_severity_counts();

  //--------------------------------------------------------------------------
  // Group: id Count
  //--------------------------------------------------------------------------

  int get_id_count( const std::string& id ) const override;

  void set_id_count( const std::string& id, int count ) override;

  void incr_id_count( const std::string& id );

  //--------------------------------------------------------------------------
  // Group: Message recording
  //--------------------------------------------------------------------------

//  virtual void set_message_database( uvm_tr_database* database );

//  virtual uvm_tr_database* get_message_database() const;

  void get_severity_set( std::vector<uvm_severity>& q ) const override;

  void get_id_set( std::vector<std::string>& q ) const override;

  //----------------------------------------------------------------------------
  // Group: Message processing
  //----------------------------------------------------------------------------

  void process_report_message( uvm_report_message* report_message ) override;

  void execute_report_message( uvm_report_message* report_message,
                                       const std::string& composed_message ) override;

  std::string compose_report_message( uvm_report_message* report_message,
                                              const std::string& report_object_name = "") const override;

  void report_summarize( UVM_FILE file = 0 ) const override;

  // variables in UVM 1.2 class definition - TODO make methods

  bool enable_report_id_count_summary;
  bool record_all_messages;
  bool show_verbosity;
  bool show_terminator;

  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////

  void f_display( UVM_FILE file, const std::string& str ) const;



  const std::string get_type_name() const override;

 private:
  // local data members

  int m_quit_count;
  int m_max_quit_count;

  bool max_quit_overridable;

  typedef std::map<uvm_severity, int> severity_count_map;
  mutable severity_count_map m_severity_count;
  typedef severity_count_map::iterator severity_count_itt;
  typedef severity_count_map::const_iterator severity_count_citt;

  typedef std::map<std::string, int> id_count_map;
  mutable id_count_map m_id_count;
  typedef id_count_map::iterator id_count_itt;
  typedef id_count_map::const_iterator id_count_citt;

  // TODO transaction database
  //uvm_tr_database* m_message_db;
  //uvm_tr_stream* m_streams[string][string]; // ro.name,rh.name
  //std::map<std::string, std::map<std::string, uvm_tr_stream*> > m_streams;

}; // class uvm_default_report_server

} // namespace uvm

#endif // UVM_DEFAULT_REPORT_SERVER_H_

