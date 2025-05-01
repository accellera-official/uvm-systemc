//---------------------------------------------------------------------- 
//   Copyright 2011-2018 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2015-2021 NXP B.V.
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

// This test creates a simple hierarchy where three leaf cells belong
// to three different domains. The environment puts the three
// domains into lockstep to make sure they are phased together.
// The jump_leaf is a leaf that should jump independently  from
// the other components but the other components should hold up
// the main phase until the jumped component reaches them.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

// global counter of phases
std::map<uvm_phase*, bool> phase_run;

// global check result
bool failed;

// global shutdown time
sc_time shutdown_time; // this is set by the jump leaf

// global reset time
sc_time reset_time;

class base : public uvm_component
{
 public:
  sc_time localdelay;
  sc_time domaindelay;

  base( uvm_component_name name) : uvm_component(name)
  {
    localdelay = sc_time(100, SC_SEC);
    domaindelay = sc_time(300, SC_SEC);
  }

  void build_phase(uvm_phase& phase) override
  {
    phase_run[uvm_build_phase::get()] = true;
    UVM_INFO("BUILD", "Starting Build", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("BUILD", "Expected Build start time of 0 s");
    }
    UVM_INFO("BUILD", "Ending Build", UVM_NONE);
  }

  void reset_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "reset phase");
    phase_run[uvm_reset_phase::get()] = true;
    UVM_INFO("RESET", "Starting Reset", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RESET", "Expected Reset start time of 0 s");
    }
    wait(localdelay);
    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this, "reset phase");
  }

  void main_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "main phase");
    phase_run[uvm_main_phase::get()] = true;
    UVM_INFO("MAIN", "Starting Main", UVM_NONE);
    if (sc_time_stamp() != domaindelay)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Main start time of " << domaindelay << " s";
      UVM_ERROR("MAIN", str.str());
    }
    wait(localdelay);
    UVM_INFO("MAIN", "Ending Main", UVM_NONE);
    phase.drop_objection(this, "main phase");
  }

  void shutdown_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "shutdown phase");
    phase_run[uvm_shutdown_phase::get()] = true;
    UVM_INFO("SHUTDOWN", "Starting Shutdown", UVM_NONE);
    if (sc_time_stamp() != shutdown_time)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Shutdown start time of " << 2*domaindelay << " s";
      UVM_ERROR("SHUTDOWN", str.str());
    }
    wait(localdelay);
    UVM_INFO("SHUTDOWN", "Ending Shutdown", UVM_NONE);
    phase.drop_objection(this, "shutdown phase");
  }

  void run_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "run phase");
    phase_run[uvm_run_phase::get()] = true;
    UVM_INFO("RUN", "Starting Run", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RUN", "Expected Run start time of 0 s");
    }
    wait(5*localdelay);
    UVM_INFO("RUN", "Ending Run", UVM_NONE);
    phase.drop_objection(this, "run phase");
  }

   void extract_phase(uvm_phase& phase) override
  {
    phase_run[uvm_extract_phase::get()] = true;
    UVM_INFO("EXTRACT", "Starting Extract", UVM_NONE);
    if (sc_time_stamp() != 5*domaindelay)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected extract start time of " << 5*domaindelay << " s";
      UVM_ERROR("EXTRACT", str.str());
    }
      UVM_INFO("EXTRACT", "Ending Extract", UVM_NONE);
  }

}; // class base


class jump_leaf : public uvm_component
{
 public:
  bool was_reset;
  sc_time localdelay;
  sc_time domaindelay;

  jump_leaf( uvm_component_name name )
  : uvm_component(name)
  {
    was_reset = false;
    localdelay = sc_time(100, SC_SEC);
    domaindelay = sc_time(300, SC_SEC);
  }

  void build_phase(uvm_phase& phase) override
  {
    phase_run[uvm_build_phase::get()] = true;
    UVM_INFO("BUILD", "Starting Build", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("BUILD", "Expected Build start time of 0 s");
    }
    UVM_INFO("BUILD", "Ending Build", UVM_NONE);
  }

