//----------------------------------------------------------------------
//   Copyright 2014 Université Pierre et Marie Curie, Paris
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2020 Synopsys, Inc.
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

#include <systemc>

#include "uvmsc/base/uvm_component.h"
#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/factory/uvm_object_wrapper.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/seq/uvm_sequencer.h"
#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/conf/uvm_config_db.h"
#include "uvmsc/print/uvm_printer.h"

namespace uvm {

// TODO - replace by proven RNG
unsigned int urandom_range( unsigned int max, unsigned int min)
{
  if (min == max) return min;
  unsigned int i = (unsigned int)(rand() % (max - min) + min + 1);
  return i;
}

//------------------------------------------------------------------------------
// static data member initialization
//------------------------------------------------------------------------------

int uvm_sequencer_base::g_sequencer_id = 1;
int uvm_sequencer_base::g_sequence_id = 1;
int uvm_sequencer_base::g_request_id = 0;

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

uvm_sequencer_base::uvm_sequencer_base( uvm_component_name name_ )
  : uvm_component( name_ )
{
  m_sequencer_id = g_sequencer_id++;
  m_arbitration = SEQ_ARB_FIFO;

  seq_req_t_str[SEQ_TYPE_REQ] = "SEQ_TYPE_REQ";
  seq_req_t_str[SEQ_TYPE_LOCK] = "SEQ_TYPE_LOCK";
  seq_req_t_str[SEQ_TYPE_GRAB] = "SEQ_TYPE_GRAB";

  m_wait_for_item_sequence_id = -1;
  m_wait_for_item_transaction_id = -1;

  arb_sequence_q.clear();
  lock_list.clear();

  m_lock_arb_size = -1;
  m_arb_size = -1;

  srand(25); // TODO make global function to specify seed for RNG

  m_current_sequence_item = NULL;
}

//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

uvm_sequencer_base::~uvm_sequencer_base()
{
  for( arb_sequence_q_vectorT::iterator
       it = arb_sequence_q.begin();
       it != arb_sequence_q.end();
       it++ )
    delete *it;

  for( lock_vectorT::iterator
       it = lock_list.begin();
       it != lock_list.end();
       it++ )
    delete *it;

  // now all dynamic objects are cleared, we can clear the list itself
  arb_sequence_q.clear();
  lock_list.clear();
}

//----------------------------------------------------------------------
// member function: is_child
//
//! Returns true if the child sequence is a child of the parent sequence
//! and false otherwise.
//----------------------------------------------------------------------

bool uvm_sequencer_base::is_child( uvm_sequence_base* parent,
                                   const uvm_sequence_base* child ) const
{
  uvm_sequence_base* child_parent;

  if (child == NULL)
    uvm_report_fatal("uvm_sequencer", "is_child passed NULL child", UVM_NONE);

  if (parent == NULL)
    uvm_report_fatal("uvm_sequencer", "is_child passed NULL parent", UVM_NONE);

  child_parent = child->get_parent_sequence();

  while (child_parent != NULL) {
    if (child_parent->get_inst_id() == parent->get_inst_id())
      return true;
    child_parent = child_parent->get_parent_sequence();
  }
  return false;
}

//----------------------------------------------------------------------
// member function: user_priority_arbitration (virtual)
//
//! When the sequencer arbitration mode is set to SEQ_ARB_USER (via the
//! #set_arbitration method), the sequencer will call this function each
//! time that it needs to arbitrate among sequences.
//!
//! Derived sequencers may override this method to perform a custom arbitration
//! policy. The override must return one of the entries from the
//! \p avail_sequences queue, which are indexes into an internal queue,
//! arb_sequence_q.
//! The default implementation behaves like SEQ_ARB_FIFO, which returns the
//! entry at avail_sequences[0].
//----------------------------------------------------------------------

int uvm_sequencer_base::user_priority_arbitration( std::vector<int> avail_sequences )
{
  return avail_sequences[0];
}

//----------------------------------------------------------------------
// member function: execute_item (virtual)
//
//! Executes the given transaction item directly on this sequencer. A temporary
//! parent sequence is automatically created for the item.  There is no capability to
//! retrieve responses. If the driver returns responses, they will accumulate in the
//! sequencer, eventually causing response overflow unless
//! uvm_sequence_base::set_response_queue_error_report_disabled is called.
//----------------------------------------------------------------------

void uvm_sequencer_base::execute_item( uvm_sequence_item* item )
{
  uvm_sequence_base* seq = new uvm_sequence_base(sc_core::sc_gen_unique_name("parent_seq"));
  item->set_sequencer(this);
  item->set_parent_sequence(seq);
  seq->set_sequencer(this);
  seq->start_item(item);
  seq->finish_item(item);
  // TODO check if we need to add a conditional seq->get_response(rsp);
}

//----------------------------------------------------------------------
// member function: start_phase_sequence
//
//! Start the default sequence for this phase, if any.
//! The default sequence is configured via resources using
//! either a sequence instance or sequence type (object wrapper).
//! If both are used, the sequence instance takes precedence.
//! When attempting to override a previous default sequence setting,
//! you must override both the instance and type (wrapper) resources,
//! else your override may not take effect.
//----------------------------------------------------------------------

void uvm_sequencer_base::start_phase_sequence( uvm_phase& phase )
{
  uvm_object_wrapper* wrapper = NULL;
  uvm_sequence_base* seq = NULL;

  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* f = cs->get_factory();

  // default sequence instance?
  if (!uvm_config_db<uvm_sequence_base*>::get(
    this, phase.get_name()+"_phase", "default_sequence", seq) || seq == NULL)
  {
    // default sequence object wrapper?

    if (uvm_config_db<uvm_object_wrapper*>::get(
          this, phase.get_name() + "_phase", "default_sequence", wrapper) && wrapper != NULL)
    {
      // use wrapper is a sequence type
      seq =  dynamic_cast<uvm_sequence_base*>
        (f->create_object_by_type(wrapper, get_full_name(), wrapper->get_type_name() ) );
      if(seq == NULL)
      {
        std::ostringstream msg;
        msg << "Default sequence for phase '"
            <<  phase.get_name()
            << "' %s is not a sequence type.";
        uvm_report_warning("PHASESEQ", msg.str() );
        return;
      }
    }

    else
    {
      std::ostringstream msg;
      msg << "No default phase sequence for phase '"
          << phase.get_name() << "' found for " << get_full_name();
      UVM_INFO("PHASESEQ", msg.str(), UVM_FULL);
      return;
    }
  }

  std::ostringstream msg;
  msg << "Starting default sequence '"
      << seq->get_type_name()
      << "' for phase '"
      << phase.get_name()
      << "' for sequencer " << get_full_name();
  uvm_report_info("PHASESEQ", msg.str(), UVM_FULL);

  seq->print_sequence_info = true;
  seq->set_sequencer(this);
  //seq.reseed(); // TODO: randomization
  seq->set_starting_phase(&phase); //NEW!

  /* TODO randomize
  if (!seq->do_not_randomize && !seq->randomize())
  {
    std::ostringstream msg;
    msg << "Randomization failed for default sequence '"
        << seq->get_type_name()
        << "' for phase '" <<  phase.get_name() << "'";
    uvm_report_warning("STRDEFSEQ", msg.str());
    return;
  }
  */

  // launch default sequence as new process
  sc_core::sc_spawn(sc_bind(&uvm_sequencer_base::m_start_default_seq_proc, this, seq));
}

//----------------------------------------------------------------------
// member function: wait_for_grant
//
//! This member function issues a request for the specified sequence.  If
//! item_priority is not specified, then the current sequence priority will
//! be used by the arbiter.  If a #lock_request is made, then the sequencer
//! will issue a lock immediately before granting the sequence.
//! (Note that the lock may be granted without the sequence being granted
//! if is_relevant is not asserted).
//!
//! When this member function returns, the sequencer has granted the sequence,
//! and the sequence must call send_request without inserting any simulation
//! delay other than delta cycles.  The driver is currently waiting for the
//! next item to be sent via the #send_request call.
//----------------------------------------------------------------------

void uvm_sequencer_base::wait_for_grant(uvm_sequence_base* sequence_ptr,
                                        int item_priority,
                                        bool lock_request)
{
  uvm_sequence_request* req_s;
  int my_seq_id;

  if (sequence_ptr == NULL)
    uvm_report_fatal("uvm_sequencer_base",
       "wait_for_grant passed NULL sequence_ptr", UVM_NONE);

  my_seq_id = m_register_sequence(sequence_ptr);

  // If lock_request is asserted, then issue a lock.  Don't wait for the response, since
  // there is a request immediately following the lock request
  if (lock_request)
  {
    req_s = new uvm_sequence_request();
    req_s->grant = false;
    req_s->sequence_id = my_seq_id;
    req_s->request = SEQ_TYPE_LOCK;
    req_s->sequence_ptr = sequence_ptr;
    req_s->request_id = g_request_id++;
    arb_sequence_q.push_back(req_s);
  }

  // Push the request onto the queue
  req_s = new uvm_sequence_request();
  req_s->grant = false;
  req_s->request = SEQ_TYPE_REQ;
  req_s->sequence_id = my_seq_id;
  req_s->item_priority = item_priority;
  req_s->sequence_ptr = sequence_ptr;
  req_s->request_id = g_request_id++;
  arb_sequence_q.push_back(req_s);
  m_update_lists();

  // Wait until this entry is granted
  // Continue to point to the element, since location in queue will change
  m_wait_for_arbitration_completed(req_s->request_id);

  // The wait_for_grant_semaphore is used only to check that send_request
  // is only called after wait_for_grant.  This is not a complete check, since
  // requests might be done in parallel, but it will catch basic errors
  req_s->sequence_ptr->m_wait_for_grant_semaphore++;
}

//----------------------------------------------------------------------
// member function: wait_for_item_done
//
//! A sequence may optionally call #wait_for_item_done. This member function will block
//! until the driver calls item_done() or put() on a transaction issued by the
//! specified sequence.  If no \p transaction_id parameter is specified, then the
//! call will return the next time that the driver calls item_done() or put().
//! If a specific transaction_id is specified, then the call will only return
//! when the driver indicates that it has completed that specific item.
//!
//! Note that if a specific transaction_id has been specified, and the driver
//! has already issued an item_done or put for that transaction, then the call
//! will hang waiting for that specific \p transaction_id.
//----------------------------------------------------------------------

void uvm_sequencer_base::wait_for_item_done( uvm_sequence_base* sequence_ptr,
                                             int transaction_id )
{
  int sequence_id;

  sequence_id = sequence_ptr->m_get_sqr_sequence_id(m_sequencer_id, true);

  m_wait_for_item_sequence_id = -1;
  m_wait_for_item_transaction_id = -1;

  if (transaction_id == -1)
  {
    do
     sc_core::wait (m_wait_for_item_sequence_ev);
    while(m_wait_for_item_sequence_id != sequence_id );
  }
  else
  {
    do
     sc_core::wait (m_wait_for_item_sequence_ev);
    while(!(m_wait_for_item_sequence_id == sequence_id &&
            m_wait_for_item_transaction_id == transaction_id));

  }
}

//----------------------------------------------------------------------
// member function: is_blocked
//
//! Returns true if the sequence referred to by sequence_ptr is currently locked
//! out of the sequencer.  It will return false if the sequence is currently
//! allowed to issue operations.
//!
//! Note that even when a sequence is not blocked, it is possible for another
//! sequence to issue a lock before this sequence is able to issue a request
//! or lock.
//----------------------------------------------------------------------

bool uvm_sequencer_base::is_blocked( const uvm_sequence_base* sequence_ptr ) const
{
  if (sequence_ptr == NULL)
    uvm_report_fatal("uvm_sequence_controller",
                     "is_blocked passed NULL sequence_ptr", UVM_NONE);

  for (unsigned int i =0; i < lock_list.size(); i++)
  {
    if ((lock_list[i]->get_inst_id() !=
         sequence_ptr->get_inst_id()) &&
        (is_child(lock_list[i], sequence_ptr) == 0))
      return true;
  }

  return false;
}

//----------------------------------------------------------------------
// member function: has_lock
//
//! Returns true if the sequence referred to in the parameter currently has a lock
//! on this sequencer, and false otherwise.
//!
//! Note that even if this sequence has a lock, a child sequence may also have
//! a lock, in which case the sequence is still blocked from issueing
//! operations on the sequencer
//----------------------------------------------------------------------

bool uvm_sequencer_base::has_lock( uvm_sequence_base* sequence_ptr )
{
  if (sequence_ptr == NULL)
    uvm_report_fatal("uvm_sequence_controller",
                     "has_lock passed null sequence_ptr", UVM_NONE);

  m_register_sequence(sequence_ptr);

  for(unsigned int i = 0; i < lock_list.size(); i++)
  {
    if (lock_list[i]->get_inst_id() == sequence_ptr->get_inst_id())
      return true;
  }
  return false;
}


//----------------------------------------------------------------------
// member function: lock (virtual)
//
//! Requests a lock for the sequence specified by argument \p sequence_ptr.
//!
//! A lock request will be arbitrated the same as any other request. A lock is
//! granted after all earlier requests are completed and no other locks or
//! grabs are blocking this sequence.
//!
//! The lock call will return when the lock has been granted.
//----------------------------------------------------------------------

void uvm_sequencer_base::lock( uvm_sequence_base* sequence_ptr )
{
  m_lock_req(sequence_ptr, true);
}

//----------------------------------------------------------------------
// member function: grab (virtual)
//
//! Requests a grab for the sequence specified by argument \p sequence_ptr.
//!
//! A grab request is put in front of the arbitration queue. It will be
//! arbitrated before any other requests. A grab is granted when no other
//! grabs or locks are blocking this sequence.
//!
//! The grab call will return when the grab has been granted.
//----------------------------------------------------------------------

void uvm_sequencer_base::grab( uvm_sequence_base* sequence_ptr )
{
  m_lock_req(sequence_ptr, false);
}

//----------------------------------------------------------------------
// member function: unlock (virtual)
//
//! Removes any locks and grabs obtained by the specified argument \p sequence_ptr.
//----------------------------------------------------------------------

void uvm_sequencer_base::unlock( uvm_sequence_base* sequence_ptr )
{
  m_unlock_req(sequence_ptr);
}

//----------------------------------------------------------------------
// member function: ungrab (virtual)
//
//! Removes any locks and grabs obtained by the specified argument \p sequence_ptr.
//----------------------------------------------------------------------

void uvm_sequencer_base::ungrab( uvm_sequence_base* sequence_ptr )
{
  m_unlock_req(sequence_ptr);
}

//----------------------------------------------------------------------
// member function: stop_sequences (virtual)
//
//! The member function stop_sequences shall tell the sequencer to kill
//! all sequences and child sequences currently operating on the sequencer,
//! and remove all requests, locks and responses that are currently queued.
//! This essentially resets the sequencer to an idle state.
//----------------------------------------------------------------------

void uvm_sequencer_base::stop_sequences()
{
  uvm_sequence_base* seq_ptr;

  seq_ptr = m_find_sequence(-1);
  while (seq_ptr != NULL)
  {
    m_kill_sequence(seq_ptr);
    seq_ptr = m_find_sequence(-1);
  }
}

//----------------------------------------------------------------------
// member function: is_grabbed (virtual)
//
//! Returns true if any sequence currently has a lock or grab on this sequencer,
//! false otherwise.
//----------------------------------------------------------------------

bool uvm_sequencer_base::is_grabbed() const
{
  return (lock_list.size() != 0);
}

//----------------------------------------------------------------------
// member function: current_grabber (virtual)
//
//! Returns a reference to the sequence that currently has a lock or grab on
//! the sequence.  If multiple hierarchical sequences have a lock, it returns
//! the child that is currently allowed to perform operations on the sequencer.
//----------------------------------------------------------------------

uvm_sequence_base* uvm_sequencer_base::current_grabber() const
{
  if (lock_list.size() == 0)
    return NULL;

  return lock_list[lock_list.size()-1];
}

//----------------------------------------------------------------------
// member function: has_do_available (virtual)
//
//! The member function has_do_available shall return true if any sequence
//! running on this sequencer is ready to supply a transaction, otherwise
//! it shall return false
//----------------------------------------------------------------------

bool uvm_sequencer_base::has_do_available()
{
  for (unsigned int i = 0; i < arb_sequence_q.size(); i++)
  {
    if ( (arb_sequence_q[i]->sequence_ptr->is_relevant()) &&
         (!is_blocked(arb_sequence_q[i]->sequence_ptr) )
       )
      return true;
  }
  return false;
}

//----------------------------------------------------------------------
// member function: set_arbitration
//
//! Specifies the arbitration mode for the sequencer. It is one of
//!
//! SEQ_ARB_FIFO          - Requests are granted in FIFO order (default)
//! SEQ_ARB_WEIGHTED      - Requests are granted randomly by weight
//! SEQ_ARB_RANDOM        - Requests are granted randomly
//! SEQ_ARB_STRICT_FIFO   - Requests at highest priority granted in fifo order
//! SEQ_ARB_STRICT_RANDOM - Requests at highest priority granted in randomly
//! SEQ_ARB_USER          - Arbitration is delegated to the user-defined
//!                         function, user_priority_arbitration. That function
//!                         will specify the next sequence to grant.
//!
//! The default user function specifies FIFO order.
//----------------------------------------------------------------------

void uvm_sequencer_base::set_arbitration(SEQ_ARB_TYPE val)
{
  m_arbitration = val;
}

//----------------------------------------------------------------------
// member function: get_arbitration
//
//! Return the current arbitration mode set for this sequencer. See
//! member function #set_arbitration for a list of possible modes.
//----------------------------------------------------------------------

SEQ_ARB_TYPE uvm_sequencer_base::get_arbitration() const
{
  return m_arbitration;
}

//----------------------------------------------------------------------
// member function: wait_for_sequences
//
//! Waits for a sequence to have a new item available. Uses
//! #uvm_wait_for_nba_region to give a sequence as much time as
//! possible to deliver an item before advancing time.
//----------------------------------------------------------------------

void uvm_sequencer_base::wait_for_sequences() const
{
  uvm_wait_for_nba_region();
}

//----------------------------------------------------------------------
// member function: send_request (virtual)
//
//! Derived classes implement this function to send a request item to the
//! sequencer, which will forward it to the driver.  If the rerandomize bit
//! is set, the item will be randomized before being sent to the driver.
//!
//! This member function may only be called after a #wait_for_grant call.
//----------------------------------------------------------------------

void uvm_sequencer_base::send_request(uvm_sequence_base* sequence_ptr,
                                      uvm_sequence_item* seq_item,
                                      bool rerandomize)
{
  // virtual member function, will be overloaded
}

//----------------------------------------------------------------------
// member function: build_phase
//----------------------------------------------------------------------

void uvm_sequencer_base::build_phase(uvm_phase& phase)
{
  uvm_component::build_phase(phase);
}


////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: m_register_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

int uvm_sequencer_base::m_register_sequence(uvm_sequence_base* sequence_ptr)
{
  if (sequence_ptr->m_get_sqr_sequence_id(m_sequencer_id, 1) > 0)
    return sequence_ptr->get_sequence_id();

  sequence_ptr->m_set_sqr_sequence_id(m_sequencer_id, g_sequence_id++);

  reg_sequences[sequence_ptr->get_sequence_id()] = sequence_ptr;

  return sequence_ptr->get_sequence_id();
}

//----------------------------------------------------------------------
// member function: m_update_lists
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_update_lists()
{
  m_lock_arb_size++;
  m_lock_arb_size_changed.notify();
}



//----------------------------------------------------------------------
// member function: m_get_seq_item_priority
//
// Implementation-defined member function
//----------------------------------------------------------------------

int uvm_sequencer_base::m_get_seq_item_priority(uvm_sequence_request* seq_q_entry)
{
  // If the priority was set on the item, then that is used
  if (seq_q_entry->item_priority != -1)
  {
    if (seq_q_entry->item_priority <= 0)
    {
      std::ostringstream str;
      str << "Sequence item from "
          << seq_q_entry->sequence_ptr->get_full_name()
          << " has illegal priority: "
          << seq_q_entry->item_priority;
      uvm_report_fatal("SEQITEMPRI", str.str(), UVM_NONE);
    }
    return seq_q_entry->item_priority;
  }

  // Otherwise, use the priority of the calling sequence
  if (seq_q_entry->sequence_ptr->get_priority() < 0)
  {
    std::ostringstream str;
    str << "Sequence " << seq_q_entry->sequence_ptr->get_full_name()
        << " has illegal priority: "
        << seq_q_entry->sequence_ptr->get_priority();
    uvm_report_fatal("SEQDEFPRI", str.str(), UVM_NONE);
  }

  return seq_q_entry->sequence_ptr->get_priority();
}

//----------------------------------------------------------------------
// member function: m_wait_for_arbitration_completed
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_wait_for_arbitration_completed(int request_id)
{
  int lock_arb_size = 0;

  // Search the list of arb_wait_q, see if this item is done
  for(;;) // check forever
  {
    if (arb_completed.find(request_id) != arb_completed.end()) // exists
    {
      lock_arb_size = m_lock_arb_size;
      arb_completed.erase(request_id); // delete
      return;
    }

    do {  // was wait(lock_arb_size != m_lock_arb_size);
      sc_core::wait  (m_lock_arb_size_changed);
    }
    while(lock_arb_size == m_lock_arb_size);
  }

}

//----------------------------------------------------------------------
// member function: m_sequence_exiting
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_sequence_exiting(uvm_sequence_base* sequence_ptr)
{
  remove_sequence_from_queues(sequence_ptr);
}

//----------------------------------------------------------------------
// member function: remove_sequence_from_queues
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::remove_sequence_from_queues(uvm_sequence_base* sequence_ptr)
{
  int seq_id;
  unsigned int i;

  seq_id = sequence_ptr->m_get_sqr_sequence_id(m_sequencer_id, false);

  // Remove all queued items for this sequence and any child sequences
  i = 0;
  do
  {
    if (arb_sequence_q.size() > i)
    {
      if ((arb_sequence_q[i]->sequence_id == seq_id) ||
          (is_child(sequence_ptr, arb_sequence_q[i]->sequence_ptr)))
      {
        if (sequence_ptr->get_sequence_state() == UVM_FINISHED)
        {
          std::ostringstream str;
          str << "Parent sequence '" << sequence_ptr->get_full_name()
              << "' should not finish before all items from itself and items "
              << "from descendent sequences are processed."
              << std::endl
              << "The item request from the sequence '"
              << arb_sequence_q[i]->sequence_ptr->get_full_name()
              << "' is being removed.";
          UVM_ERROR("SEQFINERR", str.str() );
        }
        arb_sequence_q.erase(arb_sequence_q.begin()+i);
        m_update_lists();
      }
      else
        i++;
    }
  }
  while (i < arb_sequence_q.size());

  // remove locks for this sequence, and any child sequences
  i = 0;
  do
  {
    if (lock_list.size() > i)
    {
      if ((lock_list[i]->get_inst_id() == sequence_ptr->get_inst_id()) ||
          (is_child(sequence_ptr, lock_list[i])))
      {
        if (sequence_ptr->get_sequence_state() == UVM_FINISHED)
        {
          std::ostringstream str;
          str << "Parent sequence '"
              << sequence_ptr->get_full_name()
              << "' should not finish before locks from itself and "
              << "descedent sequences are removed."
              << std::endl
              << "The lock held by the child sequence '"
              << lock_list[i]->get_full_name()
              << "' is being removed.";
          UVM_ERROR("SEQFINERR", str.str() );
        }
        lock_list.erase(lock_list.begin()+i);
        m_update_lists();
      }
      else
        i++;
    }
  }
  while (i < lock_list.size() );

  // Unregister the sequence_id, so that any returning data is dropped
  m_unregister_sequence(sequence_ptr->m_get_sqr_sequence_id(m_sequencer_id, true));
}

//----------------------------------------------------------------------
// member function: m_unregister_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_unregister_sequence(int sequence_id)
{
  if (reg_sequences.find(sequence_id) == reg_sequences.end()) // not exists
    return;

  reg_sequences.erase(sequence_id); // otherwise delete item
}

//----------------------------------------------------------------------
// member function: grant_queued_locks
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::grant_queued_locks()
{
  unsigned int i = 0;
  int temp = 0;

  while (i < arb_sequence_q.size())
  {
    // Check for lock requests.  Any lock request at the head
    // of the queue that is not blocked will be granted immediately.
    temp = 0;
    if (i < arb_sequence_q.size())
    {
      if (arb_sequence_q[i]->request == SEQ_TYPE_LOCK)
      {
         /* TODO grant_queued_locks
         if ((arb_sequence_q[i].process_id.status == process::KILLED) ||
             (arb_sequence_q[i].process_id.status == process::FINISHED))
         {
            std::ostringstream str;
            str << "The task responsible for requesting a lock on sequencer '"
                << this->get_full_name()
                << "' for sequence '"
                << arb_sequence_q[i]->sequence_ptr->get_full_name()
                << "' has been killed, to avoid a deadlock the sequence will be removed from the arbitration queues.";
            UVM_ERROR("SEQLCKZMB", str.str() );
            remove_sequence_from_queues(arb_sequence_q[i]->sequence_ptr);
            continue;
         }
         */
         temp = (is_blocked(arb_sequence_q[i]->sequence_ptr) == 0);
      }
    }

    // Grant the lock request and remove it from the queue.
    // This is a loop to handle multiple back-to-back locks.
    // Since each entry is deleted, i remains constant
    while (temp)
    {
      lock_list.push_back(arb_sequence_q[i]->sequence_ptr);
      m_set_arbitration_completed(arb_sequence_q[i]->request_id);
      arb_sequence_q.erase(arb_sequence_q.begin()+i);
      m_update_lists();

      temp = 0;
      if (i < arb_sequence_q.size())
      {
        if (arb_sequence_q[i]->request == SEQ_TYPE_LOCK)
          temp = is_blocked(arb_sequence_q[i]->sequence_ptr) == 0;
      }
    }

    i++;
  } // while

}

//----------------------------------------------------------------------
// member function: m_select_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_select_sequence()
{
  int selected_sequence;

  // Select a sequence
  do
  {
    wait_for_sequences();
    selected_sequence = m_choose_next_request();
    if (selected_sequence == -1)
      m_wait_for_available_sequence();
  }
  while (selected_sequence == -1);

  // issue grant
  if (selected_sequence >= 0)
  {
    m_set_arbitration_completed(arb_sequence_q[selected_sequence]->request_id);
    arb_sequence_q.erase(arb_sequence_q.begin() + selected_sequence);
    m_update_lists();
  }
}

//----------------------------------------------------------------------
// member function: m_set_arbitration_completed
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_set_arbitration_completed(int request_id)
{
  arb_completed[request_id] = true;
}

//----------------------------------------------------------------------
// member function: m_choose_next_request
//
// Implementation-defined member function
//----------------------------------------------------------------------

int uvm_sequencer_base::m_choose_next_request()
{
  int temp = 0;
  int sum_priority_val = 0;
  std::vector<int> avail_sequences;
  std::vector<int> highest_sequences;
  int highest_pri = 0;
  std::string s;
  unsigned int i = 0;

  avail_sequences.clear();
  highest_sequences.clear();

  grant_queued_locks();

  while ( i < arb_sequence_q.size() )
  {
  /* TODO introduce process / phasing
    if (((*it)->process_id.status() == uvm_process::KILLED) ||
        ((*it)->process_id.status() == uvm_process::FINISHED))
    {
      std::ostringstream str;
      str << " The task responsible for requesting a wait_for_grant on sequencer '"
          <<  this->get_full_name() << "' for sequence '"
          << (*it)->sequence_ptr->get_full_name()
          << "' has been killed, to avoid a deadlock the sequence will be removed from the arbitration queues.";
      UVM_ERROR("SEQREQZMB", str.str());
      remove_sequence_from_queues((*it)->sequence_ptr);
      continue;
    } */

    if ( i < arb_sequence_q.size() )
      if (arb_sequence_q[i]->request == SEQ_TYPE_REQ)
        if (!is_blocked(arb_sequence_q[i]->sequence_ptr))
          if (arb_sequence_q[i]->sequence_ptr->is_relevant())
          {
            if (m_arbitration == SEQ_ARB_FIFO)
              return i;
            else avail_sequences.push_back(i);
          }
    i++;
  }

  // Return immediately if there are 0 or 1 available sequences
  if (m_arbitration == SEQ_ARB_FIFO)
    return -1;

  if (avail_sequences.size() < 1)
    return -1;

  if (avail_sequences.size() == 1)
      return avail_sequences[0]; // return first element

  // If any locks are in place, then the available queue must
  // be checked to see if a lock prevents any sequence from proceeding
  if (lock_list.size() > 0)
  {
    for (i = 0; i < avail_sequences.size(); i++)
    {
      if (is_blocked(arb_sequence_q[avail_sequences[i]]->sequence_ptr))
      {
        avail_sequences.erase(avail_sequences.begin()+i);
        i--;
      }
    }
    if (avail_sequences.size() < 1)
      return -1;
    if (avail_sequences.size() == 1)
      return avail_sequences[0];
  }

  // Weighted Priority Distribution
  // Pick an available sequence based on weighted priorities of available sequences
  if (m_arbitration == SEQ_ARB_WEIGHTED)
  {
    sum_priority_val = 0;

    for (i = 0; i < avail_sequences.size(); i++)
      sum_priority_val += m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]);

