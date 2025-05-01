//---------------------------------------------------------------------- 
//   Copyright 2010-2018 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Mentor Graphics Corporation
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

// This test creates a simple hierarchy where two leaf cells belong
// to two different domains. It verifies that the domains run
// independently in the runtime phases but together in the common
// phases.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

bool failed;
std::map<uvm_phase*, bool> phase_map_run;

#define PHASE(NAME,DELAY,STARTTIME) \
  void NAME##_phase(uvm_phase& phase) override { \
    std::string str; \
    str = "start " + std::string(#NAME); \
    phase.raise_objection(this,str); \
    phase_map_run[uvm_##NAME##_phase::get()] = true; \
    str = "Starting " + std::string(#NAME); \
    UVM_INFO(#NAME, str, UVM_NONE); \
    if(sc_time_stamp() != STARTTIME) { \
      failed = true; \
      std::ostringstream sstr; \
      sstr << "Expected start time of " << STARTTIME; \
      UVM_ERROR(#NAME, sstr.str()); \
    } \
    wait(DELAY); \
    str = "Ending " + std::string(#NAME); \
    UVM_INFO(#NAME, str, UVM_NONE); \
    str = "end " + std::string(#NAME); \
    phase.drop_objection(this,str); \
  }


class base : public uvm_component
{
 public:
  sc_time delay;
  sc_time maxdelay;

  base(uvm_component_name name)
  : uvm_component(name)
  {
    delay = sc_time(300, SC_SEC);
    maxdelay = 5*delay;
  }

  PHASE(reset, delay, SC_ZERO_TIME);
  PHASE(main, delay, delay);
  PHASE(shutdown, delay, 2*delay);
  PHASE(run, maxdelay, SC_ZERO_TIME);

  void extract_phase(uvm_phase& phase) override
  {
    phase_map_run[uvm_extract_phase::get()] = true;
    UVM_INFO("EXTRACT", "Starting Extract", UVM_NONE);

    if( sc_time_stamp() != maxdelay )
    {
      failed = true;
      std::ostringstream str;
      str << "Expected extract start time of " << maxdelay;
      UVM_ERROR("extract", str.str());
    }
    UVM_INFO("EXTRACT", "Ending Extract", UVM_NONE);
  }
}; // class base


class leaf : public base
{
 public:
  leaf(uvm_component_name name)
  : base(name)
  {}
}; // class leaf


class env : public base
{
 public:
  leaf l1, l2;

  UVM_COMPONENT_UTILS(env);

  static uvm_domain& get_domain1()
  {
    static uvm_domain uvm_domain1("domain1");
    return uvm_domain1;
  }

  static uvm_domain& get_domain2()
  {
    static uvm_domain uvm_domain2("domain2");
    return uvm_domain2;
  }

  env( uvm_component_name name)
  : base(name), l1("l1"), l2("l2")
  {
    l1.maxdelay = sc_time(1500, SC_SEC);
  }

  void connect_phase(uvm_phase& phase) override
  {
    l1.set_domain(get_domain1());
    l2.set_domain(get_domain2());
  }
}; // class env


class test : public base
{
 public:
  env env1, env2;
  uvm_domain domain1, domain2;
  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name)
  : base(name), env1("env1"), env2("env2"),
    domain1("env_domain1"), domain2("env_domain2")
  {
    env1.l1.delay = sc_time(205, SC_SEC);
    env1.l2.delay = sc_time(300, SC_SEC);
    env2.l1.delay = sc_time(108, SC_SEC);
    env2.l2.delay = sc_time(151, SC_SEC);
  }

  void connect_phase(uvm_phase& phase) override
  {
    //env1 and env2 comps are in different domains
    env2.l1.set_domain(domain1);
    env2.l2.set_domain(domain2);
  }

  void report_phase(uvm_phase& phase) override
  {
    phase_map_run[uvm_report_phase::get()] = true;
    if(phase_map_run.size() != 6)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 6 phases, got " << phase_map_run.size();
      UVM_ERROR("NUMPHASES", str.str());
    }

    if (failed)
      std::cout << "*** UVM TEST FAILED ***" << std::endl;
    else
      std::cout << "*** UVM TEST PASSED ***" << std::endl;
  }
}; //class test


int sc_main(int, char*[])
{
  // set globals
  failed = false;
  phase_map_run.clear();

  test test1("test1");
  run_test();
  return 0;
}
