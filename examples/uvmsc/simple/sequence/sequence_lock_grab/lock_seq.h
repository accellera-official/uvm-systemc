//------------------------------------------------------------------------------
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2020 Bosch Sensortec GmbH
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
//------------------------------------------------------------------------------
//
// The sequence which sends sequence items - four of these are run in parallel

#ifndef LOCK_SEQ_H_
#define LOCK_SEQ_H_

#include <uvm>
#include <systemc>

#include "seq_arb_item.h"

class lock_seq : public uvm::uvm_sequence<seq_arb_item>
{
 public:
  UVM_OBJECT_UTILS(lock_seq);

  int seq_no;

  lock_seq( const std::string& name = "lock_seq" )
    : uvm::uvm_sequence<seq_arb_item>(name), seq_no(0)
  {}

  void body()
  {
    seq_arb_item* req;

    if(m_sequencer->is_blocked(this))
      uvm::uvm_report_info("lock_seq", "This sequence is blocked by an existing lock");
    else
      uvm::uvm_report_info("lock_seq", "This sequence is not blocked by an existing lock");

    // Lock call - which blocks until it is granted
    m_sequencer->lock(this);

    if(m_sequencer->is_grabbed())
      if(m_sequencer->current_grabber() != this)
        uvm::uvm_report_info("lock_seq", "Lock sequence waiting for current grab or lock to complete");

    req = seq_arb_item::type_id::create("req");
    req->seq_no = 6;

    for(int i=0; i<4; i++)
    {
      start_item(req);
      finish_item(req);
    }

    // Unlock call - must be issued
    m_sequencer->unlock(this);

  } // body

};

#endif // LOCK_SEQ_H_
