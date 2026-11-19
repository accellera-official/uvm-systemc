//---------------------------------------------------------------------- 
//   Copyright 2010 Synopsys, Inc. 
//   Copyright 2010-2011 Mentor Graphics Corporation
//   Copyright 2011 Cadence Design Systems, Inc.
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

#include <systemc>
#include <uvm>
#include <string>
#include <cstdarg>

using namespace uvm;

class test : public uvm_test
{
 public:
  int n_ph;

  UVM_COMPONENT_UTILS(test);

  std::string last_phase;

  void check_the_phase(std::string prev, std::string curr)
  {
    UVM_INFO("Test", "Executing phase " + curr + "...", UVM_LOW);

    if (prev != last_phase)
      UVM_ERROR("Test", "Phase before " + curr + " was " + last_phase + " instead of " + prev + ".");

    last_phase = curr;
    n_ph++;
  }

  void check_the_runtime_phase(std::string prev, std::string curr, uvm::uvm_phase& phase)
  {
    phase.raise_objection(this);
    UVM_INFO("Test", "Starting phase " + curr + "...", UVM_LOW);

    sc_core::wait(10, sc_core::SC_US); // #10;

    if (prev != last_phase)
        UVM_ERROR("Test", "Previous phase was " + last_phase + " instead of " + prev + ".");

    sc_core::wait(10, sc_core::SC_US); // #10;

    last_phase = curr;
    UVM_INFO("Test", "Ending phase " + curr + "...", UVM_LOW);
    n_ph++;
    phase.drop_objection(this);
  }

  test( uvm::uvm_component_name name = "my_comp")
  : uvm_test(name), n_ph(0)
  {}

  void build_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("", "build");
  }

  void connect_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("build", "connect");
  }

  void end_of_elaboration_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("connect", "end_of_elaboration");
  }

  void start_of_simulation_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("end_of_elaboration", "start_of_simulation");
  }

  void run_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("start_of_simulation", "run", phase);
  }

  void pre_reset_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    check_the_runtime_phase("start_of_simulation", "pre_reset", phase);

    // Make sure the last phase is not "run"
    sc_core::wait(10, sc_core::SC_US); // #10;

    last_phase = "pre_reset";
    phase.drop_objection(this);
  }

  void reset_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("pre_reset", "reset", phase);
  }

  void post_reset_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("reset", "post_reset", phase);
  }

  void pre_configure_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("post_reset", "pre_configure", phase);
  }

  void configure_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("pre_configure", "configure", phase);
  }

  void post_configure_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("configure", "post_configure", phase);
  }

  void pre_main_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("post_configure", "pre_main", phase);
  }

  void main_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("pre_main", "main", phase);
  }

  void post_main_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("main", "post_main", phase);
  }

  void pre_shutdown_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("post_main", "pre_shutdown", phase);
  }

  void shutdown_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("pre_shutdown", "shutdown", phase);
  }

  void post_shutdown_phase(uvm::uvm_phase& phase) override
  {
    check_the_runtime_phase("shutdown", "post_shutdown", phase);
  }

  void extract_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("post_shutdown", "extract");
  }

  void check_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("extract", "check");
  }

  void report_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("check", "report");
  }

  void final_phase(uvm::uvm_phase& phase) override
  {
    check_the_phase("report", "final");
  }
}; // class test


int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  UVM_INFO("Test", "Phasing one component through default phases...", UVM_NONE);

  test test1("test1");

  run_test();

  if (test1.n_ph != 21)
  {
    std::ostringstream msg;
    msg << "A total of " << test1.n_ph << "phase methods were executed instead of 21.";
    UVM_ERROR("Test", msg.str());
  }

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout << "!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