    temp = urandom_range(sum_priority_val-1, 0);

    sum_priority_val = 0;

    for (i = 0; i < avail_sequences.size(); i++)
    {
      if( (m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]) +
           sum_priority_val) > temp )
        return avail_sequences[i];
      sum_priority_val += m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]);
    }
    uvm_report_fatal("Sequencer", "UVM Internal error in weighted arbitration code", UVM_NONE);
  }

  //  Random Distribution
  if (m_arbitration == SEQ_ARB_RANDOM)
  {
    i = urandom_range((unsigned int)avail_sequences.size()-1, 0);
    return avail_sequences[i];
  }

  //  Strict Fifo
  if ((m_arbitration == SEQ_ARB_STRICT_FIFO) || (m_arbitration == SEQ_ARB_STRICT_RANDOM))
  {
    highest_pri = 0;

    // Build a list of sequences at the highest priority
    for (i = 0; i < avail_sequences.size(); i++)
    {
      if (m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]) > highest_pri)
      {
        // New highest priority, so start new list
        highest_sequences.clear();
        highest_sequences.push_back(avail_sequences[i]);
        highest_pri = m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]);
      }
      else
        if (m_get_seq_item_priority(arb_sequence_q[avail_sequences[i]]) == highest_pri)
          highest_sequences.push_back(avail_sequences[i]);
    }

    // Now choose one based on arbitration type
    if (m_arbitration == SEQ_ARB_STRICT_FIFO)
      return(highest_sequences[0]);

    i = urandom_range((unsigned int)highest_sequences.size()-1, 0); // TODO improve random function (see above)
    return highest_sequences[i];
  }

  if (m_arbitration == SEQ_ARB_USER)
  {
    i = user_priority_arbitration( avail_sequences );

    // Check that the returned sequence is in the list of available sequences.  Failure to
    // use an available sequence will cause highly unpredictable results.

    // highest_sequences = avail_sequences.find with (item == i);
    for( unsigned int j = 0; j < avail_sequences.size(); j++)
    {
      if (i == (unsigned int)avail_sequences[j])
        highest_sequences.push_back(j);
    }

    if (highest_sequences.size() == 0)
    {
      std::ostringstream str;
      str << "Error in User arbitration, sequence "
          << i << " not available " << std::endl << convert2string() << std::endl;
      uvm_report_fatal("Sequencer", str.str(), UVM_NONE);
    }
    return i;
  }

  uvm_report_fatal("Sequencer", "Internal error: Failed to choose sequence", UVM_NONE);

  return -1;
}


