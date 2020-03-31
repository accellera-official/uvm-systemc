//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#include <sstream>
#include <systemc>
#include "sysc/kernel/sc_dynamic_processes.h"

#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/report/uvm_report_object.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/base/uvm_coreservice_t.h"
#include "uvmsc/base/uvm_default_coreservice_t.h"
#include "uvmsc/factory/uvm_factory.h"

using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

uvm_sequence_base::uvm_sequence_base( uvm_object_name name_ )
 : uvm_sequence_item( name_ ), m_sequence_state(UVM_CREATED)
{
  m_wait_for_grant_semaphore = 0;
  m_next_transaction_id = 1;

  //starting_phase = NULL;
  m_sqr_seq_ids.clear();

  m_priority = -1;
  m_use_response_handler = false;
  response_queue_depth = -1; // no limit on depth
  response_queue_error_report_disabled = false;

  response_queue.clear();
  m_tr_handle = 0;

  is_rel_default = true;
  wait_rel_default = false;

  m_automatic_phase_objection_dap = NULL;
  m_starting_phase_dap = NULL;
  m_init_phase_daps(true);
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

uvm_sequence_base::~uvm_sequence_base()
{
  m_clear();
}

//----------------------------------------------------------------------
// member function: is_item (virtual)
//
//! Returns true on items and false on sequences. As this object is a sequence,
//! #is_item will always return false.
//----------------------------------------------------------------------

bool uvm_sequence_base::is_item() const
{
  return false;
}

//----------------------------------------------------------------------
// member function: get_sequence_state
//
//! Returns the sequence state as an enumerated value. Can use to wait on
//! the sequence reaching or changing from one or more states.
//----------------------------------------------------------------------

uvm_sequence_state_enum uvm_sequence_base::get_sequence_state() const
{
  return m_sequence_state;
}

//----------------------------------------------------------------------
// member function: wait_for_sequence_state
//
//! Waits until the sequence reaches the given state. If the sequence
//! is already in this state, this method returns immediately.
//----------------------------------------------------------------------

void uvm_sequence_base::wait_for_sequence_state( unsigned int state_mask )
{
  // TODO check: first boolean expression and then wait, or first wait and then check?
  while (!(m_sequence_state & state_mask))
  {
    sc_core::wait(m_sequence_state_ev);
  }
}

//----------------------------------------------------------------------
// member function: start (virtual)
//
//! Executes this sequence, returning when the sequence has completed.
//----------------------------------------------------------------------

void uvm_sequence_base::start(
  uvm_sequencer_base* sequencer,
  uvm_sequence_base* parent_sequence,
  int this_priority,
  bool call_pre_post )
{
  set_item_context(parent_sequence, sequencer);

  if ( !( (m_sequence_state == UVM_CREATED) || (m_sequence_state == UVM_STOPPED) || (m_sequence_state == UVM_FINISHED) ) )
  {
    std::ostringstream str;
    str << "Sequence " << get_full_name() << " already started.";
    uvm_report_fatal("SEQ_NOT_DONE", str.str(), UVM_NONE);
  }

  if (m_parent_sequence != NULL)
     m_parent_sequence->children_array[this] = true;

  if (this_priority < -1)
  {
    std::ostringstream str;
    str << "Sequence " << get_full_name() << "start has illegal priority: " << this_priority;
    uvm_report_fatal("SEQ_PRIO_ERR", str.str(), UVM_NONE);
  }

  if (this_priority < 0)
  {
    if (parent_sequence == NULL)
      this_priority = 100;
     else
       this_priority = parent_sequence->get_priority();
  }

  // Check that the response queue is empty from earlier runs
  clear_response_queue();

  m_priority = this_priority;

  if (get_sequencer() != NULL)
  {
    if (m_parent_sequence == NULL)
      m_tr_handle = m_sequencer->begin_tr(*this, get_name());
    else
      m_tr_handle = m_sequencer->begin_child_tr( *this, m_parent_sequence->m_tr_handle,
                                                 get_root_sequence_name());
  }

  // Ensure that the sequence_id is initialized in case this sequence
  // has been stopped previously
  set_sequence_id(-1);

  // Remove all sqr_seq_ids
  m_sqr_seq_ids.clear();

  // Register the sequence with the sequencer if defined
  if (get_sequencer() != NULL)
    get_sequencer()->m_register_sequence(this);

  // Change the state to PRE_START, do this before the fork so that
  // the "if (!(m_sequence_state inside {...}" works
  m_sequence_state = UVM_PRE_START;
  m_sequence_state_ev.notify();

  SC_FORK
    m_sequence_process = sc_spawn(sc_bind(&uvm_sequence_base::m_start_core, this, parent_sequence, call_pre_post))
  SC_JOIN

  if (m_sequencer != NULL)
    m_sequencer->end_tr(*this);

  // Clean up any sequencer queues after exiting; if we
  // were forcibly stopped, this step has already taken place
  if (m_sequence_state != UVM_STOPPED)
    if (get_sequencer() != NULL)
      get_sequencer()->m_sequence_exiting(this);

  // allow stopped and finish waiters to resume
  sc_core::wait(SC_ZERO_TIME);

  if ((m_parent_sequence != NULL) && (m_parent_sequence->children_array.find(this)!= m_parent_sequence->children_array.end()))
    m_parent_sequence->children_array.erase(this); // TODO also delete pointer here?

  old_automatic_phase_objection = get_automatic_phase_objection();
  m_init_phase_daps(true);
  set_automatic_phase_objection(old_automatic_phase_objection);
}

//----------------------------------------------------------------------
// member function: pre_start (virtual)
//
//! This member function is a user-definable callback that is called before the
//! optional execution of #pre_body.
//! This member function should not be called directly by the user.
//----------------------------------------------------------------------

void uvm_sequence_base::pre_start()
{
  return;
}

//----------------------------------------------------------------------
// member function: pre_body (virtual)
//
//! This member function is a user-definable callback that is called before the
//! execution of #body only when the sequence is started with <start>.
//! If #start is called with call_pre_post set to false, the the method #pre_body is not
//! called.
//! This member function should not be called directly by the application.
//----------------------------------------------------------------------

void uvm_sequence_base::pre_body()
{
  return;
}

//----------------------------------------------------------------------
// member function: pre_do (virtual)
//
//! This  member function is a user-definable callback that is called on the
//! parent sequence, if the sequence has issued a wait_for_grant() call and
//! after the sequencer has selected this sequence, and before the item is
//! randomized.
//! This member function should not be called directly by the application.
//----------------------------------------------------------------------

void uvm_sequence_base::pre_do( bool is_item )
{
   return;
}

//----------------------------------------------------------------------
// member function: mid_do (virtual)
//
//! This member function is a user-definable callback function that is called
//! after the sequence item has been randomized, and just before the item is
//! sent to the driver.
//! This member function should not be called directly by the application.
//----------------------------------------------------------------------

void uvm_sequence_base::mid_do( uvm_sequence_item* this_item )
{
  return;
}

//----------------------------------------------------------------------
// member function: body (virtual)
//
//! This is the user-defined member function where the main sequence code resides.
//! This member function should not be called directly by the user.
//----------------------------------------------------------------------

void uvm_sequence_base::body()
{
  uvm_report_warning("uvm_sequence_base", "Body definition undefined");
}

//----------------------------------------------------------------------
// member function: post_do (virtual)
//
//! This member function is a user-definable callback function that is called
//! after the driver has indicated that it has completed the item, using either
//! this item_done or put methods.
//! This member function should not be called directly by the user.
//----------------------------------------------------------------------

void uvm_sequence_base::post_do( uvm_sequence_item* this_item )
{
  return;
}

//----------------------------------------------------------------------
// member function: post_body (virtual)
//
//! This member function is a user-definable callback that is called after the
//! execution of #body only when the sequence is started with #start.
//! If <start> is called with call_pre_post set to false, the method #post_body is not
//! called.
//! This member function is a user-definable callback task that is called after the
//! execution of the body, unless the sequence is started with call_pre_post=0.
//! This member function should not be called directly by the user.
//----------------------------------------------------------------------

void uvm_sequence_base::post_body()
{
  return;
}

//----------------------------------------------------------------------
// member function: post_start (virtual)
//
//! This member function is a user-definable callback that is called after the
//! optional execution of #post_body.
//! This member function should not be called directly by the user.
//----------------------------------------------------------------------

void uvm_sequence_base::post_start()
{
  return;
}

//----------------------------------------------------------------------
// member function get_starting_phase
// Returns the 'starting phase'.
//
//! If non-NULL, the starting phase specifies the phase in which this
//! sequence was started.  The starting phase is set automatically when
//! this sequence is started as the default sequence on a sequencer.
//----------------------------------------------------------------------

uvm_phase* uvm_sequence_base::get_starting_phase() const
{
  return m_starting_phase_dap->get();
}

//----------------------------------------------------------------------
// member function: set_starting_phase
// Sets the 'starting phase'.
//
// NOTE: Internally, the uvm_sequence_base uses a uvm_get_to_lock_dap to
// protect the starting phase value from being modified
// after the reference has been read.  Once the sequence has ended
// its execution (either via natural termination, or being killed),
// then the starting phase value can be modified again.
//----------------------------------------------------------------------

void uvm_sequence_base::set_starting_phase( uvm_phase* phase )
{
  m_starting_phase_dap->set(phase);
}

//----------------------------------------------------------------------
// member function: set_automatic_phase_objection
// Sets the 'automatically object to starting phase' bit.
//
// The most common interaction with the starting phase
// within a sequence is to simply ~raise~ the phase's objection
// prior to executing the sequence, and ~drop~ the objection
// after ending the sequence (either naturally, or
// via a call to kill). In order to
// simplify this interaction for the user, the UVM
// provides the ability to perform this functionality
// automatically.
//
// NOTE 1: Internally, the uvm_sequence_base uses a uvm_get_to_lock_dap to
// protect the ~automatic_phase_objection~ value from being modified
// after the reference has been read.  Once the sequence has ended
// its execution (either via natural termination, or being killed),
// then the ~automatic_phase_objection~ value can be modified again.
//
// NOTE 2: NEVER set the automatic phase objection bit to true if your sequence
// runs with a forever loop inside of the body, as the objection will
// never get dropped!
//----------------------------------------------------------------------

void uvm_sequence_base::set_automatic_phase_objection( bool value )
{
  m_automatic_phase_objection_dap->set(value);
}


//----------------------------------------------------------------------
// member function: get_automatic_phase_objection
// Returns (and locks) the value of the 'automatically object to
// starting phase' bit.
//
// If 1, then the sequence will automatically raise an objection
// to the starting phase (if the starting phase is not ~null~) immediately
// prior to <pre_start> being called.  The objection will be dropped
// after <post_start> has executed, or <kill> has been called.
//----------------------------------------------------------------------

bool uvm_sequence_base::get_automatic_phase_objection() const
{
  return m_automatic_phase_objection_dap->get();
}

//----------------------------------------------------------------------
// member function: set_priority
//
//! The priority of a sequence may be changed at any point in time.  When the
//! priority of a sequence is changed, the new priority will be used by the
//! sequencer the next time that it arbitrates between sequences.
//!
//! The default priority value for a sequence is 100.  Higher values result
//! in higher priorities.
//----------------------------------------------------------------------

void uvm_sequence_base::set_priority( int value )
{
  m_priority = value;
}

//----------------------------------------------------------------------
// member function: get_priority
//
//! This function returns the current priority of the sequence.
//----------------------------------------------------------------------

int uvm_sequence_base::get_priority() const
{
  return m_priority;
}

//----------------------------------------------------------------------
// member function: is_relevant (virtual)
//
//! The default #is_relevant implementation returns true, indicating that the
//! sequence is always relevant.
//!
//! Users may choose to override with their own virtual function to indicate
//! to the sequencer that the sequence is not currently relevant after a
//! request has been made.
//!
//! When the sequencer arbitrates, it will call is_relevant on each requesting,
//! unblocked sequence to see if it is relevant. If a false is returned, then the
//! sequence will not be chosen.
//!
//! If all requesting sequences are not relevant, then the sequencer will call
//! #wait_for_relevant on all sequences and re-arbitrate upon its return.
//!
//! Any sequence that implements #is_relevant must also implement
//! wait_for_relevant so that the sequencer has a way to wait for a
//! sequence to become relevant.
//----------------------------------------------------------------------

bool uvm_sequence_base::is_relevant() const
{
  is_rel_default = true;
  return true;
}

//----------------------------------------------------------------------
// member function: wait_for_relevant (virtual)
//
//! This method is called by the sequencer when all available sequences are
//! not relevant.  When wait_for_relevant returns the sequencer attempt to
//! re-arbitrate.
//
//! Returning from this call does not guarantee a sequence is relevant,
//! although that would be the ideal. The method provide some delay to
//! prevent an infinite loop.
//
//! If a sequence defines is_relevant so that it is not always relevant (by
//! default, a sequence is always relevant), then the sequence must also supply
//! a wait_for_relevant method.
//----------------------------------------------------------------------

void uvm_sequence_base::wait_for_relevant() const
{
  // callback to be overwritten by application

  sc_core::sc_event _forever;
  wait_rel_default = true;
  if (is_rel_default != wait_rel_default)
    uvm_report_fatal("RELMSM",
      "is_relevant() was implemented without defining wait_for_relevant()", UVM_NONE);
  sc_core::wait(_forever);  // this is intended to never return
}


//----------------------------------------------------------------------
// member function: lock
//
//! Requests a lock on the specified sequencer. If sequencer is NULL, the lock
//! will be requested on the current default sequencer.
//!
//! A lock request will be arbitrated the same as any other request.  A lock is
//! granted after all earlier requests are completed and no other locks or
//! grabs are blocking this sequence.
//!
//! The lock call will return when the lock has been granted.
//----------------------------------------------------------------------

void uvm_sequence_base::lock( uvm_sequencer_base* sequencer )
{
  if (sequencer == NULL)
    sequencer = m_sequencer;

  if (sequencer == NULL)
    uvm_report_fatal("LOCKSEQR", "Null m_sequencer reference", UVM_NONE);

  sequencer->lock(this);
}

//----------------------------------------------------------------------
// member function: grab
//
//! Requests a grab on the specified sequencer.  If no argument is supplied,
//! the lock will be requested on the current default sequencer.
//!
//! A grab request is put in front of the arbitration queue. It will be
//! arbitrated before any other requests. A grab is granted when no other grabs
//! or locks are blocking this sequence.
//!
//! The grab call will return when the grab has been granted.
//----------------------------------------------------------------------

void uvm_sequence_base::grab( uvm_sequencer_base* sequencer )
{
  if (sequencer == NULL)
  {
    if (m_sequencer == NULL)
      uvm_report_fatal("GRAB", "Null m_sequencer reference", UVM_NONE);

    m_sequencer->grab(this);
  }
  else
    sequencer->grab(this);
}


//----------------------------------------------------------------------
// member function: unlock
//
//! Removes any locks or grabs obtained by this sequence on the specified
//! sequencer. If sequencer is null, then the unlock will be done on the
//! current default sequencer.
//----------------------------------------------------------------------

void uvm_sequence_base::unlock( uvm_sequencer_base* sequencer )
{
  if (sequencer == NULL)
  {
    if (m_sequencer == NULL)
      uvm_report_fatal("UNLOCK", "Null m_sequencer reference", UVM_NONE);

    m_sequencer->unlock(this);
  }
  else
    sequencer->unlock(this);
}


//----------------------------------------------------------------------
// member function: ungrab
//
//! Removes any locks or grabs obtained by this sequence on the specified
//! sequencer. If sequencer is NULL, then the unlock will be done on the
//! current default sequencer.
//----------------------------------------------------------------------

void uvm_sequence_base::ungrab( uvm_sequencer_base* sequencer )
{
  unlock(sequencer);
}

//--------------------------------------------------------------------------
// member function: is_blocked
//
//! Returns a boolean indicating whether this sequence is currently prevented from
//! running due to another lock or grab. True is returned if the sequence is
//! currently blocked. false is returned if no lock or grab prevents this
//! sequence from executing. Note that even if a sequence is not blocked, it
//! is possible for another sequence to issue a lock or grab before this
//! sequence can issue a request.
//--------------------------------------------------------------------------

bool uvm_sequence_base::is_blocked() const
{
  return m_sequencer->is_blocked(this);
}

//--------------------------------------------------------------------------
// member function: has_lock
//
//! Returns true if this sequence has a lock, and false otherwise.
//!
//! Note that even if this sequence has a lock, a child sequence may also have
//! a lock, in which case the sequence is still blocked from issuing
//! operations on the sequencer.
//--------------------------------------------------------------------------

bool uvm_sequence_base::has_lock()
{
  return m_sequencer->has_lock(this);
}

//--------------------------------------------------------------------------
// member function: kill
//
//! This member function will kill the sequence, and cause all current locks and
//! requests in the sequence's default sequencer to be removed. The sequence
//! state will change to STOPPED, and the post_body() and post_start() callback
//! methods will not be executed.
//!
//! If a sequence has issued locks, grabs, or requests on sequencers other than
//! the default sequencer, then care must be taken to unregister the sequence
//! with the other sequencer(s) using the sequencer unregister_sequence()
//! method.
//--------------------------------------------------------------------------

void uvm_sequence_base::kill()
{
  if (m_sequence_process.valid())
  {
    // If we are not connected to a sequencer, then issue
    // kill locally.
    if (m_sequencer == NULL)
    {
      m_kill();

      // We need to drop the objection if we raised it...
      if (get_automatic_phase_objection())
         m_safe_drop_starting_phase("automatic phase objection");

      return;
    }
    // If we are attached to a sequencer, then the sequencer
    // will clear out queues, and then kill this sequence
    m_sequencer->m_kill_sequence(this);

    // We need to drop the objection if we raised it...
    if (get_automatic_phase_objection())
       m_safe_drop_starting_phase("automatic phase objection");
    return;
  }
}

//--------------------------------------------------------------------------
// member function: do_kill (virtual)
//
//! This member function is a hook for the application that is called whenever
//! a sequence is
//! terminated by using either sequence.kill() or sequencer.stop_sequences()
//! (which effectively calls sequence.kill()).
//--------------------------------------------------------------------------

void uvm_sequence_base::do_kill()
{
  return;
}


//--------------------------------------------------------------------------
// Group: Sequence item execution
//--------------------------------------------------------------------------

//----------------------------------------------------------------------
// member function: create_item
//
//! The member function #create_item will create and initialize a #sequence_item or #sequence
//! using the factory.  The #sequence_item or #sequence will be initialized
//! to communicate with the specified sequencer.
//----------------------------------------------------------------------

uvm_sequence_item* uvm_sequence_base::create_item( uvm_object_wrapper* type_var,
                                                   uvm_sequencer_base* l_sequencer,
                                                   const std::string& name )
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_factory* f_ = cs->get_factory();

  uvm_sequence_item* item  = dynamic_cast<uvm_sequence_item*>
    (f_->create_object_by_type( type_var, this->get_full_name(), name ));

  item->set_item_context(this, l_sequencer);
  return item;
}

