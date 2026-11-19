//---------------------------------------------------------------------- 
//   Copyright 2010-2018 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2017 NVIDIA Corporation
//   Copyright 2010 Synopsys, Inc.
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
    phase.raise_objection(this);
    phase_run[uvm_reset_phase::get()] = true;
    UVM_INFO("RESET", "Starting Reset", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RESET", "Expected Reset start time of 0 s");
    }
    wait(localdelay);
    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this);
  }

  void main_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
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
    phase.drop_objection(this);
  }

  void shutdown_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_shutdown_phase::get()] = true;
    UVM_INFO("SHUTDOWN", "Starting Shutdown", UVM_NONE);
    if (sc_time_stamp() != 2*domaindelay)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected Shutdown start time of " << 2*domaindelay << " s";
      UVM_ERROR("SHUTDOWN", str.str());
    }
    wait(localdelay);
    UVM_INFO("SHUTDOWN", "Ending Shutdown", UVM_NONE);
    phase.drop_objection(this);
  }

  void run_phase(uvm_phase& phase) override
  {
    phase.raise_objection(this);
    phase_run[uvm_run_phase::get()] = true;
    UVM_INFO("RUN", "Starting Run", UVM_NONE);
    if (sc_time_stamp() != SC_ZERO_TIME)
    {
      failed = true;
      UVM_ERROR("RUN", "Expected Run start time of 0 s");
    }
    wait(5*localdelay);
    UVM_INFO("RUN", "Ending Run", UVM_NONE);
    phase.drop_objection(this);
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


class leaf : public base
{
 public:
  leaf(uvm_component_name name) : base(name)
  {}
}; // class leaf


class test : public base
{
 public:
  leaf l1, l2, l3;
  uvm_domain domain1, domain2, domain3;

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name)
  : base(name),
    l1("l1"), l2("l2"), l3("l3"),
    domain1("domain1"), domain2("domain2"), domain3("domain3")
  {
    l3.localdelay = sc_time(250, SC_SEC);
    l1.localdelay = l1.domaindelay;

    set_domain(domain1);
    //l1.set_domain(domain1); // NOTE also commented out in UVM-SV test
    l2.set_domain(domain2);
    l3.set_domain(domain3);

    // Lockstep the domains
    l2.domaindelay = l1.domaindelay;
    l3.domaindelay = l1.domaindelay;
    domain1.sync(domain2);
    domain1.sync(domain3);
  }

  //NOTE also commented out in UVM-SV test
  //void run_phase(uvm_phase& phase)
  //{
  //  phase.raise_objection(this);
  //  phase.drop_objection(this);
  //}

  void report_phase(uvm_phase& phase) override
  {
    phase_run[uvm_report_phase::get()] = true;
    if (phase_run.size() != 7)
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

  test test1("test1");
  run_test();
  return 0;
}