//----------------------------------------------------------------------
// member function: m_wait_for_available_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_wait_for_available_sequence()
{
  unsigned int i;
  std::list<int> is_relevant_entries;

  // This routine will wait for a change in the request list, or for
  // wait_for_relevant to return on any non-relevant, non-blocked sequence
  m_arb_size = m_lock_arb_size;

  for (i = 0; i < arb_sequence_q.size(); i++)
  {
    if (arb_sequence_q[i]->request == SEQ_TYPE_REQ)
    {
      if (!is_blocked(arb_sequence_q[i]->sequence_ptr))
      {
        if (!arb_sequence_q[i]->sequence_ptr->is_relevant())
          is_relevant_entries.push_back(i);
      }
    }
  }

  // Typical path - don't need fork if all queued entries are relevant
  if (is_relevant_entries.size() == 0)
  {
    m_wait_arb_not_equal();
    return;
  }

  // TODO - selection in case not all queued entries are relevant...

  std::cout << "UVM-SystemC FATAL ERROR Occurred" << std::endl;
  std::cout << "Please contact uvm-systemc-feedback@lists.accellera.org" << std::endl;
  exit(-1);
}

//----------------------------------------------------------------------
// member function: m_wait_arb_not_equal
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_wait_arb_not_equal()
{
  do
  { // SV: wait(m_arb_size != m_lock_arb_size) wait(0) halts
    sc_core::wait (m_lock_arb_size_changed);
  }
  while (m_arb_size == m_lock_arb_size);
}

