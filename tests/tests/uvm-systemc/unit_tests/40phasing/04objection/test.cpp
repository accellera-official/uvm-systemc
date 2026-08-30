//---------------------------------------------------------------------- 
//   Copyright 2010-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2011 Synopsys, Inc.
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

// This test creates a simple hierarchy and makes sure that a
// component can create objections to phase processing independent
// of the implicit objections.
//
// The top component executes a run phase that raises objections for
// three phases and drops the objections at desired times to let
// the phasing move forward.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

//global variables
int phase_transition_time = 1000;
std::map<uvm_phase*, bool> phase_run;
bool failed = false;

class base : public uvm_component
{
 public:
  bool do_delay;

  base( uvm_component_name name)
  : uvm_component(name), do_delay(true) {}

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
      std::ostringstream str;
      str << "Expected Reset start time of 0s, got "
          << phase_transition_time
          << " its now "
          << sc_time_stamp();
      UVM_ERROR("RESET", str.str());
    }
    wait(100, SC_MS);
    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this);
  }

  void main_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_main_phase::get()] = true;
    UVM_INFO("MAIN", "Starting Main", UVM_NONE);
    // Even though there is not configure phase, the test is holding
    // up the configure phase.
    if(sc_time_stamp() != sc_time(2*phase_transition_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected main start time of "
          << 2*phase_transition_time
          << "ms, got "
          << sc_time_stamp();
      UVM_ERROR("MAIN", str.str());
    }
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
    wait(100, SC_MS);
    UVM_INFO("RUN", "Ending Run", UVM_NONE);
    phase.drop_objection(this);
  }

  void extract_phase(uvm_phase& phase) override
  {
    phase_run[uvm_extract_phase::get()] = true;
    UVM_INFO("EXTRACT", "Starting Extract", UVM_NONE);
    // Even though there is not configure phase, the test is holding
    // up the configure phase.
    if(sc_time_stamp() != sc_time(3*phase_transition_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected extract start time of "
          << 3*phase_transition_time
          << "ms, but got "
          << sc_time_stamp();
      UVM_ERROR("EXTRACT", str.str());
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
  int phases_run;

  UVM_COMPONENT_UTILS(test);

  test(uvm_component_name name) : base(name), l1("l1"), l2("l2")
  {
    do_delay = false;
    phases_run = 0;
  }

  void connect_phase(uvm_phase& phase) override
  {}

  // Do objections to phases proceeding
  void run_phase(uvm_phase& phase) override
  {
    uvm_domain* uvm_d = uvm_domain::get_uvm_domain();
    uvm_phase* reset_p = uvm_d->find(uvm_reset_phase::get());
    uvm_phase* config_p = uvm_d->find(uvm_configure_phase::get());
    uvm_phase* main_p = uvm_d->find(uvm_main_phase::get());

    UVM_INFO("TEST_RUN", "Setting up objections to certain phases", UVM_NONE);

    // Do the raise, wait, drop for each phase
    SC_FORK
      sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, reset_p)),
      sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, config_p)),
      sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, main_p))
    SC_JOIN
    UVM_INFO("TEST_RUN", "Ending run phase", UVM_NONE);
  }

  void do_phase_test(uvm_phase* phase)
  {
    // Raise the objection
    phase->raise_objection(this, "test " + phase->get_name() + " objection");
    UVM_INFO("do_phase_test", "Phase " + phase->get_name() + " objection raised. Wait for phase to be started...", UVM_NONE);

    // Wait for phase to be started
    phase->wait_for_state(UVM_PHASE_EXECUTING, UVM_EQ);

    // Wait for the desired time
    UVM_INFO("do_phase_test", "Phase " + phase->get_name() + " started, now wait for 1000ms...", UVM_NONE);
    wait(phase_transition_time, SC_MS);

    // Drop the objection
    UVM_INFO("do_phase_test", "Phase " + phase->get_name() + " objection dropped", UVM_NONE);
    phase->drop_objection(this, "test " + phase->get_name() + " objection");
    ++phases_run;
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

    if(phases_run != 3)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 3 objection processes, got " << phases_run;
      UVM_ERROR("NUMOBJS", str.str());
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
