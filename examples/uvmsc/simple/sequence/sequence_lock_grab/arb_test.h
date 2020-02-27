//------------------------------------------------------------------------------
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2019 COSEDA Technologies GmbH
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

#ifndef ARB_TEST_H_
#define ARB_TEST_H_

#include <uvm>
#include <systemc>

#include "seq_arb_driver.h"
#include "seq_arb_sequencer.h"
#include "arb_example_seq.h"

// Overall test class that builds the test bench and sets the arbitration
// algorithm according to the ARB_TYPE plusarg

class arb_test : public uvm::uvm_component
{
 public:
  UVM_COMPONENT_UTILS(arb_test);

  seq_arb_driver*    m_driver;
  seq_arb_sequencer* m_sequencer;
  arb_example_seq*   m_seq;

  arb_test( uvm::uvm_component_name name ) : uvm::uvm_component(name) {}

  void build_phase(uvm::uvm_phase& phase)
  {
    m_driver = seq_arb_driver::type_id::create("m_driver", this);
    m_sequencer = seq_arb_sequencer::type_id::create("m_sequencer", this);
    m_seq = arb_example_seq::type_id::create("m_seq", this);
  }

  void connect_phase(uvm::uvm_phase& phase)
  {
    m_driver->seq_item_port.bind(m_sequencer->seq_item_export);
  }

  void run_phase(uvm::uvm_phase& phase)
  {
    int arb_type = 0;

    phase.raise_objection(this, "Starting Grab-Lock test");

    if (uvm::uvm_config_db<int>::get(this, "*", "arb_type", arb_type))
    {
      std::ostringstream str;
      str << "Sequencer Arbitration selected: " << uvm::uvm_seq_arb_name[arb_type];
      uvm_report_info("arb_test:", str.str());
    }
    else
      uvm_report_fatal("arb_test:", "The ARB_TYPE plusarg was not specified on the command line");

    switch(arb_type)
    {
      case uvm::SEQ_ARB_FIFO:          m_seq->arb_type = uvm::SEQ_ARB_FIFO; break;
      case uvm::SEQ_ARB_WEIGHTED:      m_seq->arb_type = uvm::SEQ_ARB_WEIGHTED; break;
      case uvm::SEQ_ARB_RANDOM:        m_seq->arb_type = uvm::SEQ_ARB_RANDOM; break;
      case uvm::SEQ_ARB_STRICT_FIFO:   m_seq->arb_type = uvm::SEQ_ARB_STRICT_FIFO; break;
      case uvm::SEQ_ARB_STRICT_RANDOM: m_seq->arb_type = uvm::SEQ_ARB_STRICT_RANDOM; break;
      case uvm::SEQ_ARB_USER:          m_seq->arb_type = uvm::SEQ_ARB_USER; break;
      default:                         m_seq->arb_type = uvm::SEQ_ARB_FIFO; break;
    }
    m_seq->start(m_sequencer);
    sc_core::wait(100, sc_core::SC_MS);
    phase.drop_objection(this, "Finishing arbitration test");
  }

}; // class arb_test

#endif // ARB_TEST_H_
