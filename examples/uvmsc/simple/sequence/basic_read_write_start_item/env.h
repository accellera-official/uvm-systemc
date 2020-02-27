//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

#ifndef ENV_H_
#define ENV_H_

#include <systemc>
#include "sysc/kernel/sc_dynamic_processes.h"
#include <tlm.h>
#include <uvm>

#include "sequenceA.h"
#include "bus_trans.h"
#include "my_driver.h"
#include "my_sequencer.h"

#define NUM_SEQS 10

class env: public uvm::uvm_env
{
 public:
  // instances
  my_sequencer<bus_req, bus_rsp>* sqr;
  my_driver<bus_req, bus_rsp>*    drv;

  // sequence
  sequenceA<bus_req, bus_rsp>* sequence_a[NUM_SEQS];

  UVM_COMPONENT_UTILS(env);

  env( uvm::uvm_component_name name ) : uvm::uvm_env( name ), sqr(0), drv(0) {}

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm::uvm_env::build_phase(phase);

    sqr = my_sequencer<bus_req, bus_rsp>::type_id::create("sequence_controller", this);
    drv = my_driver<bus_req, bus_rsp>::type_id::create("slave", this);

    for (int i = 0; i < NUM_SEQS; i++)
    {
      std::ostringstream str;
      str << "sequenceA" << i;
      sequence_a[i] = new sequenceA<bus_req, bus_rsp>(str.str());
    }
  }

  void connect_phase(uvm::uvm_phase& phase)
  {
    drv->seq_item_port(sqr->seq_item_export);
  }

  void run_phase(uvm::uvm_phase& phase)
  {
    phase.raise_objection(this);
    SC_FORK
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 0)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 1)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 2)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 3)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 4)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 5)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 6)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 7)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 8)),
      sc_core::sc_spawn(sc_bind(&env::start_sequence, this, 9))
    SC_JOIN
    phase.drop_objection(this);
  }

  void start_sequence(int n)
  {
    sequence_a[n]->start(sqr, NULL);
  }

  virtual ~env()
  {
    for (int i = 0; i < NUM_SEQS; i++)
    {
      delete sequence_a[i];
    }
  }
};

#endif /* ENV_H_ */
