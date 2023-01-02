//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2012-2022 NXP B.V.
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

#ifndef UVM_SEQUENCER_PARAM_BASE_H_
#define UVM_SEQUENCER_PARAM_BASE_H_

#include <sstream>
#include <tlm.h>

#include <list>

#include "uvmsc/base/uvm_component_name.h"
#include "uvmsc/phasing/uvm_phase.h"
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/seq/uvm_sequence_base.h"
#include "uvmsc/seq/uvm_sequencer_analysis_fifo.h"
#include "uvmsc/base/uvm_transaction.h"
#include "uvmsc/base/uvm_object_globals.h"
#include "uvmsc/tlm1/uvm_analysis_export.h"

namespace uvm {

// forward declarations
//class uvm_sequence_base;
class uvm_sequence_item;

//----------------------------------------------------------------------
// CLASS: uvm_sequencer_param_base <REQ,RSP>
//
//! Extends class #uvm_sequencer_base with an API depending on specific
//! request (REQ) and response (RSP) types.
//----------------------------------------------------------------------

template <typename REQ = uvm_sequence_item, typename RSP = REQ>
class uvm_sequencer_param_base : public uvm_sequencer_base
{
 public:
  tlm::tlm_fifo<REQ> m_req_fifo;
  //tlm::tlm_analysis_fifo<REQ> m_req_fifo; // TODO add analysis fifo

  uvm_analysis_export<RSP> rsp_export;

  uvm_sequencer_analysis_fifo<RSP> sqr_rsp_analysis_fifo;

  explicit uvm_sequencer_param_base( uvm_component_name name_ );
  virtual ~uvm_sequencer_param_base();

  void send_request(uvm_sequence_base* sequence_ptr,
                    uvm_sequence_item* seq_item,
                    bool rerandomize = false);

  REQ get_current_item() const;

  // Group: Requests

  int get_num_reqs_sent() const;
  void set_num_last_reqs(unsigned int max);
  unsigned int get_num_last_reqs() const;
  REQ* last_req(unsigned int n = 0);

  // Group: Responses

  int get_num_rsps_received() const;
  void set_num_last_rsps(unsigned int max);
  unsigned int get_num_last_rsps() const;
  RSP* last_rsp(unsigned int n = 0);

  /////////////////////////////////////////////////////
  // Implementation-defined member functions below,
  // not part of UVM Class reference / LRM
  /////////////////////////////////////////////////////

  virtual const char* kind() const; // SystemC API
  virtual const std::string get_type_name() const;
  void put_response_base( const RSP& rsp );
  void m_last_req_push_front( const REQ& item );
  void m_last_rsp_push_front( const RSP& item );

  virtual void connect_phase( uvm_phase& phase );
  virtual void build_phase( uvm_phase& phase );

 private:
  // class data members
  unsigned int m_num_reqs_sent;
  unsigned int m_num_last_reqs;
  unsigned int m_num_last_rsps;
  unsigned int m_num_rsps_received;


  typedef std::list<REQ*> m_last_req_buffer_listT;
  typedef typename m_last_req_buffer_listT::iterator m_last_req_buffer_list_ItT;
  m_last_req_buffer_listT m_last_req_buffer;

