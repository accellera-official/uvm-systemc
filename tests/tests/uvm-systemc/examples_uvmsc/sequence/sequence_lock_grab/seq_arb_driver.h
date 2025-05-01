//------------------------------------------------------------------------------
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2013-2014 NXP B.V.
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

#ifndef SEQ_ARB_DRIVER_H_
#define SEQ_ARB_DRIVER_H_

#include <uvm>
#include <systemc>
#include "seq_arb_item.h"

// Receives sequence items the sequences and and keeps a running total

class seq_arb_driver : public uvm::uvm_driver<seq_arb_item>
{
 public:
  UVM_COMPONENT_UTILS(seq_arb_driver);

  // Counters to keep track of sequence accesses
  int seq_1;
  int seq_2;
  int seq_3;
  int seq_4;
  int grab_seq;
  int lock_seq;

  seq_arb_driver( uvm::uvm_component_name name ) : uvm::uvm_driver<seq_arb_item>(name)
  {
    seq_1 = 0;
    seq_2 = 0;
    seq_3 = 0;
    seq_4 = 0;
    grab_seq = 0;
    lock_seq = 0;
  }

  void run_phase(uvm::uvm_phase& phase) override
  {

    seq_arb_item req;
    seq_arb_item rsp;

    while(1)
    {
      seq_item_port->get(req);

      switch(req.seq_no)
      {
        case 1: seq_1++; break;
        case 2: seq_2++; break;
        case 3: seq_3++; break;
        case 4: seq_4++; break;
        case 5: grab_seq++; break;
        case 6: lock_seq++; break;
        default: break;
      }

      std::ostringstream info;
      info << "Access totals: SEQ_1:" << seq_1
           << " SEQ_2:" << seq_2
           << " SEQ_3:" << seq_3
           << " SEQ_4:" << seq_4
           << " GRAB:" << grab_seq
           << " LOCK:" << lock_seq;
      uvm_report_info("RECEIVED_SEQ", info.str());
      sc_core::wait(10, sc_core::SC_MS);
    }
  }

}; // class seq_arb_driver

#endif // SEQ_ARB_DRIVER_H_