//----------------------------------------------------------------------
// member function: m_find_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

uvm_sequence_base* uvm_sequencer_base::m_find_sequence(int sequence_id)
{
  // When sequence_id is -1, return the first available sequence.  This is used
  // when deleting all sequences
  if (sequence_id == -1)
  {
    if (reg_sequences.begin()->first) // when valid sequence_id
      return reg_sequences.begin()->second; // return uvm_sequence_base
    else return NULL;
  }

  //in all other cases
  return reg_sequences[sequence_id];
}

//----------------------------------------------------------------------
// member function: m_kill_sequence
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::m_kill_sequence( uvm_sequence_base* sequence_ptr )
{
  remove_sequence_from_queues(sequence_ptr);
  sequence_ptr->m_kill();
}

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequencer_base::do_print( const uvm_printer& printer ) const
{
  uvm_component::do_print(printer);
  printer.print_array_header("arbitration_queue", (int)arb_sequence_q.size());

  unsigned int i = 0;

  for (i = 0; i<arb_sequence_q.size(); i++)
  {
    std::ostringstream str1, str2;
    str1 << "[" << i << "]" << std::endl;
    str2 << seq_req_t_str[arb_sequence_q[i]->request] << "@seqid" // TODO request name
         << arb_sequence_q[i]->sequence_id << "[";
    printer.print_string(str1.str(), str2.str());
  }
  printer.print_array_footer( (int)arb_sequence_q.size() );

  printer.print_array_header("lock_queue", (int)lock_list.size());

  for (i = 0; i < lock_list.size(); i++)
  {
    std::ostringstream str1, str2;
    str1 << "[" << i << "]" << std::endl;
    str2 << lock_list[i]->get_full_name() << "@seqid"
         << lock_list[i]->get_sequence_id() << "[";
    printer.print_string(str1.str(), str2.str());
  }
  printer.print_array_footer( (int)lock_list.size() );
}


