//----------------------------------------------------------------------
//   Copyright 2013-2025 NXP B.V.
//   Copyright 2011 Synopsys, Inc.
//   All Rights Reserved
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

#ifndef TB_ENV_H_
#define TB_ENV_H_

#include <systemc>
#include <uvm>

class tb_env : public uvm::uvm_env
{
 public:

  UVM_COMPONENT_UTILS(tb_env);

  tb_env(uvm::uvm_component_name name) : uvm::uvm_env(name) {}

  void build_phase(uvm::uvm_phase& phase) override
  {
    uvm::uvm_config_db<sc_core::sc_time>::set(nullptr, "global_timer.*",    "timeout", sc_core::sc_time(1000, sc_core::SC_US));
    uvm::uvm_config_db<sc_core::sc_time>::set(nullptr, "global_timer.main", "timeout", sc_core::sc_time(3000, sc_core::SC_US));
    uvm::uvm_config_db<sc_core::sc_time>::set(nullptr, "global_timer.run",  "timeout", sc_core::sc_time(0, sc_core::SC_US));
  }
   
  void reset_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    sc_core::wait(20, sc_core::SC_US);
    phase.drop_objection(this);
  }
   
  void configure_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    sc_core::wait(200, sc_core::SC_US);
    phase.drop_objection(this);
  }
   
  void main_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    sc_core::wait(1000, sc_core::SC_US);
    phase.drop_objection(this);
  }

  void shutdown_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    sc_core::wait(10, sc_core::SC_US);
    phase.drop_objection(this);
  }
};

#endif /* TB_ENV_H_ */
