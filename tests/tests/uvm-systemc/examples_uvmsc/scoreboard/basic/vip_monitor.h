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

#ifndef VIP_MONITOR_H_
#define VIP_MONITOR_H_

#include <systemc>
#include <tlm.h>
#include <uvm>

#include "vip_if.h"

class vip_monitor : public uvm::uvm_monitor
{
 public:
  uvm::uvm_analysis_port<vip_packet> item_collected_port;

  vip_if* vif;
  bool checks_enable;
  bool coverage_enable;

  vip_monitor(uvm::uvm_component_name name)
  : uvm_monitor(name),
    item_collected_port("item_collected_port"),
    vif(0),
    checks_enable(false),
    coverage_enable(false)
  {}

  UVM_COMPONENT_UTILS(vip_monitor);

  void build_phase(uvm::uvm_phase& phase) override
  {
    std::cout << sc_core::sc_time_stamp() << ": build_phase " << name() << std::endl;

    uvm::uvm_monitor::build_phase(phase);

    if (!uvm::uvm_config_db<vip_if*>::get(this, "*", "vif", vif))
      UVM_FATAL(name(), "Virtual interface not defined! Simulation aborted!");

    uvm::uvm_config_db<bool>::get(this, "*", "checks_enable", checks_enable);
    uvm::uvm_config_db<bool>::get(this, "*", "coverage_enable", coverage_enable);
  }

  void run_phase( uvm::uvm_phase& phase ) override
  {
    vip_packet p;

    while (true) // monitor forever
    {
      sc_core::wait( vif->sig_data.default_event() ); // wait for input changes
      p.data = vif->sig_data.read();
      std::cout << sc_core::sc_time_stamp() << ": " << name() << " received " << p.data << std::endl;
      item_collected_port.write(p);

      if(checks_enable) { std::cout << "no checks yet" << std::endl; }
      if(coverage_enable) { std::cout << "no checks yet" << std::endl; }
    }
  }
};

#endif /* VIP_MONITOR_H_ */