//----------------------------------------------------------------------
// member function: start_item (virtual)
//
//! The member function #start_item and #finish_item together will initiate operation of
//! a sequence item. If the item has not already been
//! initialized using #create_item, then it will be initialized here to use
//! the default sequencer specified by #m_sequencer.  Randomization
//! may be done between #start_item and #finish_item to ensure late generation
//!
//!  TODO - randomization not implemented
//----------------------------------------------------------------------

void uvm_sequence_base::start_item( uvm_sequence_item* item,
                                    int set_priority,
                                    uvm_sequencer_base* sequencer )
{
  uvm_sequence_base* seq;

  if(item == NULL)
  {
    std::ostringstream msg;
    msg << "attempting to start a null item from sequence '"
        << get_full_name() << "'";
    uvm_report_fatal("NULLITM", msg.str(), UVM_NONE);
    return;
  }

  seq = dynamic_cast<uvm_sequence_base*>(item);
  if(seq != NULL)
  {
    std::ostringstream msg;
    msg << "attempting to start a sequence using start_item() from sequence '"
        << get_full_name() << "'. Use seq.start() instead.";
    uvm_report_fatal("SEQNOTITM", msg.str(), UVM_NONE);
    return;
  }

  if (sequencer == NULL)
      sequencer = item->get_sequencer();

  if(sequencer == NULL)
      sequencer = get_sequencer();

  if(sequencer == NULL)
  {
    std::ostringstream msg;
    msg << "neither the item's sequencer nor dedicated sequencer has been "
        << "supplied to start item in " << get_full_name();
    uvm_report_fatal("SEQ", msg.str(), UVM_NONE);
    return;
  }

  item->set_item_context(this, sequencer);

  if (set_priority < 0)
    set_priority = get_priority();

  sequencer->wait_for_grant(this, set_priority);

#ifndef UVM_DISABLE_AUTO_ITEM_RECORDING
  // TODO transaction recording
  //sequencer->begin_child_tr(item, m_tr_handle, item->get_root_sequence_name()));
#endif
  pre_do(true);
}