  typedef std::list<RSP*> m_last_rsp_buffer_listT;
  typedef typename m_last_rsp_buffer_listT::iterator m_last_rsp_buffer_list_ItT;
  m_last_rsp_buffer_listT m_last_rsp_buffer;
};

/////////////////////////////////////////////
// Class implementation starts here
/////////////////////////////////////////////

//----------------------------------------------------------------------
// constructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequencer_param_base<REQ,RSP>::uvm_sequencer_param_base( uvm_component_name name_ )
  : uvm_sequencer_base( name_ ),
    m_req_fifo("m_req_fifo", UVM_MAX_SEQS), // set fifo depth here
    rsp_export("rsp_export"),
    sqr_rsp_analysis_fifo("sqr_rsp_analysis_fifo")
{
  m_num_reqs_sent = 0;
  m_num_last_reqs = 1;
  m_num_last_rsps = 0;
  m_num_rsps_received = 0;

  m_last_req_buffer.clear();
  m_last_rsp_buffer.clear();

  // TODO: underlying SC TLM fifo's does not implement the method print_enabled
  //sqr_rsp_analysis_fifo.print_enabled = false;
  //m_req_fifo.print_enabled = false;
}

//----------------------------------------------------------------------
// destructor
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
uvm_sequencer_param_base<REQ,RSP>::~uvm_sequencer_param_base()
{
  for (m_last_req_buffer_list_ItT
       it = m_last_req_buffer.begin();
       it != m_last_req_buffer.end();
       it++ )
    delete *it;

  for (m_last_rsp_buffer_list_ItT
       it = m_last_rsp_buffer.begin();
       it != m_last_rsp_buffer.end();
       it++ )
    delete *it;
}

//----------------------------------------------------------------------
// connect_phase (virtual)
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::connect_phase(uvm_phase& phase)
{
  uvm_sequencer_base::connect_phase(phase);
  rsp_export.connect(sqr_rsp_analysis_fifo.analysis_export);
}

//----------------------------------------------------------------------
// build_phase (virtual)
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::build_phase(uvm_phase& phase)
{
  uvm_sequencer_base::build_phase(phase);
  sqr_rsp_analysis_fifo.sequencer_ptr = this;
}

//----------------------------------------------------------------------
// member function: send_request
//
//! The send_request function may only be called after a #wait_for_grant call.
//! This call will send the request item, \p seq_item, to the sequencer pointed to by
//! \p sequence_ptr. The sequencer will forward it to the driver. If \p rerandomize
//! is set, the item will be randomized before being sent to the driver.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::send_request( uvm_sequence_base* sequence_ptr,
                                                      uvm_sequence_item* seq_item,
                                                      bool rerandomize)
{
  REQ* param_tp;
  REQ param_t;

  if (sequence_ptr == NULL)
    uvm_report_fatal("SNDREQ", "Send request sequence_ptr is null", UVM_NONE);

  if (sequence_ptr->m_wait_for_grant_semaphore < 1)
    uvm_report_fatal("SNDREQ", "Send request called without wait_for_grant", UVM_NONE);

  sequence_ptr->m_wait_for_grant_semaphore--;

  param_tp = dynamic_cast<REQ*>(seq_item);

  if (param_tp != NULL)
  {
    param_t = *param_tp; // TODO workaround: dereference to get const REQ& for the TLM put. Alternative?

    if (rerandomize)
    {
      // TODO no randomization for UVM-SC yet
      // report error is dummy for now
      uvm_report_error("SEQR-RND", "Randomization not implemented and thus skipped", UVM_NONE);

      //if (!param_t->randomize())
      //  uvm_report_warning("SQRSNDREQ", "Failed to rerandomize sequence item in send_request");
    }

    if (param_t.get_transaction_id() == -1)
      param_t.set_transaction_id(sequence_ptr->m_next_transaction_id++);
    
    param_t.set_sequence_id(sequence_ptr->m_get_sqr_sequence_id(m_sequencer_id, 1)); //TODO no direct access to m_get_sqr_sequence_id

    m_last_req_push_front(param_t);
  }
  else
    uvm_report_fatal(name(),"Send_request failed to cast sequence item", UVM_NONE);

  seq_item->set_sequencer(this);

  if (!m_req_fifo.nb_put(param_t))
  {
    std::ostringstream str;
    str << "Concurrent calls to send_request() not supported. Check your driver for concurrent calls to get_next_item()";
    uvm_report_fatal(name(), str.str(), UVM_NONE);
  }

  m_num_reqs_sent++;

  // Grant any locks as soon as possible
  grant_queued_locks();
}

