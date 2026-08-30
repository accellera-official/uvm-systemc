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

const unsigned int num_seqs = 1000;
const unsigned int num_trans_per_seq = 10000;

class env: public uvm::uvm_env
{
 public:
  // instances
  my_sequencer<bus_trans, bus_trans>* sqr;
  my_driver<bus_trans, bus_trans>*    drv;

  // sequence
  sequenceA<bus_trans, bus_trans>* sequence_a = nullptr;

  UVM_COMPONENT_UTILS(env);

  env( uvm::uvm_component_name name ) : uvm::uvm_env( name ), sqr(0), drv(0) {}

  void build_phase(uvm::uvm_phase& phase) override
  {
    uvm::uvm_env::build_phase(phase);

    sqr = my_sequencer<bus_trans, bus_trans>::type_id::create("sequence_controller", this);
    drv = my_driver<bus_trans, bus_trans>::type_id::create("slave", this);


  }

  void connect_phase(uvm::uvm_phase& phase) override
  {
    drv->seq_item_port(sqr->seq_item_export);
  }

  void run_phase(uvm::uvm_phase& phase) override
  {
	phase.raise_objection(this);
	for (unsigned int i = 0; i < num_seqs; i++)
	{
		sequence_a = sequenceA<bus_trans, bus_trans>::type_id::create("sequenceA" + std::to_string(i));
		sequence_a->set_num_loops(num_trans_per_seq);
		sequence_a->start(sqr, nullptr);
		sequenceA<bus_trans, bus_trans>::type_id::destroy(sequence_a);
	}
	phase.drop_objection(this);
  }


};

#endif /* ENV_H_ */
