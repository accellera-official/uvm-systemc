//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2012-2019 NXP B.V.
//   Copyright 2013 Fraunhofer-Gesellschaft zur Foerderung
//					der angewandten Forschung e.V.
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

#ifndef UVM_SEQUENCER_H_
#define UVM_SEQUENCER_H_

#include <systemc>
#include <tlm.h>

#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/seq/uvm_sequencer_param_base.h"
#include "uvmsc/seq/uvm_sequence_item.h"
#include "uvmsc/seq/uvm_sequencer_ifs.h"
#include "uvmsc/tlm1/uvm_sqr_connections.h"

namespace uvm {

//----------------------------------------------------------------------
// CLASS: uvm_sequencer<REQ,RSP>
//
// The class uvm_sequencer defines the interface for the TLM communication
// of sequences or sequence-items by providing access via an export object
// of class sc_export.
//----------------------------------------------------------------------

template <typename REQ = uvm_sequence_item, typename RSP = REQ>
class uvm_sequencer : public uvm_sequencer_param_base<REQ,RSP>,
                      public uvm_sqr_if_base<REQ, RSP>
{
 public:
  typedef uvm_sequencer<REQ, RSP> this_type;

  //--------------------------------------------------------------------------
  // UVM Standard LRM API below
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Group: variables
  //--------------------------------------------------------------------------

  // TLM export port
//  sc_core::sc_export<uvm_sqr_if_base<REQ, RSP> > seq_item_export;

   uvm_seq_item_pull_imp<REQ, RSP, this_type > seq_item_export;

  //--------------------------------------------------------------------------
  // Constructor and destructor
  //--------------------------------------------------------------------------

  explicit uvm_sequencer( uvm_component_name name_ );
  virtual ~uvm_sequencer();

  //--------------------------------------------------------------------------
  // Group: Sequencer interface
  //--------------------------------------------------------------------------

  //virtual REQ get_next_item( tlm::tlm_tag<REQ>* req = NULL );
  virtual REQ get_next_item( REQ* req = NULL );
  virtual void get_next_item( REQ& req );

  virtual bool try_next_item( REQ& req );

  virtual void item_done( const RSP& item, bool use_item = true );
  virtual void item_done(); // TODO - set default to NULL/NIL for template param, combine into one method?

  virtual void put( const RSP& rsp );
  virtual void put_response( const RSP& rsp ); // TODO not in standard anymore? remove?

  virtual void get( REQ& req );
  virtual REQ get( REQ* req = NULL );
  //virtual REQ get( tlm::tlm_tag<REQ>* req = NULL );

  virtual void peek( REQ& req );
  virtual REQ peek( REQ* req = NULL );
  //virtual REQ peek( tlm::tlm_tag<REQ>* req = NULL ); // FIXME: should be const in line with SystemC TLM API?

  virtual void stop_sequences();

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual const std::string get_type_name() const;

  virtual const char* kind() const; // SystemC API

  this_type get_if() { static uvm_sequencer<REQ, RSP> m_if("m_if"); return m_if; }

  // data members

  mutable bool sequence_item_requested;
  bool get_next_item_called;

}; // class uvm_sequencer


/////////////////////////////////////////////
// Class implementation starts here
/////////////////////////////////////////////

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequencer<REQ,RSP>::uvm_sequencer( uvm_component_name name_ )
  : uvm_sequencer_param_base<REQ,RSP>( name_ ), seq_item_export("seq_item_export")
{
  // bind export to itself (since it is an imp port)
  // TODO should be replaced with 2nd constructor argument
  this->seq_item_export(*this);

  sequence_item_requested = false;
  get_next_item_called = false;

}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequencer<REQ,RSP>::~uvm_sequencer()
{}

//----------------------------------------------------------------------
// member function: kind
//
// SystemC API
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
const char* uvm_sequencer<REQ,RSP>::kind() const
{
  return "uvm::uvm_sequencer";
}

//----------------------------------------------------------------------
// member function: get_type_name
//
//! Returns the type of object
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
const std::string uvm_sequencer<REQ,RSP>::get_type_name() const
{
  return std::string(this->kind());
}

//----------------------------------------------------------------------
// member function: item_done()
//
//! The member function #item_done shall indicate that the request is
//! completed.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::item_done()
{
  RSP dummy;
  item_done(dummy, false);
}

//----------------------------------------------------------------------
// member function: item_done(a,b)
//
//! The member function #item_done shall indicate that the request is
//! completed.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::item_done(const RSP& item, bool use_item)
{
  REQ req;

  // Set flag to allow next get_next_item or peek to get a new sequence_item
  sequence_item_requested = false;
  get_next_item_called = false;

  if (this->m_req_fifo.nb_get(req) == 0)
  {
    std::ostringstream str;
    str << "Item_done() called with no outstanding requests." << std::endl;
    str << "Each call to item_done() must be paired with a previous call to get_next_item().";
    uvm_report_fatal(this->get_type_name(), str.str() );
  }
  else
  {
    this->m_wait_for_item_sequence_id = req.get_sequence_id();
    this->m_wait_for_item_transaction_id = req.get_transaction_id();
    this->m_wait_for_item_sequence_ev.notify();
  }

  if (use_item)
    seq_item_export->put(item);

  // Grant any locks as soon as possible
  this->grant_queued_locks();
}

//----------------------------------------------------------------------
// member function: get
//
//! The member function #get shall retrieve the next available item from
//! a sequence.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ uvm_sequencer<REQ,RSP>::get(REQ* req)
{
  REQ r;

  if (!sequence_item_requested)
    this->m_select_sequence();

  if (req!=NULL)
    this->m_current_sequence_item = req;

  sequence_item_requested = true;

  r = this->m_req_fifo.peek((tlm::tlm_tag<REQ>*)(req)); //note: we peek here, as we do the get in the item_done() call
  item_done();
  return r;
}

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::get( REQ& req )
{
  this->m_current_sequence_item = &req;
  req = get();
}

//----------------------------------------------------------------------
// member function: peek
//
//! The member function #peek shall return the current request item
//! if one is in the FIFO.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ uvm_sequencer<REQ,RSP>::peek(REQ* req)
{
  REQ r;

  if (!sequence_item_requested )
    this->m_select_sequence();

  if (req!=NULL)
    this->m_current_sequence_item = req;

  // Set flag indicating that the item has been requested to ensure that
  // item_done() or get() is called between requests
  sequence_item_requested = true;

  r = this->m_req_fifo.peek((tlm::tlm_tag<REQ>*)req);
  return r;
}


template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::peek( REQ& req )
{
  req = peek();
  this->m_current_sequence_item = &req;
}

//----------------------------------------------------------------------
// member function: get_next_item
//
//! The member function get_next_item shall retrieve the next available
//! item from a sequence.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ uvm_sequencer<REQ,RSP>::get_next_item(REQ* req)
{
  // If a sequence_item has already been requested, then get_next_item()
  // should not be called again until item_done() has been called.
  if (get_next_item_called)
    uvm_report_error(this->get_full_name(),
      "get_next_item() called twice without item_done or get in between", UVM_NONE);

  if (req!=NULL)
    this->m_current_sequence_item = req;

  if (!sequence_item_requested)
    this->m_select_sequence();

  // Set flag indicating that the item has been requested to ensure that item_done or get
  // is called between requests
  sequence_item_requested = true;
  get_next_item_called = true;

  return this->m_req_fifo.peek((tlm::tlm_tag<REQ>*)req);
}

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::get_next_item( REQ& req )
{
  this->m_current_sequence_item = &req;
  req = get_next_item();
}


//----------------------------------------------------------------------
// member function: try_next_item
//
//! The member function try_next_item shall retrieve the next available
//! item from a sequence if one is available.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
bool uvm_sequencer<REQ,RSP>::try_next_item( REQ& req )
{
  int selected_sequence;
  sc_core::sc_time arb_time;
  uvm_sequence_base* seq;

  if (get_next_item_called)
  {
    uvm_report_error(this->get_full_name(), "get_next_item/try_next_item called twice without item_done or get in between", UVM_NONE);
    return false;
  }

  // allow state from last transaction to settle such that sequences'
  // relevancy can be determined with up-to-date information
  this->wait_for_sequences();

  // choose the sequence based on relevancy
  selected_sequence = this->m_choose_next_request();

  // return if none available
  if (selected_sequence == -1)
  {
    return false;
  }

  // now, allow chosen sequence to resume
  this->m_set_arbitration_completed(this->arb_sequence_q[selected_sequence]->request_id);
  seq = this->arb_sequence_q[selected_sequence]->sequence_ptr;
  this->arb_sequence_q.erase(this->arb_sequence_q.begin()+selected_sequence);
  this->m_update_lists();
  sequence_item_requested = true;
  get_next_item_called = true;

  // give it one NBA to put a new item in the fifo
  this->wait_for_sequences();

  // attempt to get the item; if it fails, produce an error and return
  if (!this->m_req_fifo.nb_peek(req)) //try_peek
  {
    std::ostringstream str;
    str << "try_next_item: the selected sequence '"
        <<  seq->get_full_name()
        << "' did not produce an item within an NBA delay. "
        << "Sequences should not consume time between calls to start_item and finish_item. "
        << "Returning NULL item.";
    uvm_report_error("TRY_NEXT_BLOCKED", str.str(), UVM_NONE);
    return false;
  }

  return true;
}


//----------------------------------------------------------------------
// member function: put
//
//! The member function put shall send a response back to the sequence
//! that issued the request.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::put( const RSP& rsp )
{
  RSP* crsp = const_cast<RSP*>(&rsp);
  RSP* item = new RSP(rsp.get_name()+"_q");
  *item = *crsp; // copy response in temporary item for queue
  this->put_response_base(*item);
  sc_core::wait(sc_core::SC_ZERO_TIME); // TODO do we really need this?
}

//----------------------------------------------------------------------
// member function: put_reponse
//
//! Implementation-defined member function
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::put_response( const RSP& rsp )
{
  RSP* crsp = const_cast<RSP*>(&rsp);
  RSP* item = new RSP(rsp.get_name()+"_q");
  *item = *crsp; // copy response in temporary item for queue
  this->put_response_base(*item);
  sc_core::wait(sc_core::SC_ZERO_TIME);  // TODO do we really need this?
}

//----------------------------------------------------------------------
// member function: stop_sequences
//
//! Tells the sequencer to kill all sequences and child sequences currently
//! operating on the sequencer, and remove all requests, locks and responses
//! that are currently queued.  This essentially resets the sequencer to an
//! idle state.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer<REQ,RSP>::stop_sequences()
{
  REQ t;
  uvm_sequencer_param_base<REQ,RSP>::stop_sequences();
  sequence_item_requested  = false;
  get_next_item_called     = false;

  // Empty the request fifo
  if (this->m_req_fifo.used())
  {
    uvm_report_info(this->get_full_name(), "Sequences stopped.  Removing all requests from sequencer FIFO.");
    while (this->m_req_fifo.nb_get(t)); //try_get
  }
}

} /* namespace uvm */

#endif /* UVM_SEQUENCER_H_ */