//----------------------------------------------------------------------
// member function: analysis_write
//
// Implementation defined.
//----------------------------------------------------------------------

void uvm_sequencer_base::analysis_write(uvm_sequence_item t)
{
  return;
}

//----------------------------------------------------------------------
// member function: m_lock_req
//
// Implementation defined.
// Called by a sequence to request a lock.
// Puts the lock request onto the arbitration queue.
//----------------------------------------------------------------------

void uvm_sequencer_base::m_lock_req( uvm_sequence_base* sequence_ptr, bool lock)
{
  uvm_sequence_request* new_req;

  if (sequence_ptr == NULL)
    uvm_report_fatal("uvm_sequence_controller",
                     "lock_req passed null sequence_ptr", UVM_NONE);

  m_register_sequence(sequence_ptr);

  new_req = new uvm_sequence_request(); // TODO delete at some stage...
  new_req->grant = 0;
  new_req->sequence_id = sequence_ptr->get_sequence_id();
  new_req->request = SEQ_TYPE_LOCK;
  new_req->sequence_ptr = sequence_ptr;
  new_req->request_id = g_request_id++;
  //new_req->process_id = process::self(); // TODO

  if (lock == true)
  {
    // Locks are arbitrated just like all other requests
    arb_sequence_q.push_back(new_req);
  }
  else
  {
    // Grabs are not arbitrated - they go to the front
    // UVMSV- TODO:
    // Missing: grabs get arbitrated behind other grabs
    arb_sequence_q.insert(arb_sequence_q.begin(), new_req);
    m_update_lists();
  }

  // If this lock can be granted immediately, then do so.
  grant_queued_locks();

  m_wait_for_arbitration_completed(new_req->request_id);
}

