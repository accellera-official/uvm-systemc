//---------------------------------------------------------------------- 
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2015 NXP B.V.
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

// This test creates a simple hierarchy and makes sure that phases
// start at the correct time. The env has no delays in it so all
// phase processed in the env end immediately.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

//global variables
std::map<uvm_phase*, bool> phase_run;
bool failed = false;

class base : public uvm_component
{
 public:
  bool do_delay;

  base( uvm_component_name name)
  : uvm_component(name), do_delay(true)
  {
  }

  void build_phase(uvm_phase& phase) override
  {
    phase_run[uvm_build_phase::get()] = true;
    UVM_INFO("BUILD", "Starting Build", UVM_NONE);
    if(sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("BUILD", "Expected Build start time of 0s");
    }
    UVM_INFO("BUILD", "Ending Build", UVM_NONE);
  }

  void reset_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_reset_phase::get()] = true;
    UVM_INFO("RESET", "Starting Reset", UVM_NONE);
    if(sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RESET", "Expected Reset start time of 0s");
    }
    if(do_delay)
      wait(100, SC_MS);

    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this);
  }

  void main_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_main_phase::get()] = true;
    UVM_INFO("MAIN", "Starting Main", UVM_NONE);
    if(sc_time_stamp() != sc_time(100, SC_MS))
    {
      failed = true;
      UVM_ERROR("MAIN", "Expected Main start time of 100ms");
    }
    if(do_delay)
      wait(100, SC_MS);
    UVM_INFO("MAIN", "Ending Main", UVM_NONE);
    phase.drop_objection(this);
  }

  void run_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_run_phase::get()] = true;
    UVM_INFO("RUN", "Starting Run", UVM_NONE);
    if(sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RUN", "Expected Run start time of 0s");
    }
    if(do_delay)
      wait(1000, SC_MS);

    UVM_INFO("RUN", "Ending Run", UVM_NONE);
    phase.drop_objection(this);
  }

  void extract_phase(uvm_phase& phase) override
  {
    phase_run[uvm_extract_phase::get()] = true;
    UVM_INFO("EXTRACT", "Starting Extract", UVM_NONE);
    if(sc_time_stamp() != sc_time(1000, SC_MS))
    {
      failed = true;
      UVM_ERROR("extract", "Expected extract start time of 1000ms");
    }
    UVM_INFO("EXTRACT", "Ending Extract", UVM_NONE);
  }

}; // class base

class leaf : public base
{
 public:
  leaf(uvm_component_name name)
  : base(name) {}

}; // class leaf

class test : public base
{
 public:
  leaf l1, l2;

  UVM_COMPONENT_UTILS(test);

  test(uvm_component_name name) : base(name), l1("l1"), l2("l2")
  {
    do_delay = false;
  }

  void report_phase(uvm_phase& phase) override
  {
    phase_run[uvm_report_phase::get()] = true;
    if(phase_run.size() != 6)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 6 phases, got " << phase_run.size();
      UVM_ERROR("NUMPHASES", str.str());
    }

    if (failed)
      std::cout << "*** UVM TEST FAILED ***" << std::endl;
    else
      std::cout << "*** UVM TEST PASSED ***" << std::endl;
  }
}; // class test


int sc_main(int, char*[])
{
  test test1("test1");
  run_test();
  return 0;
}
