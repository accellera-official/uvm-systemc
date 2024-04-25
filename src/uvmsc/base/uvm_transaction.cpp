//----------------------------------------------------------------------
//   Copyright 2014 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
//   Copyright 2012-2015 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
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

#include <sstream>

#include "uvmsc/base/uvm_transaction.h"
#include "uvmsc/print/uvm_printer.h"
#include "uvmsc/base/uvm_component.h"

using namespace sc_core;

namespace uvm {

//----------------------------------------------------------------------
// Constuctors
//----------------------------------------------------------------------

uvm_transaction::uvm_transaction() : uvm_object( sc_core::sc_gen_unique_name( "transaction" ))
{
  m_init_transaction();
}

uvm_transaction::uvm_transaction( uvm_object_name name ) : uvm_object( name )
{
  m_init_transaction();
}

//----------------------------------------------------------------------
// member function: set_transaction_id
//
//! Sets this transactions numeric identifier to id. If not set via this
//! method, the transaction ID defaults to -1.
//! When using sequences to generate stimulus, the transaction ID is used
//! along with the sequence ID to route responses in sequencers and to
//! correlate responses to requests.
//----------------------------------------------------------------------

void uvm_transaction::set_transaction_id(int id)
{
  m_transaction_id = id;
}

//----------------------------------------------------------------------
// member function:  get_transaction_id
//
//! Returns this transactions numeric identifier, which is -1 if not set
//! explicitly by set_transaction_id.
//! When using a uvm_sequence<REQ,RSP> to generate stimulus, the transaction
//! ID is used along with the sequence ID to route responses in sequencers
//! and to correlate responses to requests.
//----------------------------------------------------------------------

int uvm_transaction::get_transaction_id() const
{
  return m_transaction_id;
}

//----------------------------------------------------------------------
// Member function: begin_tr
//
//! This function indicates that the transaction has been started and is not
//! the child of another transaction. Generally, a consumer component begins
//! execution of a transactions it receives.
//!
//! Typically a <uvm_driver> would call <uvm_component::begin_tr>, which
//! calls this method, before actual execution of a sequence item transaction.
//! Sequence items received by a driver are always a child of a parent sequence.
//! In this case, begin_tr obtains the parent handle and delegates to <begin_child_tr>.
//!
//! See <accept_tr> for more information on how the
//! begin-time might differ from when the transaction item was received.
//!
//! This function performs the following actions:
//!
//! - The transaction's internal start time is set to the current simulation
//!   time, or to begin_time if provided and non-zero. The begin_time may be
//!   any time, past or future, but should not be less than the accept time.
//!
//! - If recording is enabled, then a new database-transaction is started with
//!   the same begin time as above.
//!
//! - The <do_begin_tr> method is called to allow for any post-begin action in
//!   derived classes.
//!
//! - The transaction's internal begin event is triggered. Any processes
//!   waiting on this event will resume in the next delta cycle.
//!
//! The return value is a transaction handle, which is valid (non-zero) only if
//! recording is enabled. The meaning of the handle is implementation specific.
//----------------------------------------------------------------------


int uvm_transaction::begin_tr( const sc_time& begin_time )
{
  // TODO begin transaction
  return 0;
}


void uvm_transaction::end_tr( const sc_time& end_time, bool free_handle )
{
  // TODO incomplete at this stage, only notification we are done with transaction

  end_event.trigger();
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: m_init_transaction
//
// Implementation defined
// Helper functions to initialize transaction object
//----------------------------------------------------------------------

void uvm_transaction::m_init_transaction()
{
  m_transaction_id =-1;
  initiator = NULL; // TODO assign parent component

  begin_time = SC_ZERO_TIME; // TODO replace by sc_max_time?
  end_time = SC_ZERO_TIME;
  accept_time = SC_ZERO_TIME;
}


//----------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------

uvm_transaction::~uvm_transaction()
{
  // TODO cleanup
  //std::cout << "destructor of uvm_transaction " << get_full_name() << std::endl;
}

//----------------------------------------------------------------------
// member function: do_print
//
//! Implementation-defined member function
//----------------------------------------------------------------------

void uvm_transaction::do_print( const uvm_printer& printer ) const
{
  std::string str;

  uvm_object::do_print(printer);

  // TODO replace by sc_max_time?
  if(accept_time != SC_ZERO_TIME )
    printer.print_time("accept_time", accept_time);

  if(begin_time != SC_ZERO_TIME )
    printer.print_time("begin_time", begin_time);

  if(end_time != SC_ZERO_TIME )
    printer.print_time("end_time", end_time);

  if(initiator != NULL)
  {
    std::ostringstream str;
    str << "@" << initiator->get_inst_id();
    printer.print_generic("initiator", initiator->get_type_name(), -1, str.str());
  }
}

} /* namespace uvm */
