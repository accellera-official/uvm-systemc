//----------------------------------------------------------------------
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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
//----------------------------------------------------------------------

#ifndef UVM_COMPONENT_H_
#define UVM_COMPONENT_H_

#include <iostream>
#include <map>
#include <list>
#include <systemc>
#include "uvmsc/report/uvm_report_object.h"

//////////////

namespace uvm {

// forward declaration of necessary classes.
class uvm_object;
class uvm_phase;
class uvm_domain;
class uvm_transaction;
class uvm_config_object_wrapper;
class uvm_component_name;
class uvm_objection;
class uvm_coreservice_t;

//------------------------------------------------------------------------------
// CLASS: uvm_component
//
//! Base class for structural UVM elements.
//! Derives from sc_module, and provides name, and hierarchy information.
//------------------------------------------------------------------------------

class uvm_component : public sc_core::sc_module,
                      public uvm_report_object
{
 public:
  friend class uvm_config_mgr;
  friend class uvm_topdown_phase;
  friend class uvm_bottomup_phase;
  friend class uvm_build_phase;
  friend class uvm_connect_phase;
  friend class uvm_end_of_elaboration_phase;
  friend class uvm_start_of_simulation_phase;
  friend class uvm_run_phase;
  friend class uvm_extract_phase;
  friend class uvm_check_phase;
  friend class uvm_report_phase;
  friend class uvm_final_phase;
  friend class uvm_pre_reset_phase;
  friend class uvm_reset_phase;
  friend class uvm_post_reset_phase;
  friend class uvm_pre_configure_phase;
  friend class uvm_configure_phase;
  friend class uvm_post_configure_phase;
  friend class uvm_pre_main_phase;
  friend class uvm_main_phase;
  friend class uvm_post_main_phase;
  friend class uvm_pre_shutdown_phase;
  friend class uvm_shutdown_phase;
  friend class uvm_post_shutdown_phase;
  friend class uvm_process_phase;
  friend class uvm_sequencer_base;
  friend class uvm_report_object;

  typedef uvm_component this_type;

  explicit uvm_component( uvm_component_name nm );

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // Group: Hierarchy Interface
  //----------------------------------------------------------------------------

  virtual uvm_component* get_parent() const;

  virtual const std::string get_full_name() const;

  void get_children( std::vector<uvm_component*>& children ) const;

  uvm_component* get_child( const std::string& name ) const;

  int get_next_child( std::string& name ) const;

  int get_first_child ( std::string& name ) const;

  int get_num_children() const;

  bool has_child( const std::string& name ) const;

  uvm_component* lookup( const std::string& name ) const;

  unsigned int get_depth() const;

  //----------------------------------------------------------------------------
  // Group: Phasing Interface
  //----------------------------------------------------------------------------

  virtual void build_phase( uvm_phase& phase );

  virtual void connect_phase( uvm_phase& phase );

  virtual void end_of_elaboration_phase( uvm_phase& phase );

  virtual void start_of_simulation_phase( uvm_phase& phase );

  virtual void run_phase( uvm_phase& phase );

  virtual void pre_reset_phase( uvm_phase& phase );

  virtual void reset_phase( uvm_phase& phase );

  virtual void post_reset_phase( uvm_phase& phase );

  virtual void pre_configure_phase( uvm_phase& phase );

  virtual void configure_phase( uvm_phase& phase );

  virtual void post_configure_phase( uvm_phase& phase );

  virtual void pre_main_phase( uvm_phase& phase );

  virtual void main_phase( uvm_phase& phase );

  virtual void post_main_phase( uvm_phase& phase );

  virtual void pre_shutdown_phase( uvm_phase& phase );

  virtual void shutdown_phase( uvm_phase& phase );

  virtual void post_shutdown_phase( uvm_phase& phase );

  virtual void extract_phase( uvm_phase& phase );

  virtual void check_phase( uvm_phase& phase );

  virtual void report_phase( uvm_phase& phase );

  virtual void final_phase( uvm_phase& phase );

  virtual void phase_started( uvm_phase& phase );

  virtual void phase_ready_to_end( uvm_phase& phase );

  virtual void phase_ended( uvm_phase& phase );

  //--------------------------------------------------------------------
  // Group: phase / schedule / domain API
  //--------------------------------------------------------------------

  void set_domain( uvm_domain& domain, int hier = 1 );

  uvm_domain* get_domain() const;

  void define_domain(  uvm_domain& domain );

  void set_phase_imp( uvm_phase* phase, uvm_phase* imp, int hier = 1 );

  virtual bool suspend(); // TODO: UVM standard defines it should return void

  virtual bool resume(); // TODO: UVM standard defines it should return void

  //----------------------------------------------------------------------------
  // Group: Configuration interface
  //----------------------------------------------------------------------------

  void check_config_usage( bool recurse = true );

  virtual void apply_config_settings( bool verbose = false );

  void print_config( bool recurse = false, bool audit = false ) const;

  void print_config_with_audit( bool recurse = false ) const;

  void print_config_matches( bool enable = true );

  //--------------------------------------------------------------------------
  // Group: Objection Interface
  //--------------------------------------------------------------------------

  virtual void raised( uvm_objection* objection,
                       uvm_object* source_obj,
                       const std::string& description,
                       int count );

  virtual void dropped( uvm_objection* objection,
                        uvm_object* source_obj,
                        const std::string& description,
                        int count );

  virtual void all_dropped( uvm_objection* objection,
                            uvm_object* source_obj,
                            const std::string& description,
                            int count );

  //--------------------------------------------------------------------------
  // Group: Factory Interface
  //--------------------------------------------------------------------------

  uvm_component* create_component( const std::string& requested_type_name,
                                   const std::string& name );

  uvm_object* create_object( const std::string& requested_type_name,
                             const std::string& name );

  static void set_type_override_by_type( uvm_object_wrapper* original_type,
                                         uvm_object_wrapper* override_type,
                                         bool replace = true );

  void set_inst_override_by_type( const std::string& relative_inst_path,
                                  uvm_object_wrapper* original_type,
                                  uvm_object_wrapper* override_type );

  static void set_type_override( const std::string& original_type_name,
                                 const std::string& override_type_name,
                                 bool replace = true );

  void set_inst_override( const std::string& relative_inst_path,
                          const std::string& original_type_name,
                          const std::string& override_type_name );

  void print_override_info( const std::string& requested_type_name = "",
                            const std::string& name = "");

  //--------------------------------------------------------------------------
  // Group: Hierarchical Reporting Interface
  //--------------------------------------------------------------------------

  void set_report_id_verbosity_hier( const std::string& id, int verbosity );

  void set_report_severity_id_verbosity_hier( uvm_severity severity,
                                              const std::string& id,
                                              int verbosity);

  void set_report_severity_action_hier( uvm_severity severity,
                                        uvm_action action );

  void set_report_id_action_hier( const std::string& id,
                                  uvm_action action );

  void set_report_severity_id_action_hier( uvm_severity severity,
                                           const std::string& id,
                                           uvm_action action );

  void set_report_default_file_hier( UVM_FILE file );

  void set_report_severity_file_hier( uvm_severity severity,
                                      UVM_FILE file );

  void set_report_id_file_hier( const std::string& id,
                                UVM_FILE file );

  void set_report_severity_id_file_hier( uvm_severity severity,
                                         const std::string& id,
                                         UVM_FILE file );

  void set_report_verbosity_level_hier( int verbosity );

  virtual void pre_abort();


  //--------------------------------------------------------------------------
  // Group: Recording Interface
  //--------------------------------------------------------------------------

  void accept_tr( const uvm_transaction& tr,
                  const sc_core::sc_time& accept_time = sc_core::SC_ZERO_TIME );

  // TODO protected? See UVM standard
  virtual void do_accept_tr( const uvm_transaction& tr );

  int begin_tr( const uvm_transaction& tr,
                const std::string& stream_name = "main",
                const std::string& label = "",
                const std::string& desc = "",
                const sc_core::sc_time& begin_time = sc_core::SC_ZERO_TIME,
                int parent_handle = 0 );

  int begin_child_tr( const uvm_transaction& tr,
                      int parent_handle = 0,
                      const std::string& stream_name = "main",
                      const std::string& label = "",
                      const std::string& desc = "",
                      const sc_core::sc_time& begin_time = sc_core::SC_ZERO_TIME );

  // TODO protected? See UVM standard
  virtual void do_begin_tr( const uvm_transaction& tr,
                            const std::string& stream_name,
                            int tr_handle );

  void end_tr( const uvm_transaction& tr,
               const sc_core::sc_time& end_time = sc_core::SC_ZERO_TIME,
               bool free_handle = true );

  // TODO protected? See UVM standard
  virtual void do_end_tr( const uvm_transaction& tr,
                          int tr_handle );

  int record_error_tr( const std::string& stream_name = "main",
                       uvm_object* info = NULL,
                       const std::string& label = "error_tr",
                       const std::string& desc = "",
                       const sc_core::sc_time& error_time = sc_core::SC_ZERO_TIME,
                       bool keep_active = false );

  int record_event_tr( const std::string& stream_name = "main",
                       uvm_object* info = NULL,
                       const std::string& label = "event_tr",
                       const std::string& desc = "",
                       const sc_core::sc_time& event_time = sc_core::SC_ZERO_TIME,
                       bool keep_active = false );

  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////
  /////////////////////////////////////////////////////

  virtual const std::string get_type_name() const;

  virtual ~uvm_component();

  // SystemC compatibility
  virtual const char* kind() const;

  // avoid ambiguity with sc_object::print
  using uvm_report_object::print;

 private:
#if IEEE_1666_SYSTEMC >= 202301L
  // allow hierarchy manipulation
  using sc_core::sc_object::get_hierarchy_scope;
#endif

  void print_config_settings( const std::string& field = "",
                              uvm_component* comp = NULL,
                              bool recurse = false);

  virtual void set_name( const std::string& name );

  virtual void do_print( const uvm_printer& printer ) const;

  void m_set_full_name();

  void m_extract_name( const std::string& name,
                       std::string& leaf,
                       std::string& remainder ) const;

  //----------------------------------------------------------------------------
  // 'legacy' from SystemC - recommended not to use
  //----------------------------------------------------------------------------

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();
  virtual void start_of_simulation();
  virtual void end_of_simulation();

  void m_apply_verbosity_settings(uvm_phase* phase);

 protected:

  void m_do_pre_abort();

  void m_set_run_handle(sc_core::sc_process_handle h);

  sc_core::sc_process_handle m_run_handle;

  uvm_component* m_comp_parent;


 private:
  std::string prepend_name( const std::string& s );

  bool m_add_child(uvm_component* child, const std::string& name = "");

  unsigned int recording_detail;
  //-------------
  // data members
  //-------------
  static bool global_timeout_spawned_;

 public:

  typedef std::map< std::string, uvm_component*> m_children_mapT;
  typedef m_children_mapT::const_iterator m_children_mapcItT;

  m_children_mapT m_children;

  mutable unsigned int m_children_cnt;

  std::string m_name;

  std::map< uvm_component*, uvm_component*> m_children_by_handle;

  //--------------------------------------------------------------------------
  // member variable: print_config_matches
  //
  // Setting this static variable causes get_config_* to print info about
  // matching configuration settings as they are being applied.
  //
  // TODO: prefixed with underscore since we made this a method in UVM-SystemC
  //--------------------------------------------------------------------------

  static bool _print_config_matches;

  //--------------------------------------------------------------------------
  // member variable: print_enabled
  //
  // This flag determines if this component should automatically be printed as a
  // child of its parent object.
  //
  // By default, all children are printed. However, this flag allows a parent
  // component to disable the printing of specific children.
  //--------------------------------------------------------------------------

  bool print_enabled;

  //--------------------------------------------------------------------------
  // member variable: recorder
  //
  // Specifies the <uvm_recorder> object to use for <begin_tr> and other
  // methods in the <Recording Interface>. Default is <uvm_default_recorder>.
  //--------------------------------------------------------------------------
  // TODO uvm_recorder
  //uvm_recorder recorder;

 public:
  //------------------------------
  // Internal phasing data members
  //------------------------------
  uvm_phase* m_current_phase;  // the most recently executed phase
  std::map<uvm_phase*,uvm_phase*> m_phase_imps;

 public:
  int  m_phasing_active;
  bool m_build_done;

 protected:
  uvm_domain* m_domain;         // set_domain stores our domain handle

  uvm_phase* m_schedule; // temporary schedule

  uvm_domain* m_common; //common domain

  // keep track of dynamically added elements
  std::list<uvm_phase*> m_schedule_list;
  std::list<uvm_config_object_wrapper*> m_config_object_wrapper_list;
};

//----------------------------------------------------------------------------
// Class: uvm_config_object_wrapper
//
//! Implementation-defined class to store the cloned object along with
//! the original object. Used for set_config_object(...) member function
//----------------------------------------------------------------------------

class uvm_config_object_wrapper
{
 public:
   uvm_object* obj;
   bool clone;

};

/////////////////////////////////////////////


} // namespace uvm

#endif /* UVM_COMPONENT_H_ */