//----------------------------------------------------------------------
// member function: m_unlock_req
//
// Implementation defined
// Called by a sequence to request an unlock.  This
// will remove a lock for this sequence if it exists
//----------------------------------------------------------------------

void uvm_sequencer_base::m_unlock_req( uvm_sequence_base* sequence_ptr )
{
  if (sequence_ptr == NULL)
  {
    uvm_report_fatal("uvm_sequencer",
                     "m_unlock_req passed null sequence_ptr", UVM_NONE);
  }

  m_register_sequence(sequence_ptr);

  for(unsigned int i = 0; i < lock_list.size(); i++)
  {
    if (lock_list[i]->get_inst_id() == sequence_ptr->get_inst_id())
    {
      lock_list.erase(lock_list.begin()+i);
      m_update_lists();
      return;
    }
  }
  std::ostringstream str;
  str << "Sequence '" << sequence_ptr->get_full_name()
      << "' called ungrab / unlock, but didn't have lock";
  uvm_report_warning("SQRUNL", str.str(), UVM_NONE);
}

//----------------------------------------------------------------------
// member function: m_start_default_seq_proc
//
// Implementation defined
// Start default sequence as forked process
//----------------------------------------------------------------------

void uvm_sequencer_base::m_start_default_seq_proc(uvm_sequence_base* seq)
{
  seq->start(this, NULL);
}


} /* namespace uvm */
