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

#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "vip_agent.h"
#include "vip_packet.h"
#include "scoreboard.h"
#include "sequence.h"

class testbench : public uvm::uvm_env
{
 public:
  // instances
  vip_agent* agent1;
  vip_agent* agent2;
  scoreboard* scoreboard0;

  UVM_COMPONENT_UTILS(testbench);

  testbench( uvm::uvm_component_name name) : uvm::uvm_env(name)
  {
   std::cout << sc_core::sc_time_stamp() << ": constructor " << name << std::endl;
  }

  void build_phase(uvm::uvm_phase& phase)
  {
    std::cout << sc_core::sc_time_stamp() << ": build_phase " << name() << std::endl;

    uvm::uvm_env::build_phase(phase);

    agent1 = vip_agent::type_id::create("agent1", this);
    assert(agent1);

    agent2 = vip_agent::type_id::create("agent2", this);
    assert(agent2);

    scoreboard0 = scoreboard::type_id::create("scoreboard0", this);
    assert(scoreboard0);

    uvm::uvm_config_db<int>::set(this, "agent1", "is_active", uvm::UVM_ACTIVE);
    uvm::uvm_config_db<int>::set(this, "agent2", "is_active", uvm::UVM_PASSIVE);

    uvm::uvm_config_db<uvm_object_wrapper*>
      ::set(this,"agent1.sequencer.run_phase","default_sequence",
      sequence<vip_packet>::type_id::get());
  }

  void connect_phase(uvm::uvm_phase& phase)
  {
    std::cout << sc_core::sc_time_stamp() << ": connect_phase " << name() << std::endl;

    agent1->monitor->item_collected_port.connect(scoreboard0->xmt_listener_imp);
    agent2->monitor->item_collected_port.connect(scoreboard0->rcv_listener_imp);
  }

};

#endif /* TESTBENCH_H_ */