//----------------------------------------------------------------------
// member function: finish_item (virtual)
//
//! The member function #finish_item, together with #start_item together will initiate operation of
//! a #sequence_item.  #finish_item must be called
//! after #start_item with no delays or delta-cycles.  Randomization, or other
//! functions may be called between the calls #start_item and #finish_item.
//----------------------------------------------------------------------

void uvm_sequence_base::finish_item( uvm_sequence_item* item,
                                     int set_priority )
{
  uvm_sequencer_base* sequencer;

  sequencer = item->get_sequencer();

  if (sequencer == NULL)
      uvm_report_fatal("STRITM", "sequence_item has null sequencer", UVM_NONE);

  mid_do(item);
  sequencer->send_request(this, item);
  sequencer->wait_for_item_done(this, -1);

  // FIXME: dirty workaround to comply to UVM-SV semantics which are non-TLM conform!
  // Copy the current values of a request_item from the sequencer, which might be changed 'in flight'
  // without asking for explicit response message using get_response(rsp)
  uvm_sequence_item* latest_item = sequencer->m_current_sequence_item;

#ifndef UVM_DISABLE_AUTO_ITEM_RECORDING
  sequencer->end_tr(*item);
#endif

  post_do(latest_item);
}

//----------------------------------------------------------------------
// member function: wait_for_grant (virtual)
//
//! The member function #wait_for_grant shall issue a request to the current
//! sequencer.  If \p item_priority is
//! not specified, then the current sequence priority will be used by the
//! arbiter. If a \p lock_request is made, then the sequencer will issue a lock
//! immediately before granting the sequence.  (Note that the lock may be
//! granted without the sequence being granted if #is_relevant is not asserted).
//!
//! When this method returns, the sequencer has granted the sequence, and the
//! sequence must call send_request without inserting any simulation delay
//! other than delta cycles.  The driver is currently waiting for the next
//! item to be sent via the send_request call.
//----------------------------------------------------------------------

