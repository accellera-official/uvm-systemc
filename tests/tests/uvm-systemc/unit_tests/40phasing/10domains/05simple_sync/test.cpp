//---------------------------------------------------------------------- 
//   Copyright 2011-2018 Cadence Design Systems, Inc.
//   Copyright 2011 Mentor Graphics Corporation
//   Copyright 2012 Cisco Systems, Inc.
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
// to two different domains, and their independent time progressing is
// checked. Domain synchronization is done using sync.

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

// global check result
bool failed;

class comp_type : public uvm_component
{
 public:
  sc_time start_reset, start_main, start_shutdown;
  sc_time end_reset, end_main, end_shutdown;
  sc_time delay;

  comp_type( uvm_component_name name )
  : uvm_component(name)
  {
    start_reset = SC_ZERO_TIME;
    start_main = SC_ZERO_TIME;
    start_shutdown = SC_ZERO_TIME;
    end_reset = SC_ZERO_TIME;
    end_main = SC_ZERO_TIME;
    end_shutdown = SC_ZERO_TIME;
    delay = sc_time(200, SC_SEC);
  }

  void reset_phase(uvm_phase& phase) override
  {
    UVM_INFO("RESET", "Starting Reset", UVM_NONE);
    phase.raise_objection(this, "start reset");
    wait(delay);
    UVM_INFO("RESET", "Ending Reset", UVM_NONE);
    phase.drop_objection(this, "start reset");
  }

  void main_phase(uvm_phase& phase) override
  {
    UVM_INFO("MAIN", "Starting Main", UVM_NONE);
    phase.raise_objection(this, "start main");
    wait(delay);
    UVM_INFO("MAIN", "Ending Main", UVM_NONE);
    phase.drop_objection(this, "start main");
  }

  void shutdown_phase(uvm_phase& phase) override
  {
    UVM_INFO("SHUTDOWN", "Starting Shutdown", UVM_NONE);
    phase.raise_objection(this, "start shutdown");
    wait(delay);
    UVM_INFO("SHUTDOWN", "Ending Shutdown", UVM_NONE);
    phase.drop_objection(this, "start shutdown");
  }

  void phase_started(uvm_phase& phase) override
  {
    if (phase.is(uvm_reset_phase::get())) start_reset = sc_time_stamp();
    if (phase.is(uvm_main_phase::get())) start_main = sc_time_stamp();
    if (phase.is(uvm_shutdown_phase::get())) start_shutdown = sc_time_stamp();
  }

  void phase_ended(uvm_phase& phase) override
  {
    if (phase.is(uvm_reset_phase::get())) end_reset = sc_time_stamp();
    if (phase.is(uvm_main_phase::get())) end_main = sc_time_stamp();
    if (phase.is(uvm_shutdown_phase::get())) end_shutdown = sc_time_stamp();
  }
}; // class comp_type

class test : public uvm_component
{
 public:
  comp_type leaf1, leaf2;
  uvm_domain domain1, domain2;
  bool failed {false};

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name)
  : uvm_component(name), leaf1("leaf1"), leaf2("leaf2"), domain1("domain1"), domain2("domain2")
  {
    leaf2.delay = sc_time(250, SC_SEC);

    leaf1.set_domain(domain1);
    leaf2.set_domain(domain2);

    // Sync domain1 main_phase with domain2 post_reset_phase
    domain1.sync(domain2, uvm_main_phase::get(), uvm_post_reset_phase::get());
  }

  void report_phase(uvm_phase& phase) override
  {
    std::cout << "l1  reset:    " << leaf1.start_reset    << " - " << leaf1.end_reset    << std::endl;
    std::cout << "l1  main:     " << leaf1.start_main     << " - " << leaf1.end_main     << std::endl;
    std::cout << "l1  shutdown: " << leaf1.start_shutdown << " - " << leaf1.end_shutdown << std::endl;
    std::cout << "l2  reset:    " << leaf2.start_reset    << " - " << leaf2.end_reset    << std::endl;
    std::cout << "l2  main:     " << leaf2.start_main     << " - " << leaf2.end_main     << std::endl;
    std::cout << "l2  shutdown: " << leaf2.start_shutdown << " - " << leaf2.end_shutdown << std::endl;


    if( leaf1.start_reset != SC_ZERO_TIME || leaf1.end_reset != sc_time(200, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf1 reset " << leaf1.start_reset
                << " - " << leaf1.end_reset << ", expected 0 - 200 ***" << std::endl;
      failed = true;
    }

    // Since leaf1 main is synced with leaf2 post_reset, it does not start until leaf2 post_reset starts at 250
    if( leaf1.start_main != sc_time(250, SC_SEC) || leaf1.end_main != sc_time(450, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf1 main " << leaf1.start_main
                << " - " << leaf1.end_main << ", expected 250 - 450 ***" << std::endl;
      failed = true;
    }

    if( leaf1.start_shutdown != sc_time(450, SC_SEC) || leaf1.end_shutdown != sc_time(650, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf1 shutdown " << leaf1.start_shutdown
                << " - " << leaf1.end_shutdown << ", expected 450 - 650 ***" << std::endl;
      failed = true;
    }

    if( leaf2.start_reset != SC_ZERO_TIME || leaf2.end_reset != sc_time(250, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf2 reset " << leaf2.start_reset
                << " - " << leaf2.end_reset << ", expected 0 - 250 ***" << std::endl;
      failed = true;
    }

    // Since leaf2 post_reset is synced with leaf1 main, post_reset does not end until 450
    if( leaf2.start_main != sc_time(450, SC_SEC) || leaf2.end_main != sc_time(700, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf2 main " << leaf2.start_main
                << " - " << leaf2.end_main << ", expected 450 - 700 ***" << std::endl;
      failed = true;
    }

    if( leaf2.start_shutdown != sc_time(700, SC_SEC) || leaf2.end_shutdown != sc_time(950, SC_SEC))
    {
      std::cout << "*** UVM TEST FAILED, leaf2 shutdown " << leaf2.start_shutdown
                << " - " << leaf2.end_shutdown << ", expected 700 - 950 ***" << std::endl;
      failed = true;
    }

    if(!failed)
      std::cout << "*** UVM TEST PASSED ***" << std::endl;
  }
}; // class test

int sc_main(int, char*[])
{
  // set global
  failed = false;

  run_test("test");
  return 0;
}