  void reset_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "reset phase");
    was_reset = phase.get_run_count() == 1 ? false : true;

    std::cout << "RUN COUNT: " << phase.get_run_count() << std::endl;

    phase_run[uvm_reset_phase::get()] = true;

    UVM_INFO("RESET", "Starting Reset", UVM_NONE);
    if (sc_time_stamp() != reset_time)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Reset start time of " << reset_time << " s";
      UVM_ERROR("RESET", str.str());
    }
    wait(localdelay);
    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this, "reset phase");
  }

  void main_phase(uvm_phase& phase) override
  {
    sc_time expdelay;
    was_reset = phase.get_run_count() == 1 ? false : true;
    phase.raise_objection(this, "main phase");
    phase_run[uvm_main_phase::get()] = true;
    UVM_INFO("MAIN", "Starting Main", UVM_NONE);
    if (was_reset)
      expdelay = reset_time + localdelay;
    else
      expdelay = domaindelay;

    if (sc_time_stamp() != expdelay)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Main start time of " << expdelay << " s";
      UVM_ERROR("MAIN", str.str());
    }
    wait(localdelay);
    shutdown_time = sc_time_stamp();

    UVM_INFO("MAIN", "Ending Main", UVM_NONE);
    phase.drop_objection(this, "main phase");
  }

  void shutdown_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this, "shutdown phase");
    phase_run[uvm_shutdown_phase::get()] = true;
    UVM_INFO("SHUTDOWN", "Starting Shutdown", UVM_NONE);
    if (sc_time_stamp() != shutdown_time)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Shutdown start time of " << 2*domaindelay << " s";
      UVM_ERROR("SHUTDOWN", str.str());
    }
    wait(localdelay);
    UVM_INFO("SHUTDOWN", "Ending Shutdown", UVM_NONE);
    phase.drop_objection(this, "shutdown phase");
  }
}; // class jump_leaf

class leaf : public base
{
 public:
  leaf(uvm_component_name name) : base(name)
  {}
}; // class leaf


class test : public base
{
 public:
  leaf l1, l2;
  jump_leaf l3;

  uvm_domain domain1, domain2, domain3;

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name)
  : base(name),
    l1("l1"), l2("l2"), l3("l3"),
    domain1("domain1"), domain2("domain2"), domain3("domain3")
  {
    l3.localdelay = sc_time(250, SC_SEC);
    l1.localdelay = l1.domaindelay;

    set_domain(domain1);    // for this and l1
    l2.set_domain(domain2);
    l3.set_domain(domain3);

    //Lockstep the domains
    l2.domaindelay = l1.domaindelay;
    l3.domaindelay = l1.domaindelay;
    domain1.sync(domain2);
    domain1.sync(domain3);
  }

  void run_phase(uvm_phase& phase) override
  {
    uvm_phase* reset_phase = domain3.find_by_name("reset");
    uvm_phase* main_phase = domain3.find_by_name("main");
    phase.raise_objection(this, "run phase");
    wait(350.0, SC_SEC);
    for (int i = 0; i<3; i++) // repeat 3x
    {
      // We will be in main or reset when we jump
      std::cout << "!!!!!! JUMP TO RESET !!!!!" << std::endl;
      //domain3.jump_all(uvm_reset_ph);
      reset_time = sc_time_stamp();

      if(reset_phase->get_state() == UVM_PHASE_EXECUTING)
        reset_phase->jump(uvm_reset_phase::get());
      else
        main_phase->jump(uvm_reset_phase::get());

      wait(150.0, SC_SEC);
    }
    phase.drop_objection(this, "run phase");
  }

  void report_phase(uvm_phase& phase) override
  {
    uvm_phase* reset_ph;

    phase_run[uvm_report_phase::get()] = true;

    //Check that each domain executed the reset phase the correct
    //number of times.

    reset_ph = domain1.find_by_name("reset");
    if(reset_ph->get_run_count() != 1)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected domain 1 to run reset 1 time, got " << reset_ph->get_run_count();
      UVM_ERROR("domain1", str.str());
    }

    reset_ph = domain2.find_by_name("reset");
    if(reset_ph->get_run_count() != 1)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected domain 2 to run reset 1 time, got " << reset_ph->get_run_count();
      UVM_ERROR("domain1", str.str());
    }

    reset_ph = domain3.find_by_name("reset");
    if(reset_ph->get_run_count() != 4)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected domain 3 to run reset 3 time, got " << reset_ph->get_run_count();
      UVM_ERROR("domain1", str.str());
    }

    if(phase_run.size() != 7)
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
  // set globals
  phase_run.clear();
  failed = false;
  shutdown_time = SC_ZERO_TIME;
  reset_time = SC_ZERO_TIME;

  test test1("test1");
  run_test();
  return 0;
}