void uvm_sequence_base::wait_for_grant( int item_priority, bool lock_request )
{
  if (get_sequencer() == NULL)
    uvm_report_fatal("WAITGRANT", "Null m_sequencer reference", UVM_NONE);

  get_sequencer()->wait_for_grant(this, item_priority, lock_request);
}


//----------------------------------------------------------------------
// member function: send_request (virtual)
//
//! The member function #send_request may only be called after a #wait_for_grant call.
//! This call will send the request item to the sequencer, which will forward
//! it to the driver. If the rerandomize boolean is set, the item will be
//! randomized before being sent to the driver.
//----------------------------------------------------------------------

void uvm_sequence_base::send_request( uvm_sequence_item* request, bool rerandomize )
{
  // NOTE: this method shall not be called - is overloaded by implementation
  // in param_base class
  if (get_sequencer() == NULL)
      uvm_report_fatal("SENDREQ", "Unable to find sequencer.", UVM_NONE); // was: Null m_sequencer reference

  get_sequencer()->send_request(this, request, rerandomize);
}

//----------------------------------------------------------------------
// member function: wait_for_item_done (virtual)
//
//! A sequence may optionally call #wait_for_item_done. This member function will block
//! until the driver calls item_done or put.  If no \p transaction_id parameter
//! is specified, then the call will return the next time that the driver calls
//! item_done or put.  If a specific transaction_id is specified, then the call
//! will return when the driver indicates completion of that specific item.
//!
//! Note that if a specific \p transaction_id has been specified, and the driver
//! has already issued an item_done or put for that transaction, then the call
//! will hang, having missed the earlier notification.
//----------------------------------------------------------------------

