//----------------------------------------------------------------------
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2022 NXP B.V.
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

#ifndef UVM_SEQUENCER_ANALYSIS_FIFO_H_
#define UVM_SEQUENCER_ANALYSIS_FIFO_H_

#include <systemc>  // tlm::tlm_fifo
#include "uvmsc/seq/uvm_sequencer_base.h"
#include "uvmsc/tlm1/uvm_analysis_imp.h"

namespace uvm {

template <typename RSP = uvm_sequence_item>
class uvm_sequencer_analysis_fifo : public tlm::tlm_fifo<RSP>
{
public:
  uvm_analysis_imp<RSP, uvm_sequencer_analysis_fifo<RSP> > analysis_export;
  uvm_sequencer_base* sequencer_ptr;

  uvm_sequencer_analysis_fifo(std::string name_)
  : analysis_export("analysis_export", this)
  {}

  void write(const RSP& t)
  {
    if (sequencer_ptr == NULL)
      uvm_report_fatal("SEQRNULL", "The sequencer pointer is NULL when attempting a write", UVM_NONE);

    sequencer_ptr->analysis_write(t);
  }
};

} // namespace uvm

#endif // UVM_SEQUENCER_ANALYSIS_FIFO_H_
