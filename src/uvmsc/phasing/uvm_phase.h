//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#ifndef UVM_PHASE_H_
#define UVM_PHASE_H_

#include <map>
#include <list>
#include <vector>

#include <systemc>
#include "sysc/kernel/sc_dynamic_processes.h"

#include "uvmsc/base/uvm_object.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/phasing/uvm_phase_queue.h"

//////////////

namespace uvm {

// forward declaration of necessary classes
class uvm_object;
class uvm_objection;
class uvm_domain;
class uvm_component;

//------------------------------------------------------------------------------
// CLASS: uvm_phase
//
//! This base class defines everything about a phase: behavior, state, and context.
//------------------------------------------------------------------------------

class uvm_phase : public uvm_object
{
 public:
  friend class uvm_process_phase;
  friend class uvm_topdown_phase;
  friend class uvm_bottomup_phase;
  friend class uvm_domain;
  friend class uvm_root;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------
  // Group: Construction
  //--------------------

  explicit uvm_phase( const std::string& name,
                      uvm_phase_type phase_type = UVM_PHASE_SCHEDULE,
                      uvm_phase* parent = nullptr );

  uvm_phase_type get_phase_type() const;

  //-------------
  // Group: State
  //-------------

  uvm_phase_state get_state() const;

  int get_run_count() const;

  uvm_phase* find_by_name( const std::string& name,
                           bool stay_in_scope = true ) const;

  uvm_phase* find( const uvm_phase* phase,
                   bool stay_in_scope = true ) const;

  bool is( const uvm_phase* phase ) const;

  bool is_before( const uvm_phase* phase ) const;

  bool is_after( const uvm_phase* phase ) const;

  //-----------------
  // Group: Callbacks
  //-----------------

  virtual void exec_func( uvm_component* comp, uvm_phase* phase );

  // note - this was the exec_task in UVM-SV
  virtual void exec_process( uvm_component* comp, uvm_phase* phase );

  //----------------
  // Group: Schedule
  //----------------

  void add( uvm_phase* phase,
            uvm_phase* with_phase = nullptr,
            uvm_phase* after_phase = nullptr,
            uvm_phase* before_phase = nullptr );

  uvm_phase* get_parent() const;

  const std::string get_full_name() const override;

  uvm_phase* get_schedule( bool hier = false ) const;

  std::string get_schedule_name( bool hier = false ) const;

  uvm_domain* get_domain() const;

  std::string get_domain_name() const;

  uvm_phase* get_imp() const;

  //-----------------------
  // Group: Synchronization
  //-----------------------

  uvm_objection* get_objection() const;

  virtual void raise_objection ( uvm_object* obj,
                                 const std::string& description = "",
                                 int count = 1 );

  virtual void drop_objection ( uvm_object* obj,
                                const std::string& description = "",
                                int count = 1 );

  void sync( uvm_domain& target,
             uvm_phase* phase = nullptr,
             uvm_phase* with_phase = nullptr );

  void unsync( uvm_domain& target,
               uvm_phase* phase = nullptr,
               uvm_phase* with_phase = nullptr );

  void wait_for_state( uvm_phase_state state, uvm_wait_op op = UVM_EQ );

  //---------------
  // Group: Jumping
  //---------------

  void jump( const uvm_phase* phase );

  uvm_phase* get_jump_target() const;


  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  virtual void execute( uvm_component* comp,
                        uvm_phase* phase );

  virtual void kill( uvm_component* comp,
                     uvm_phase* phase );

  virtual void traverse( uvm_component* comp,
                         uvm_phase* phase,
                         uvm_phase_state state );

  void execute_phase( bool proc );

  void m_master_phase_process( uvm_phase* process_phase );

  void m_wait_for_jump();

  void m_wait_for_all_dropped();

  void m_wait_for_timeout();

  const uvm_phase* m_find_predecessor( const uvm_phase* phase,
                                       bool stay_in_scope = true,
                                       const uvm_phase* orig_phase = nullptr ) const;

  const uvm_phase* m_find_predecessor_by_name( const std::string& name,
                                               bool stay_in_scope = true,
                                               const uvm_phase* orig_phase = nullptr ) const;

  const uvm_phase* m_find_successor( const uvm_phase* phase,
                                     bool stay_in_scope = true,
                                     const uvm_phase* orig_phase = nullptr ) const;

  const uvm_phase* m_find_successor_by_name( const std::string& name,
                                             bool stay_in_scope = true,
                                             const uvm_phase* orig_phase = nullptr ) const;

  static void m_register_phases();

  static void m_prerun_phases();

  static void m_run_phases();

  static void m_run_single_phase( const std::string& phase_name );

  static void m_run_phases_process();

  void wait_for_self_and_siblings_to_drop();

  void clear_phase( uvm_phase_state state = UVM_PHASE_DORMANT );

  void clear_successors( uvm_phase_state state = UVM_PHASE_DORMANT,
                         uvm_phase* end_state = nullptr );

  void get_predecessors_for_successors( std::map<uvm_phase*, bool>& pred_of_succ );

  void m_wait_for_pred();

  void m_get_transitive_children( std::vector<uvm_phase*>& phases );

  bool m_is_domain();

  static std::map<uvm_phase*, bool>& m_executing_phases();

  static uvm_phase* m_phase_nodes(const std::string& name, uvm_phase* phase = nullptr);

  const std::string get_full_name_under() const;

  //--------------------------
  // data members below
  //--------------------------

 private:

  // Construction
  //------------------------------
  uvm_phase_type m_phase_type;
  uvm_phase* m_parent;  // our 'schedule' node [or points 'up' one level]
  uvm_phase* m_imp;     // phase imp to call when we execute this node

  typedef std::map<uvm_phase*, uvm_phase*> m_nodesmapT;
  typedef m_nodesmapT::iterator m_nodesmapItT;
  m_nodesmapT m_nodes; // register # nodes

  unsigned int max_ready_to_end_iter;

  // Schedule
  //--------------------------
  uvm_phase* m_end_node;
  typedef std::map<uvm_phase*, bool> m_schedulemapT;
  typedef m_schedulemapT::iterator m_schedulemapItT;
  typedef m_schedulemapT::const_iterator m_schedulemapcItT;
  m_schedulemapT m_predecessors;
  m_schedulemapT m_successors;

  // Synchronization
  //---------------------------------
  typedef std::vector<uvm_phase*> phase_listT;
  typedef phase_listT::iterator phase_listItT;
  phase_listT m_sync;  // schedule instance to which we are synced

  uvm_objection* phase_done;
  unsigned int m_ready_to_end_count;

  // State
  //---------------------------------
  int m_num_procs_not_yet_returned;
  uvm_phase_state m_state;
  sc_core::sc_event m_state_ev;
  int m_run_count;

  // Overall Control
  //---------------------------------
  static uvm_phase_queue<uvm_phase*>* m_phase_hopper;

  // Jumping
  //-------------------------
  bool m_jump_bkwd;
  sc_core::sc_event m_jump_bkwd_ev;
  bool m_jump_fwd;
  sc_core::sc_event m_jump_fwd_ev;
  uvm_phase* m_jump_phase;

  // Timeout and objection detection
  //--------------------------------
  sc_core::sc_event _forever; //dummy event which should never gets triggered

  // TBD add more useful debug
  //---------------------------------
  // TODO remove public access
 public:
  static bool m_phase_trace;
  bool m_use_ovm_run_semantic;

}; // class uvm_phase

//////////////

} // namespace uvm

#endif /* UVM_PHASE_H_ */