void uvm_sequence_base::wait_for_item_done( int transaction_id )
{
  if ( get_sequencer() == NULL)
      uvm_report_fatal("WAITITEMDONE", "Unable to find default sequencer.", UVM_NONE);

  get_sequencer()->wait_for_item_done(this, transaction_id);
}

//----------------------------------------------------------------------
// member function: use_response_handler
//
//! When called with enable set to true, responses will be sent to the response
//! handler. Otherwise, responses must be retrieved using get_response.
//!
//! By default, responses from the driver are retrieved in the sequence by
//! calling #get_response.
//!
//! An alternative method is for the sequencer to call the #response_handler
//! function with each response.
//----------------------------------------------------------------------

void uvm_sequence_base::use_response_handler( bool enable )
{
  m_use_response_handler = enable;
}

//----------------------------------------------------------------------
// member function: get_use_response_handler
//
//! Returns the state of the use_response_handler.
//----------------------------------------------------------------------

bool uvm_sequence_base::get_use_response_handler() const
{
  return m_use_response_handler;
}

//----------------------------------------------------------------------
// member function: response_handler (virtual)
//
//! When the #use_reponse_handler variable is set to true, this virtual member
//! function is called by the sequencer for each response that arrives
//! for this sequence.
//----------------------------------------------------------------------

