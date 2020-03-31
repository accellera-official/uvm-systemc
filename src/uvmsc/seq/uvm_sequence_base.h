//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2012-2019 NXP B.V.
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

#ifndef UVM_SEQUENCE_BASE_H_
#define UVM_SEQUENCE_BASE_H_

#include <systemc>
#include <iostream>
#include <list>

#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/factory/uvm_factory.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/dap/uvm_get_to_lock_dap.h"

namespace uvm {

// forward declaration of necessary classes.
class uvm_sequencer_base;

//------------------------------------------------------------------------------
// CLASS: uvm_sequence_base
//
//! The class #uvm_sequence_base shall provide the interfaces needed to create streams
//! of sequence items and/or other sequences.
//------------------------------------------------------------------------------ 

class uvm_sequence_base: public uvm_sequence_item
{
  friend class uvm_sequencer_base;
  template <typename REQ, typename RSP> friend class uvm_sequencer;
  template <typename REQ, typename RSP> friend class uvm_sequencer_param_base;
  friend class uvm_reg_map;

 public:
  explicit uvm_sequence_base( uvm_object_name name_ );
  virtual ~uvm_sequence_base();

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  virtual bool is_item() const;

  uvm_sequence_state_enum get_sequence_state() const;

  void wait_for_sequence_state( unsigned int state_mask );

  //--------------------------------------------------------------------------
  // Group: Sequence execution
  //--------------------------------------------------------------------------

  virtual void start( uvm_sequencer_base* sqr,
                                            uvm_sequence_base* parent_sequence = NULL,
                                            int this_priority = -1,
                                            bool call_pre_post = true );
  virtual void pre_start();
  virtual void pre_body();
  virtual void pre_do( bool is_item );
  virtual void mid_do( uvm_sequence_item* this_item );
  virtual void body();
  virtual void post_do( uvm_sequence_item* this_item );
  virtual void post_body();
  virtual void post_start();

  //--------------------------------------------------------------------------
  // Group: Run-time phasing
  //--------------------------------------------------------------------------

  uvm_phase* get_starting_phase() const;
  void set_starting_phase( uvm_phase* phase );
  bool get_automatic_phase_objection() const;
  void set_automatic_phase_objection( bool value );

  //--------------------------------------------------------------------------
  // Group: Sequence control
  //--------------------------------------------------------------------------

  void set_priority ( int value );
  int get_priority() const;
  virtual bool is_relevant() const;
  virtual void wait_for_relevant() const;
  void lock( uvm_sequencer_base* sequencer = NULL );
  void grab( uvm_sequencer_base* sequencer = NULL );
  void unlock( uvm_sequencer_base* sequencer = NULL );
  void ungrab( uvm_sequencer_base* sequencer = NULL );
  bool is_blocked() const;
  bool has_lock();
  void kill();
  virtual void do_kill();

  //--------------------------------------------------------------------------
  // Group: Sequence item execution
  //--------------------------------------------------------------------------

  uvm_sequence_item* create_item( uvm_object_wrapper* type_var,
                                  uvm_sequencer_base* l_sequencer,
                                  const std::string& name );

  virtual void start_item( uvm_sequence_item* item,
                           int set_priority = -1,
                           uvm_sequencer_base* sequencer = NULL );

  virtual void finish_item( uvm_sequence_item* item,
                            int set_priority = -1 );

  virtual void wait_for_grant( int item_priority = -1,
                               bool lock_request = false );

  virtual void send_request( uvm_sequence_item* request,
                             bool rerandomize = false );

  virtual void wait_for_item_done( int transaction_id = -1 );

  //--------------------------------------------------------------------------
  // Group: Response API
  //--------------------------------------------------------------------------

  void use_response_handler( bool enable );
  bool get_use_response_handler() const;
  virtual void response_handler( const uvm_sequence_item* response );
  void set_response_queue_error_report_disabled( bool value );
  bool get_response_queue_error_report_disabled() const;
  void set_response_queue_depth( int value );
  int get_response_queue_depth() const;
  virtual void clear_response_queue();

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

protected:

  virtual void put_response ( const uvm_sequence_item& response );
  virtual void put_base_response( const uvm_sequence_item& response );
  virtual uvm_sequence_item* get_base_response( int transaction_id = -1 );
  virtual void del_base_response( uvm_sequence_item* response );

private:
  void m_start_core( uvm_sequence_base* parent_sequence, bool call_pre_post );

  int m_get_sqr_sequence_id( int sequencer_id, bool update_sequence_id );
  void m_set_sqr_sequence_id( int sequencer_id, int sequence_id );

  void m_clear();
  void m_kill();
  //  void m_copy(); // TODO

  void m_safe_raise_starting_phase( std::string description = "",
                                    int count = 1 );
  void m_safe_drop_starting_phase( std::string description = "",
                                    int count = 1 );
  void m_init_phase_daps( bool create );

  void m_clear_phase_daps();

  // TODO do we need our own constructor?
  //uvm_sequence_base& uvm_sequence_base( const uvm_sequence_base& obj );

  /////////////////////

  // data members
public:
  sc_core::sc_event response_queue_event;
  //uvm_phase* starting_phase; //OLD-remove

private:
  int m_wait_for_grant_semaphore;
  int m_next_transaction_id;

  // Each sequencer will assign a sequence id.  When a sequence is talking to multiple
  // sequencers, each sequence_id is managed separately
  std::map<int,int> m_sqr_seq_ids;

  typedef std::map<uvm_sequence_base*, bool> children_array_mapT;
  typedef children_array_mapT::iterator children_array_ItT;
  children_array_mapT children_array;

  uvm_sequence_state_enum m_sequence_state;
  sc_core::sc_event m_sequence_state_ev;

  sc_core::sc_process_handle m_sequence_process;

  int m_priority;
  bool m_use_response_handler;
  int response_queue_depth;
  bool response_queue_error_report_disabled;

  typedef std::list<uvm_sequence_item*> response_queue_listT;

  response_queue_listT response_queue;

  int m_tr_handle;

  mutable bool is_rel_default;
  mutable bool wait_rel_default;

  // Value set via set_starting_phase
  mutable uvm_phase* m_set_starting_phase;
  // Ensures we only warn once per sequence
  mutable bool m_warn_deprecated_set;

  // Automatic Phase Objection DAP
  uvm_get_to_lock_dap<bool>* m_automatic_phase_objection_dap;
  // Starting Phase DAP
  uvm_get_to_lock_dap<uvm_phase*>* m_starting_phase_dap;

  bool old_automatic_phase_objection;

};

} /* namespace uvm */

#endif /* UVM_SEQUENCE_BASE_H_ */
