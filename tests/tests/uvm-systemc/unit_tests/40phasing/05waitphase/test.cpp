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
// process can wait for specific states of a phase.
//
// The top component executes a run phase forks processes to
// wait for state transitions of the reset, main and shutdown
// phases.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

//global variables
int phase_transition_time = 300;
std::map<uvm_phase*, bool> phase_run;
bool failed = false;

class base : public uvm_component
{
 public:
  int delay;

  base( uvm_component_name name)
  : uvm_component(name), delay(100)
  {
    //uvm_set_verbosity_level(UVM_DEBUG);
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
      std::ostringstream str;
      str << "Expected Reset start time of 0s, got "
          << sc_time_stamp();
      UVM_ERROR("RESET", str.str());
    }
    wait(delay, SC_MS);
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
    if(sc_time_stamp() != sc_time(phase_transition_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected main start time of "
          << phase_transition_time
          << "ms, got "
          << sc_time_stamp();
      UVM_ERROR("MAIN", str.str());
    }
    wait(delay, SC_MS);
    UVM_INFO("MAIN", "Ending Main", UVM_NONE);
    phase.drop_objection(this);
  }

  void shutdown_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_shutdown_phase::get()] = true;
    UVM_INFO("SHUTDOWN", "Starting Shutdown", UVM_NONE);
    // Even though there is not configure phase, the test is holding
    // up the configure phase.
    if(sc_time_stamp() != sc_time(2*phase_transition_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected shutdown start time of "
          << 2*phase_transition_time
          << "ms, got "
          << sc_time_stamp();
      UVM_ERROR("SHUTDOWN", str.str());
    }
    wait(delay, SC_MS);
    UVM_INFO("SHUTDOWN", "Ending Shutdown", UVM_NONE);
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
    wait(100, SC_MS); // hardcoded, not delay, since it is also used in top/test
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
  int start_cnt;
  int end_cnt;

  UVM_COMPONENT_UTILS(test);

  test(uvm_component_name name) : base(name), l1("l1"), l2("l2")
  {
    phases_run = 0;
    start_cnt = 0;
    end_cnt = 0;
    l2.delay = phase_transition_time;
    //uvm_set_verbosity_level(UVM_DEBUG);
  }

  void connect_phase(uvm_phase& phase) override
  {
    //uvm_phase::m_phase_trace = true;
  }

  void run1_phase(uvm_phase& phase)
  {}

  // Start up the checkers
  void start_of_simulation_phase(uvm_phase& phase) override
  {
    uvm_domain* uvm_d = uvm_domain::get_uvm_domain();
    uvm_phase* reset_p = uvm_d->find(uvm_reset_phase::get());
    uvm_phase* main_p = uvm_d->find(uvm_main_phase::get());
    uvm_phase* shutdown_p = uvm_d->find(uvm_shutdown_phase::get());

    sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, reset_p, 0));
    sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, main_p, phase_transition_time));
    sc_core::sc_spawn(sc_bind(&test::do_phase_test, this, shutdown_p, 2*phase_transition_time));
  }

  void do_phase_test(uvm_phase* phase, int start_time)
  {
    // Phase must either be dormant or scheduled at the very start
    if(phase->get_state() != UVM_PHASE_DORMANT && phase->get_state() != UVM_PHASE_SCHEDULED)
    {
      uvm_phase_state state = phase->get_state();
      failed = true;
      std::ostringstream str;
      str << "Expected phase " << phase->get_name() << " to start out dormant, but it started out " << uvm_phase_state_name[state];
      UVM_ERROR("DORMANT", str.str());
    }

    // Wait for phase to be started
    phase->wait_for_state(UVM_PHASE_EXECUTING, UVM_EQ);
    start_cnt++;
    if(sc_time_stamp() != sc_time(start_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected start time of " << start_time << "ms for phase " << phase->get_name() << ", got " << sc_time_stamp();
      UVM_ERROR("START", str.str());
    }

    phase->wait_for_state(UVM_PHASE_DONE, UVM_EQ);
    end_cnt++;
    if(sc_time_stamp() != sc_time(start_time+phase_transition_time, SC_MS))
    {
      failed = true;
      std::ostringstream str;
      str << "Expected end time of " << start_time+phase_transition_time
          << "ms for phase " << phase->get_name() << ", got " << sc_time_stamp();
      UVM_ERROR("END", str.str());
    }
  }

  // By the time this runs, the run phase is done, but it should still be
  // EXECUTING due to the runtime phases
  void main_phase(uvm_phase& phase) override
  {
    uvm_domain* domain = uvm_domain::get_common_domain();
    uvm_phase* run_phase = domain->find(uvm_run_phase::get());
    uvm_phase_state state = run_phase->get_state();

    if(state != UVM_PHASE_EXECUTING)
    {
      UVM_INFO("MAIN", "Extract schedule name = " + run_phase->get_schedule_name(), UVM_NONE);
      failed = true;
      std::ostringstream str;
      str << "Expected run phase be UVM_PHASE_EXECUTING, but it is " << uvm_phase_state_name[state];
      UVM_ERROR("READY_TO_END", str.str());
    }
  }

  void report_phase(uvm_phase& phase) override
  {
    phase_run[uvm_report_phase::get()] = true;

    if(start_cnt != 3)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 3 phases to start but got " << start_cnt;
      UVM_ERROR("NUMSTART", str.str());
    }

    if(end_cnt != 3)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 3 phases to finish but got " << end_cnt;
      UVM_ERROR("NUMSTART", str.str());
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
