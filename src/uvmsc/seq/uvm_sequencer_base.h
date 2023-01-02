//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2012-2022 NXP B.V.
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

#ifndef UVM_SEQUENCER_BASE_H_
#define UVM_SEQUENCER_BASE_H_


#include <list>
#include <vector>

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_globals.h"
#include "uvmsc/phasing/uvm_process.h"
#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/seq/uvm_sequence_item.h"


namespace uvm {


// forward declaration
class uvm_sequence_request;
class uvm_sequence_base;
class uvm_sequence_item;
class uvm_phase;

//------------------------------------------------------------------------------
// CLASS: uvm_sequencer_base
//
//! Controls the flow of sequences, which generate the stimulus (#sequence item
//! transactions) that is passed on to drivers for execution.
//------------------------------------------------------------------------------

class uvm_sequencer_base : public uvm_component
{
  friend class uvm_reg_map;
  friend class uvm_reg;
  template <typename BASE> friend class uvm_req_sequence;
  friend class uvm_reg_indirect_data;
  template <typename REQ, typename RSP> friend class uvm_sequencer;
  template <typename REQ, typename RSP> friend class uvm_sequencer_param_base;
  friend class uvm_sequence_base;
  template <typename RSP>
  friend class uvm_sequencer_analysis_fifo;

public:

  typedef enum { SEQ_TYPE_REQ,
                 SEQ_TYPE_LOCK,
                 SEQ_TYPE_GRAB} seq_req_t;

  explicit uvm_sequencer_base( uvm_component_name name_ );
  virtual ~uvm_sequencer_base();

  bool is_child( uvm_sequence_base* parent, const uvm_sequence_base* child ) const;

  virtual int user_priority_arbitration(std::vector<int> avail_sequences);

  virtual void execute_item( uvm_sequence_item* item );

  virtual void start_phase_sequence( uvm_phase& phase );

  virtual void wait_for_grant( uvm_sequence_base* sequence_ptr,
                               int item_priority = -1,
                               bool lock_request = false);

  virtual void wait_for_item_done( uvm_sequence_base* sequence_ptr,
                                   int transaction_id = -1 );

  bool is_blocked( const uvm_sequence_base* sequence_ptr ) const;

  bool has_lock( uvm_sequence_base* sequence_ptr );
  virtual void lock( uvm_sequence_base* sequence_ptr );
  virtual void grab( uvm_sequence_base* sequence_ptr );
  virtual void unlock( uvm_sequence_base* sequence_ptr );
  virtual void ungrab( uvm_sequence_base* sequence_ptr );
  virtual void stop_sequences();
  virtual bool is_grabbed() const;
  virtual uvm_sequence_base* current_grabber() const;
  virtual bool has_do_available();

  void set_arbitration( SEQ_ARB_TYPE val );

  SEQ_ARB_TYPE get_arbitration() const;

  virtual void wait_for_sequences() const;

  virtual void send_request( uvm_sequence_base* sequence_ptr,
                             uvm_sequence_item* seq_item,
                             bool rerandomize = false);

  virtual void build_phase( uvm_phase& phase );

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual const char* kind() const; // SystemC API

  virtual const std::string get_type_name() const;

 private:

  int m_register_sequence(uvm_sequence_base* sequence_ptr);
  void m_update_lists();
  int m_get_seq_item_priority(uvm_sequence_request* seq_q_entry);
  void m_wait_for_arbitration_completed(int request_id);
  void m_sequence_exiting(uvm_sequence_base* sequence_ptr);
  void remove_sequence_from_queues(uvm_sequence_base* sequence_ptr);
  void m_unregister_sequence(int sequence_id);
  void grant_queued_locks();
  void m_select_sequence();
  int m_choose_next_request();
  void m_set_arbitration_completed(int request_id);
  void m_wait_for_available_sequence();
  void m_wait_arb_not_equal();
  uvm_sequence_base* m_find_sequence(int sequence_id);
  void m_kill_sequence( uvm_sequence_base* sequence_ptr );
  virtual void do_print( const uvm_printer& printer ) const;

  virtual void analysis_write(uvm_sequence_item t);

  void m_lock_req( uvm_sequence_base* sequence_ptr, bool lock );
  void m_unlock_req( uvm_sequence_base* sequence_ptr );

  void m_start_default_seq_proc(uvm_sequence_base* seq);

  // member variables

  int m_wait_for_item_sequence_id;
  int m_wait_for_item_transaction_id;
  sc_core::sc_event m_wait_for_item_sequence_ev;

  mutable std::map<seq_req_t, std::string> seq_req_t_str;

  int m_lock_arb_size;
  int m_arb_size;

 protected:
  int m_sequencer_id;
  std::map<int, bool> arb_completed;
  std::map<int, uvm_sequence_base*> reg_sequences;

  uvm_sequencer_arb_mode  m_arbitration;
  sc_core::sc_event m_lock_arb_size_changed;

  typedef std::vector<uvm_sequence_request*> arb_sequence_q_vectorT;
  arb_sequence_q_vectorT arb_sequence_q;

  typedef std::vector<uvm_sequence_base*> lock_vectorT;
  lock_vectorT lock_list;

  static int g_sequencer_id;
  static int g_sequence_id;
  static int g_request_id;

  uvm_sequence_item* m_current_sequence_item;
};

//------------------------------------------------------------------------------
// CLASS: uvm_sequence_request
//
// Helper class for the registration of sequence requests
//------------------------------------------------------------------------------

class uvm_sequence_request
{
 public:
  bool grant;
  int sequence_id;
  int request_id;
  int item_priority;
  uvm_process process_id;
  uvm_sequencer_base::seq_req_t request;
  uvm_sequence_base* sequence_ptr;
};

/////////////////////////////////////////////
/////////////////////////////////////////////

// SystemC API
inline const char* uvm_sequencer_base::kind() const
{
  return "uvm::uvm_sequencer_base";
}

inline const std::string uvm_sequencer_base::get_type_name() const
{
  return std::string(kind());
}

} /* namespace uvm */

#endif /* UVM_SEQUENCER_BASE_H_ */