//----------------------------------------------------------------------
// member function: get_current_item
//
//! Returns the request_item currently being executed by the sequencer. If the
//! sequencer is not currently executing an item, this method will return NULL.
//!
//! The sequencer is executing an item from the time that get_next_item or peek
//! is called until the time that get or item_done is called.
//!
//! Note that a driver that only calls get() will never show a current item,
//! since the item is completed at the same time as it is requsted.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ uvm_sequencer_param_base<REQ,RSP>::get_current_item() const
{
  REQ req;
  if (!m_req_fifo.nb_peek(req))
    return NULL;
  return req;
}

//----------------------------------------------------------------------
// member function: get_num_reqs_sent
//
//! Returns the number of requests that have been sent by this sequencer.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
int uvm_sequencer_param_base<REQ,RSP>::get_num_reqs_sent() const
{
  return m_num_reqs_sent;
}

//----------------------------------------------------------------------
// member function: set_num_last_reqs
//
//! Sets the size of the last_requests buffer.  Note that the maximum buffer
//! size is 1024.  If max is greater than 1024, a warning is issued, and the
//! buffer is set to 1024.  The default value is 1.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::set_num_last_reqs(unsigned int max)
{
  if(max > 1024)
  {
    std::ostringstream msg;
    msg << "Invalid last size; 1024 is the maximum and will be used";
    uvm_report_warning("HSTOB", msg.str(), UVM_NONE);
    max = 1024;
  }

  // shrink the buffer
  while((m_last_req_buffer.size() != 0) && (m_last_req_buffer.size() > max))
  {
    delete m_last_req_buffer.back();
    m_last_req_buffer.pop_back();
  }

  m_num_last_reqs = max;
}

//----------------------------------------------------------------------
// member function: get_num_last_reqs
//
//! Returns the size of the last requests buffer, as set by set_num_last_reqs.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
unsigned int uvm_sequencer_param_base<REQ,RSP>::get_num_last_reqs() const
{
  return m_num_last_reqs;
}

//----------------------------------------------------------------------
// member function: last_req
//
//! Returns the last request item by default.  If n is not 0, then it will get
//! the n-th before last request item.  If n is greater than the last request
//! buffer size, the member function will return NULL.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
REQ* uvm_sequencer_param_base<REQ,RSP>::last_req(unsigned int n)
{
  if(n > m_num_last_reqs)
  {
    std::ostringstream msg;
    msg << "Invalid last access " << n
        << ", the max history is " << m_num_last_reqs;
    uvm_report_warning("HSTOB", msg.str(), UVM_NONE);
    return NULL;
  }

  if(n >= m_last_req_buffer.size())
  {
    return NULL;
  }

  m_last_req_buffer_list_ItT it = m_last_req_buffer.begin();
  std::advance(it, n);
  return *it;
}

//----------------------------------------------------------------------
// member function: get_num_rsps_received
//
//! Returns the number of responses received thus far by this sequencer.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
int uvm_sequencer_param_base<REQ,RSP>::get_num_rsps_received() const
{
  return m_num_rsps_received;
}

//----------------------------------------------------------------------
// member function: set_num_last_rsps
//
//! Sets the size of the last_responses buffer.  The maximum buffer size is
//! 1024. If max is greater than 1024, a warning is issued, and the buffer is
//! set to 1024.  The default value is 1.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::set_num_last_rsps(unsigned int max)
{
  if(max > 1024)
  {
    std::ostringstream msg;
    msg << "Invalid last size; 1024 is the maximum and will be used";
    uvm_report_warning("HSTOB", msg.str(), UVM_NONE);
    max = 1024;
  }

  // shrink the buffer
  while((m_last_rsp_buffer.size() != 0) && (m_last_rsp_buffer.size() > max))
  {
    delete m_last_rsp_buffer.back();
    m_last_rsp_buffer.pop_back();
  }

  m_num_last_rsps = max;
}