void uvm_sequence_base::response_handler( const uvm_sequence_item* response )
{
  uvm_report_fatal("RSPHDL", "No response handler defined!", UVM_NONE);
}

//----------------------------------------------------------------------
// member function: set_response_queue_error_report_disabled
//
//! By default, if the response_queue overflows, an error is reported. The
//! response_queue will overflow if more responses are sent to this sequence
//! from the driver than get_response calls are made. Setting value to 0
//! disables these errors, while setting it to 1 enables them.
//----------------------------------------------------------------------

void uvm_sequence_base::set_response_queue_error_report_disabled( bool value )
{
  response_queue_error_report_disabled = value;
}


//----------------------------------------------------------------------
// member function: get_response_queue_error_report_disabled
//
//! When this bit is 0 (default value), error reports are generated when
//! the response queue overflows. When this bit is 1, no such error
//! reports are generated.
//----------------------------------------------------------------------

bool uvm_sequence_base::get_response_queue_error_report_disabled() const
{
  return response_queue_error_report_disabled;
}

//----------------------------------------------------------------------
// member function: set_response_queue_depth
//
//! The default maximum depth of the response queue is 8. These method is used
//! to examine or change the maximum depth of the response queue.
//!
//! Setting the response_queue_depth to -1 indicates an arbitrarily deep
//! response queue.  No checking is done.
//----------------------------------------------------------------------

 void uvm_sequence_base::set_response_queue_depth( int value )
 {
   response_queue_depth = value;
 }

//----------------------------------------------------------------------
// member function: get_response_queue_depth
//
//! Returns the current depth setting for the response queue.
//----------------------------------------------------------------------

 int uvm_sequence_base::get_response_queue_depth() const
 {
   return response_queue_depth;
 }

//----------------------------------------------------------------------
// member function: clear_response_queue (virtual)
//
//! Empties the response queue for this sequence.
//----------------------------------------------------------------------

