//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2012-2020 NXP B.V.
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

#ifndef UVM_SEQUENCE_H_
#define UVM_SEQUENCE_H_

#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequencer.h"
#include "uvmsc/print/uvm_printer.h"

namespace uvm {

//------------------------------------------------------------------------------
// CLASS: uvm_sequence <REQ,RSP>
//
//! The class #uvm_sequence provides the interfaces necessary in order to create
//! streams of sequence items and/or other sequences.
//------------------------------------------------------------------------------

template <typename REQ = uvm_sequence_item, typename RSP = REQ>
class uvm_sequence : public uvm_sequence_base
{
 public:
  explicit uvm_sequence( uvm_object_name name_ );
  ~uvm_sequence() override;

  void send_request( uvm_sequence_item* request, bool rerandomize = false ) override;

  REQ get_current_item() const;

  virtual void get_response( RSP* response, int transaction_id = -1 );

  // Variable: req
  //
  // The sequence contains a field of the request type called req.  The user
  // can use this field, if desired, or create another field to use.  The
  // default ~do_print~ will print this field.
  REQ req;

  // Variable: rsp
  //
  // The sequence contains a field of the response type called rsp.  The user
  // can use this field, if desired, or create another field to use.   The
  // default ~do_print~ will print this field.
  RSP rsp;

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

 private:

  void put_response( const uvm_sequence_item& response_item ) override;

  void do_print( const uvm_printer& printer ) const override;

  uvm_sequencer_param_base<REQ, RSP>* param_sequencer;

};

/////////////////////////////////////////////
/////////////////////////////////////////////

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequence<REQ,RSP>::uvm_sequence( const std::string& name_ )
  : uvm_sequence_base( name_ )
{
  param_sequencer = nullptr;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequence<REQ,RSP>::~uvm_sequence()
{
}

//----------------------------------------------------------------------
// member function: send_request
//
// This member function will send the request item to the sequencer, which
// will forward it to the driver.  If the rerandomize bit is set, the item
// will be randomized before being sent to the driver. The send_request
// function mayonly be called after <uvm_sequence_base::wait_for_grant>
// returns.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequence<REQ,RSP>::send_request(uvm_sequence_item* request, bool rerandomize)
{
  REQ* m_request = nullptr;

  if (get_sequencer() == nullptr)
    uvm_report_fatal("SSENDREQ", "nullptr m_sequencer reference", UVM_NONE);

  m_request = dynamic_cast<REQ*>(request);

  if (m_request == nullptr)
    uvm_report_fatal("SSENDREQ", "Failure to cast uvm_sequence_item to request", UVM_NONE);

  m_sequencer->send_request(this, request, rerandomize);
}


//----------------------------------------------------------------------
// member function: get_current_item
//
// Returns the request item currently being executed by the sequencer. If the
// sequencer is not currently executing an item, this method will return nullptr.
//
// The sequencer is executing an item from the time that get_next_item or peek
// is called until the time that get or item_done is called.
//
// Note that a driver that only calls get will never show a current item,
// since the item is completed at the same time as it is requested.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ uvm_sequence<REQ,RSP>::get_current_item() const
{
  param_sequencer = dynamic_cast< uvm_sequencer_param_base<REQ,RSP>* >(m_sequencer);

  if (param_sequencer == nullptr)
    uvm_report_fatal("SGTCURR",
                     "Failure to cast m_sequencer to the parameterized sequencer",
                     UVM_NONE );
  return (param_sequencer->get_current_item());
}

//----------------------------------------------------------------------
// member function: get_response
//
// By default, sequences must retrieve responses by calling get_response.
// If no transaction_id is specified, this member function will return the
// next response sent to this sequence.  If no response is available in the
// response queue, the member function will wait until a response is recieved.
//
// If a transaction_id is parameter is specified, the member function will
// wait until a response with that transaction_id is received in the response
// queue.
//
// The default size of the response queue is -1, meaning that no limit is set.
// The get_response member function must be called soon enough to avoid an
// overflow of the response queue to prevent responses from being dropped.
//
// If a response is dropped in the response queue, an error will be reported
// unless the error reporting is disabled using the member function
// set_response_queue_error_report_disabled.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequence<REQ,RSP>::get_response( RSP* response, int transaction_id )
{
  RSP* rsp;
  uvm_sequence_item* item = get_base_response( transaction_id );
  rsp = dynamic_cast<RSP*>(item);
  *response = *rsp; // copy of the transaction
  del_base_response( item ); // flush response from memory
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------
// member function: put_response
//
// Implementation defined
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequence<REQ,RSP>::put_response( const uvm_sequence_item& response_item )
{
  const RSP* crsp = dynamic_cast<const RSP*>(&response_item);
  RSP* rsp = const_cast<RSP*>(crsp); // TODO avoid const_cast!

  if (rsp == nullptr)
    uvm_report_fatal("PUTRSP", "Failure to cast response in put_response.", UVM_NONE);

  put_base_response(response_item);
}

//----------------------------------------------------------------------
// member function: do_print
//
// Implementation defined
//----------------------------------------------------------------------
template <typename REQ, typename RSP>
void uvm_sequence<REQ,RSP>::do_print( const uvm_printer& printer ) const
{
  uvm_sequence_base::do_print(printer);
  printer.print_object("req", req);
  printer.print_object("rsp", rsp);
}

} /* namespace uvm */

#endif /* UVM_SEQUENCE_H_ */