//----------------------------------------------------------------------
// member function: get_num_last_rsps
//
//! Returns the max size of the last responses buffer, as set by
//! set_num_last_rsps.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
unsigned int uvm_sequencer_param_base<REQ,RSP>::get_num_last_rsps() const
{
  return m_num_last_rsps;
}

//----------------------------------------------------------------------
// member function: last_rsp
//
//! Returns the last response item by default.  If n is not 0, then it will
//! get the nth-before-last response item.  If n is greater than the last
//! response buffer size, the function will return NULL.
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
RSP* uvm_sequencer_param_base<REQ,RSP>::last_rsp(unsigned int n)
{
  if(n > m_num_last_rsps)
  {
    std::ostringstream msg;
    msg << "Invalid last access " << n
        << ", the max history is " << m_num_last_rsps;
    uvm_report_warning("HSTOB", msg.str(), UVM_NONE);
    return NULL;
  }

  if(n >= m_last_rsp_buffer.size())
  {
    return NULL;
  }

  m_last_rsp_buffer_list_ItT it = m_last_rsp_buffer.begin();
  std::advance(it, n);
  return *it;
}

////////////////////////////////////////////////////////////////////////
//////// Implementation-defined member functions start here ////////////
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
// member function: put_response_base
//
// Implementation-defined member function
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::put_response_base(const RSP& rsp)
{
  uvm_sequence_base* sequence_ptr;

  m_last_rsp_push_front(rsp);
  m_num_rsps_received++;

  // Check that set_id_info() was called
  if (rsp.get_sequence_id() == -1)
    uvm_report_fatal("SQRPUT", "Driver put a response with invalid sequence_id", UVM_NONE);

  sequence_ptr = m_find_sequence(rsp.get_sequence_id());

  if (sequence_ptr != NULL)
  {
    // If the response_handler is enabled for this sequence,
    // then call the response handler
    if ( sequence_ptr->get_use_response_handler() )
    {
      sequence_ptr->response_handler(&rsp);
      return;
    }
    sequence_ptr->put_response(rsp);
  }
  else
  {
    std::ostringstream str;
    str << "Dropping response for sequence '" << rsp.get_name() << "' (id= " << rsp.get_sequence_id() << "), since sequence is not found. Probable cause: sequence exited or has been killed.";
    uvm_report_info("Sequencer", str.str() );
  }
}

//----------------------------------------------------------------------
// member function: m_last_rsp_push_front
//
// Implementation-defined member function
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::m_last_rsp_push_front(const RSP& item)
{
  if(!m_num_last_rsps)
    return;

  if(m_last_rsp_buffer.size() == m_num_last_rsps)
  {
    delete m_last_rsp_buffer.back();
    m_last_rsp_buffer.pop_back();
  }

  RSP* rsp = new RSP(item.get_name());
  *rsp = item;
  m_last_rsp_buffer.push_front(rsp);
}


//----------------------------------------------------------------------
// member function: m_last_req_push_front
//
// Implementation-defined member function
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
void uvm_sequencer_param_base<REQ,RSP>::m_last_req_push_front(const REQ& item)
{
  if(!m_num_last_reqs)
    return;

  if(m_last_req_buffer.size() == m_num_last_reqs)
  {
    delete m_last_req_buffer.back();
    m_last_req_buffer.pop_back();
  }

  REQ* req = new REQ(item.get_name());
  *req = item;
  m_last_req_buffer.push_front(req);
}

//----------------------------------------------------------------------
// member function: kind (virtual)
//
// SystemC API
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
const char* uvm_sequencer_param_base<REQ,RSP>::kind() const
{
  return "uvm::uvm_sequencer_param_base";
}

//----------------------------------------------------------------------
// member function: get_type_name (virtual)
//
// Implementation-defined member function
//----------------------------------------------------------------------

template <typename REQ, typename RSP>
const std::string uvm_sequencer_param_base<REQ,RSP>::get_type_name() const
{
  return std::string(this->kind());
}


} /* namespace uvm */

#endif /* UVM_SEQUENCER_PARAM_BASE_H_ */