void uvm_sequence_base::clear_response_queue()
{
  if (response_queue.size() != 0)
    uvm_report_error(get_full_name(), "Response queue was not empty from previous runs! Response queue is now cleared for upcoming transactions.", UVM_NONE);

  response_queue.clear();
}


////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: m_start_core
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::m_start_core( uvm_sequence_base* parent_sequence,
                                      bool call_pre_post )
{
  // NO WAIT HERE! - It seems the SC_FORK already implements a SC_ZERO_WAIT?

  // Raise the objection if enabled
  // (This will lock the uvm_get_to_lock_dap)
  if( get_automatic_phase_objection() )
     m_safe_raise_starting_phase("automatic phase objection");

  pre_start();

  if (call_pre_post == true) {
    m_sequence_state = UVM_PRE_BODY;
    m_sequence_state_ev.notify();
    sc_core::wait(SC_ZERO_TIME);
    pre_body();
  }

  if (parent_sequence != NULL)
  {
    parent_sequence->pre_do(0);
    parent_sequence->mid_do(this);
  }

  m_sequence_state = UVM_BODY;
  m_sequence_state_ev.notify();
  sc_core::wait(SC_ZERO_TIME);
  body();

  m_sequence_state = UVM_ENDED;
  m_sequence_state_ev.notify();
  sc_core::wait(SC_ZERO_TIME);

  if (parent_sequence != NULL)
    parent_sequence->post_do(this);

  if (call_pre_post == true) {
    m_sequence_state = UVM_POST_BODY;
    m_sequence_state_ev.notify();
    sc_core::wait(SC_ZERO_TIME);
    post_body();
  }

  m_sequence_state = UVM_POST_START;
  m_sequence_state_ev.notify();
  sc_core::wait(SC_ZERO_TIME);
  post_start();

  // Drop the objection if enabled
  if( get_automatic_phase_objection() )
     m_safe_drop_starting_phase("automatic phase objection");

  m_sequence_state = UVM_FINISHED;
  m_sequence_state_ev.notify();
  sc_core::wait(SC_ZERO_TIME);
}

//----------------------------------------------------------------------
// member function: put_response
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::put_response( const uvm_sequence_item& response )
{
  put_base_response(response);
}

//----------------------------------------------------------------------
// member function: get_base_response
//
//! Implementation-defined member function
//----------------------------------------------------------------------

uvm_sequence_item* uvm_sequence_base::get_base_response( int transaction_id )
{
  while (response_queue.size() == 0)
  {
   sc_core::wait(response_queue_event);
  }

  // if transaction id has no unique ID, we simply take the first
  // element from the response queue
  if (transaction_id == -1)
  {
    uvm_sequence_item* item = response_queue.front(); // read first element
    //response_queue.pop_front(); // and remove first element afterwards
    return item;
  }

  for(;;) // forever
  {
    for (response_queue_listT::iterator
        it = response_queue.begin();
        it != response_queue.end();
        it++)
    {
      if ((*it)->get_transaction_id() == transaction_id)
      {
        uvm_sequence_item* item = (*it);
        //response_queue.erase(it);
        return item; // immediate exit loop as size has changed
      }
    }
    sc_core::wait(response_queue_event);
  }
}

//----------------------------------------------------------------------
// member function: put_base_response
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::put_base_response( const uvm_sequence_item& response )
{
  uvm_sequence_item* item = const_cast<uvm_sequence_item*>(&response); // TODO avoid const_cast!

  if ( (response_queue_depth == -1) ||
       ((int)response_queue.size() < response_queue_depth)
     )
  {
    response_queue.push_back(item);
    response_queue_event.notify();
    return;
  }

  if (!response_queue_error_report_disabled)
    uvm_report_error(get_full_name(), "Response queue overflow, response was dropped", UVM_NONE);
}

//----------------------------------------------------------------------
// member function: del_base_response
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::del_base_response( uvm_sequence_item* response )
{
  if (response_queue.size() == 0) return; // no items, nothing to delete

  response_queue_listT::iterator it = std::find(response_queue.begin(),
                                     response_queue.end(), response);

  if ( it == response_queue.end() ) return; // not found, nothing to delete
  else
  {
    delete (*it);
    response_queue.erase(it);
  }
}

//----------------------------------------------------------------------
// member function: m_get_sqr_sequence_id
//
//! Implementation-defined member function
//----------------------------------------------------------------------

int uvm_sequence_base::m_get_sqr_sequence_id( int sequencer_id,
                                              bool update_sequence_id )
{
  if (m_sqr_seq_ids.find(sequencer_id) != m_sqr_seq_ids.end() ) // if exists
  {
    if (update_sequence_id)
      set_sequence_id(m_sqr_seq_ids[sequencer_id]);
    return m_sqr_seq_ids[sequencer_id];
  }

  if (update_sequence_id)
    set_sequence_id(-1);

  return -1;
}

