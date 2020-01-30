//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2011 Synopsys, Inc.
//   All Rights Reserved
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License") you may not use this file except in
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

#include <systemc>
#include <uvm>

#ifndef TB_TIMER_H_
#define TB_TIMER_H_

class tb_timer : public uvm::uvm_component
{
public:

  tb_timer(uvm::uvm_component_name name) : uvm_component(name) { std::cout << "tb_timer" << std::endl;}

  void run_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "run", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in run phase");
    }
  }

  void pre_reset_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "pre_reset", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in pre_reset phase");
    }
  }

  void reset_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "reset", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in reset phase");
    }
  }

  void post_reset_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "post_reset", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in post_reset phase");
    }
  }

  void pre_configure_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "pre_configure", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in pre_configure phase");
    }
  }

  void configure_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "configure", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in configure phase");
    }
  }

  void post_configure_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "post_configure", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in post_configure phase");
    }
  }

  void pre_main_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "pre_main", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in pre_main phase");
    }
  }

  void main_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "main", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in main phase");
    }
  }

  void post_main_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "post_main", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in post_main phase");
    }
  }

  void pre_shutdown_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "pre_shutdown", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in pre_shutdown phase");
    }
  }

  void shutdown_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "shutdown", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in shutdown phase");
    }
  }

  void post_shutdown_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time t;
    if (uvm::uvm_config_db<sc_core::sc_time>::get(this, "post_shutdown", "timeout", t) &&
        t.to_seconds() > 0)
    {
      sc_core::wait(t);
      UVM_FATAL("TIMEOUT", "Time-out expired in post_shutdown phase");
    }
  }

  static void destroy()
  {
    if(m_global != NULL) 
    {
      delete m_global;
      m_global = NULL;
    }
  }

 private:
  static tb_timer* m_global;

};

tb_timer* tb_timer::m_global = new tb_timer("global_timer");

#endif /* TB_TIMER_H_ */
