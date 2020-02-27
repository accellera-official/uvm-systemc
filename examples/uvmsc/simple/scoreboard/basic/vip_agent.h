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

#ifndef VIP_AGENT_H_
#define VIP_AGENT_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "vip_sequencer.h"
#include "vip_driver.h"
#include "vip_monitor.h"
#include "vip_packet.h"

class vip_agent : public uvm::uvm_agent
{
 public:
  vip_sequencer<vip_packet>* sequencer;
  vip_driver<vip_packet>*    driver;
  vip_monitor*               monitor;

  vip_agent( uvm::uvm_component_name name )
    : uvm_agent(name), sequencer(0), driver(0), monitor(0)
  {
    std::cout << sc_core::sc_time_stamp() << ": constructor " << name << std::endl;
  }

  void build_phase(uvm::uvm_phase& phase)
  {
    std::cout << sc_core::sc_time_stamp() << ": build_phase " << name() << std::endl;

    uvm::uvm_agent::build_phase(phase);

    if (get_is_active() == uvm::UVM_ACTIVE)
    {
      UVM_INFO(get_name(), "is set to UVM_ACTIVE", UVM_NONE);

      sequencer = vip_sequencer<vip_packet>::type_id::create("sequencer", this);
      assert(sequencer);

      driver = vip_driver<vip_packet>::type_id::create("driver", this);
      assert(driver);
    }
    else
      UVM_INFO(get_name(), "is set to UVM_PASSIVE", UVM_NONE);

    monitor = vip_monitor::type_id::create("monitor", this);
    assert(monitor);
  }


  void connect_phase(uvm::uvm_phase& phase)
  {
    std::cout << sc_core::sc_time_stamp() << ": connect_phase " << name() << std::endl;

    if (get_is_active() == uvm::UVM_ACTIVE)
    {
      driver->seq_item_port.connect(sequencer->seq_item_export);
    }
  }

  UVM_COMPONENT_UTILS(vip_agent);

};

#endif /* VIP_AGENT_H_ */
