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

// This test creates a simple hierarchy and makes sure that the
// phase_started and phase_ended callbacks are all called once per
// component and in the correct order.
//
// The order is:
//   build
//   connect
//   end_of_elaboration
//   start_of_simulation
//   run
//     pre_reset
//     reset
//     post_reset
//     pre_configure
//     configure
//     post_configure
//     pre_main
//     main
//     post_main
//     pre_shutdown
//     shutdown
//     post_shutdown
//  extract
//  check
//  report
//  final

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;
using namespace sc_core;

// global variables
bool failed = false;
int counter = 0;

class base : public uvm_component
{
 public:
  std::map<std::string, int> phase_count;
  std::string last_phase;

  base(uvm_component_name name)
  : uvm_component(name)
  {
    last_phase.clear(); // empty string
  }

  // for parallel phase have two possible last phases
  void check_phase_callback(uvm_phase& phase, std::string last, std::string last_alt)
  {
    std::string ph_name = phase.get_name();
    int ph_cnt = phase.get_run_count();

    UVM_INFO("CHECKER", "Checking a phase...", UVM_NONE);

    if(last != last_phase && last_alt != last_phase)
    {
      UVM_ERROR("LAST_PH", "Incorrect last phase, expected " + last + " got " + last_phase);
      failed = true;
    }
    if(phase_count[ph_name] != ph_cnt)
    {
      std::ostringstream str;
      str << "Expected phase count " << phase_count[ph_name] << " for " << ph_name << " but got " << ph_cnt;
      UVM_ERROR("EXECUTED", str.str());
      failed = true;
    }
    counter++;
    //std::cout << "count: " << counter << std::endl;
    last_phase = ph_name;
  }

  void phase_started(uvm_phase& phase) override
  {
    std::string last, last_alt;
    std::string ph_name = phase.get_name();

    UVM_INFO("STARTED", "Phase started for phase " + ph_name, UVM_NONE);

    if(phase_count.find(ph_name) != phase_count.end()) // if exists
      phase_count[ph_name]++;
    else
      phase_count[ph_name] = 1;

    if (ph_name == "build") last = "";
    if (ph_name == "connect") last = "build";
    if (ph_name == "end_of_elaboration") last = "connect";
    if (ph_name == "start_of_simulation") last = "end_of_elaboration";
    if (ph_name == "run")
    {
      last = "start_of_simulation";
      //last_alt = "pre_reset";
    }
    if (ph_name == "pre_reset")
    {
      last = "start_of_simulation";
      last_alt = "run"; // account for race: if 'run' starts before 'pre_reset', it will be the last phase
    }
    if (ph_name == "reset")
    {
      last = "pre_reset";
      last_alt = "main"; // from jump_back
    }
    if (ph_name == "post_reset") last = "reset";
    if (ph_name == "pre_configure") last = "post_reset";
    if (ph_name == "configure") last = "pre_configure";
    if (ph_name == "post_configure") last = "configure";
    if (ph_name == "pre_main") last = "post_configure";
    if (ph_name == "main") last = "pre_main";
    if (ph_name == "post_main") last = "main";
    if (ph_name == "pre_shutdown") last = "post_main";
    if (ph_name == "shutdown") last = "pre_shutdown";
    if (ph_name == "post_shutdown") last = "shutdown";
    if (ph_name == "extract")
    {
      last = "run";
      last_alt = "post_shutdown";
    }
    if (ph_name == "check") last = "extract";
    if (ph_name == "report") last = "check";
    if (ph_name == "final") last = "report";

    if (last_alt.empty()) last_alt = last;
    check_phase_callback(phase, last, last_alt);
  }

  void phase_ended(uvm_phase& phase) override
  {
    std::string last, last_alt;
    std::string ph_name = phase.get_name();

    UVM_INFO("ENDED", "Phase ended for phase " + ph_name, UVM_NONE);

    if (ph_name == "build") last = "build";
    if (ph_name == "connect") last = "connect";
    if (ph_name == "end_of_elaboration") last = "end_of_elaboration";
    if (ph_name == "start_of_simulation") last = "start_of_simulation";
    if (ph_name == "run")
    {
      last = "run";
      last_alt = "post_shutdown";
    }
    if (ph_name == "pre_reset") last = "pre_reset";
    if (ph_name == "reset") last = "reset";
    if (ph_name == "post_reset") last = "post_reset";
    if (ph_name == "pre_configure") last = "pre_configure";
    if (ph_name == "configure") last = "configure";
    if (ph_name == "post_configure") last = "post_configure";
    if (ph_name == "pre_main") last = "pre_main";
    if (ph_name == "main") last = "main";
    if (ph_name == "post_main") last = "post_main";
    if (ph_name == "pre_shutdown") last = "pre_shutdown";
    if (ph_name == "shutdown") last = "shutdown";
    if (ph_name == "post_shutdown")
    {
      last = "run";
      last_alt = "post_shutdown";
    }
    if (ph_name == "extract") last = "extract";
    if (ph_name == "check") last = "check";
    if (ph_name == "report") last = "report";
    if (ph_name == "final") last = "final";

    if (last_alt.empty()) last_alt = last;

    check_phase_callback(phase, last, last_alt);
  }
};

struct leaf : public base
{
  leaf(uvm_component_name name)
  : base(name) {}
};

class test : public base
{
 public:
  leaf l1, l2;

  UVM_COMPONENT_UTILS(test);

  test(uvm_component_name name)
  : base(name),
    l1("l1"),
    l2("l2") {}

  void main_phase(uvm_phase& phase) override
  {
    static bool first = true;
    phase.raise_objection(this);
    wait(10, SC_MS);

    if(first)
    {
      first = false;
      phase.jump(uvm_reset_phase::get());
    }
    phase.drop_objection(this);
  }

  void final_phase(uvm_phase& phase) override
  {
    // 21 calls per component for three components (63) for phase started
    // 20 calls for three components (60) for phase ended (since final is still going)
    //
    // Additional calls for jump back from main to reset means that reset, post_reset,
    //    pre_configure, configure, post_configure, pre_main and main all run again.
    //    So, add 7 more calls for each component (21) for both phase started and
    //    phase_ended: 84 for STARTED, 81 for ENDED
    // But, the ENDED callbacks for *this* final phase won't have been counted yet.
    // So we expect 84 for STARTED (fully counted, because it is a bottom-up phase),
    // and 81 for ENDED => 165

    if(counter != 165)
    {
      failed = true;
      std::ostringstream str;
      str << "Expected 165 phases, got " << counter;
      UVM_ERROR("NUMPHASES", str.str());
    }
    if(failed)
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