//----------------------------------------------------------------------
// member function: m_set_sqr_sequence_id
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::m_set_sqr_sequence_id( int sequencer_id,
                                               int sequence_id )
{
  m_sqr_seq_ids[sequencer_id] = sequence_id;
  set_sequence_id(sequence_id);
}

//----------------------------------------------------------------------
// member function: m_kill
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::m_kill()
{
  do_kill();

  for( children_array_ItT it = children_array.begin();
       it != children_array.end();
       ++it )
     it->first->kill();

  if (m_sequence_process.valid())
  {
    m_sequence_process.kill(SC_INCLUDE_DESCENDANTS); // TODO check
    //m_sequence_process = null;
  }
  m_sequence_state = UVM_STOPPED;

  if ((m_parent_sequence != NULL) && (m_parent_sequence->children_array.find(this)!= m_parent_sequence->children_array.end()))
    m_parent_sequence->children_array.erase(this); // TODO also delete pointer here?
}

//----------------------------------------------------------------------
// member function: m_clear
//
// Clean-up and release memory
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_sequence_base::m_clear()
{
  for( response_queue_listT::iterator
       it = response_queue.begin();
       it != response_queue.end();
       it++)
    delete *it;

  m_clear_phase_daps();
}

/*
void uvm_sequence_base::m_copy( const uvm_sequence_base& obj )
{
  m_wait_for_grant_semaphore = obj.m_wait_for_grant_semaphore;
  m_next_transaction_id = obj.m_next_transaction_id;

  starting_phase = obj.starting_phase;

  //m_sqr_seq_ids.clear();

  m_priority = obj.m_priority;
  m_use_response_handler = obj,m_use_response_handler;
  response_queue_depth = obj.response_queue_depth;
  response_queue_error_report_disabled = obj.response_queue_error_report_disabled;

  //response_queue.clear();
  m_tr_handle = obj.m_tr_handle;

  is_rel_default = obj.is_rel_default;
}

uvm_sequence_base& uvm_sequence_base::uvm_sequence_base( const uvm_sequence_base& obj )
{
  m_copy(obj);

}

uvm_sequence_base& uvm_sequence_base::operator= ( const uvm_sequence_base& obj )
{
  if (this != &obj) {
    m_clear();
    m_copy(obj);
  }
  return *this;
}
*/

//----------------------------------------------------------------------
// member function: m_safe_raise_starting_phase
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_sequence_base::m_safe_raise_starting_phase( std::string description,
                                                     int count )
{
  uvm_phase* starting_phase = get_starting_phase();
  if (starting_phase != NULL)
    starting_phase->raise_objection(this, description, count);

}

//----------------------------------------------------------------------
// member function: m_safe_drop_starting_phase
//
// Implementation defined
//----------------------------------------------------------------------

void uvm_sequence_base::m_safe_drop_starting_phase( std::string description,
                                                    int count )
{
  uvm_phase* starting_phase = get_starting_phase();
  if (starting_phase != NULL)
    starting_phase->drop_objection(this, description, count);
}

//----------------------------------------------------------------------
// member function: m_init_phase_daps
//
// Either creates or renames DAPS
// Implementation defined
//----------------------------------------------------------------------

void uvm_sequence_base::m_init_phase_daps( bool create )
{
  std::string apo_name = get_full_name() + ".automatic_phase_objection";
  std::string sp_name = get_full_name() + ".starting_phase";

  // clean-up  old instances if they exist
  if (m_automatic_phase_objection_dap != NULL)
    delete m_automatic_phase_objection_dap;

  if (m_starting_phase_dap != NULL)
    delete m_starting_phase_dap;

  if(create)
  {
    // TODO no urgency to use the UVM factory here, so instantiting in the normal way..
    m_automatic_phase_objection_dap = new uvm_get_to_lock_dap<bool>(apo_name);
    m_starting_phase_dap = new uvm_get_to_lock_dap<uvm_phase*>(sp_name);
  }
  else
  {
    m_automatic_phase_objection_dap->set_name(apo_name);
    m_starting_phase_dap->set_name(sp_name);
  }
}

//----------------------------------------------------------------------
// member function: m_clean_phase_daps
//
// Cleanup and remove DAPs
// Implementation defined
//----------------------------------------------------------------------

void uvm_sequence_base::m_clear_phase_daps()
{
  if (m_automatic_phase_objection_dap != NULL) delete m_automatic_phase_objection_dap;
  if (m_starting_phase_dap != NULL) delete m_starting_phase_dap;
  m_automatic_phase_objection_dap = NULL;
  m_starting_phase_dap = NULL;
}


} // namespace uvm
