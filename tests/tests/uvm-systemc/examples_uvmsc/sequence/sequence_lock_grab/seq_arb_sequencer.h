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

#ifndef SEQ_ARB_SEQUENCER_H_
#define SEQ_ARB_SEQUENCER_H_

#include <uvm>
#include <systemc>

#include "seq_arb_item.h"

// This sequencer implementation contains an overloaded user_priority_arbitration method
// to illustrate how to implement the SEQ_ARB_USER approach

class seq_arb_sequencer : public uvm::uvm_sequencer<seq_arb_item>
{
 public:

  UVM_COMPONENT_UTILS(seq_arb_sequencer);

  seq_arb_sequencer( uvm::uvm_component_name name ) : uvm::uvm_sequencer<seq_arb_item>(name) {}

  // This method overrides the default user method
  // It returns the last item in the sequence queue rather than the first


  int user_priority_arbitration(std::vector<int> avail_sequences) override
  {
    int end_index;
    end_index = avail_sequences.size() - 1;
    return (avail_sequences[end_index]);
  } // user_priority_arbitration

}; // class seq_arb_sequencer

#endif // SEQ_ARB_SEQUENCER_H_
