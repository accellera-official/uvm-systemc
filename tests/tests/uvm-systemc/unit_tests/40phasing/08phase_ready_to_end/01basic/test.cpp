//------------------------------------------------------------------------------
//   Copyright 2015-2021 NXP.B.V.
//   Copyright 2011-2018 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2017 NVIDIA Corporation
//   Copyright 2012 Cisco Systems, Inc.
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

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

class passive_comp : public uvm_component
{
 public:
  bool busy;
  bool ending;

  UVM_COMPONENT_UTILS(passive_comp);

  passive_comp(uvm_component_name name)
  : uvm_component(name), busy(false), ending(false)
  {}

  void main_phase(uvm_phase& phase) override
  {
    UVM_INFO(get_name(), "main thread started...", UVM_LOW);
    while (!ending)
    {
      busy = true;

      UVM_INFO(get_name(), "main phase busy", UVM_LOW);
      wait(10, SC_SEC);
      UVM_INFO(get_name(), "main phase not busy", UVM_LOW);

      busy = false;
      if (ending)
        phase.drop_objection(this, "ok, i'm ready");

      uvm_wait_for_nba_region();
    }
    UVM_INFO(get_name(), "main thread completed...", UVM_LOW);
  }

  void phase_ready_to_end(uvm_phase& phase) override
  {
    if (phase.get_name() == "main")
    {
      UVM_INFO(get_name(), "phase ready to end for main phase", UVM_LOW);
      ending = true;
      if (busy)
      {
        phase.raise_objection(this, "not ready to end");
        UVM_INFO(get_name(), "re-raised", UVM_LOW);
      }
    }
  }
}; // class passive_comp


class active_comp : public uvm_component
{
public:
  bool called;
  bool failed;

  active_comp(uvm_component_name name)
  : uvm_component(name), called(false), failed(false) {}

  // normally wouldn't raise/drop each iter, but want to cause iter on read_to_end
  void main_phase(uvm_phase& phase) override
  {
    UVM_INFO(get_name(), "main thread started...", UVM_LOW);

    UVM_INFO(get_name(), "raise objection...", UVM_LOW);
    phase.raise_objection(this);

    wait(7, SC_SEC);

    UVM_INFO(get_name(), "drop objection...", UVM_LOW);
    phase.drop_objection(this);

    UVM_INFO(get_name(), "main thread completed...", UVM_LOW);
  }

  void objection_thread(uvm_phase* phase)
  {
    UVM_INFO(get_name(), "raise objection...", UVM_LOW);
    phase->raise_objection(this);

    wait(7, SC_SEC);

    UVM_INFO(get_name(), "drop objection...", UVM_LOW);
    phase->drop_objection(this);
  }

  void phase_ready_to_end(uvm_phase& phase) override
  {
    if (phase.get_name() == "main" && !called)
    {
      UVM_INFO(get_name(), "phase ready to end for main phase", UVM_LOW);
      sc_spawn(sc_bind(&active_comp::objection_thread, this, &phase));
      called = true;
    }
  }

  void extract_phase(uvm_phase& phase) override
  {
    UVM_INFO("EXTRACT START", "extract phase started...", UVM_LOW);
    if ( sc_time_stamp() != sc_time(14, SC_SEC))
    {
      std::ostringstream str;
      str << "extract() phase started at " << sc_time_stamp() << " instead of 14 s.";
      UVM_ERROR("test", str.str());
      failed = true;
    }
  }

  void report_phase(uvm_phase& phase) override
  {
    if(failed)
      std::cout << "*** UVM TEST FAILED ***" << std::endl;
    else
      std::cout << "*** UVM TEST PASSED ***" << std::endl;  }
}; // class active_comp


class test : public uvm_test
{
 public:
  UVM_COMPONENT_UTILS(test);

  active_comp ac;
  passive_comp pc;

  test(uvm_component_name name)
  : uvm_test(name), ac("ac"), pc("pc")
  {}
};


int sc_main(int, char*[])
{
  run_test("test");
  return 0;
}
