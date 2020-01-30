//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2011 Synopsys, Inc.
//   All Rights Reserved
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

//----------------------------------------------------------------------
// This example demonstrates the timeout mechanism caused by the objection
// concept in UVM.
//----------------------------------------------------------------------

#include <systemc>
#include <uvm>

#include "tb_timer.h"
#include "tb_env.h"

using namespace uvm;
using namespace sc_core;

class test : public uvm_test
{
 public:
  test(uvm_component_name name) : uvm_test(name) {}

  UVM_COMPONENT_UTILS(test);

  void pre_main_phase(uvm_phase& phase)
  {
    phase.raise_objection(this);
    wait(100, SC_US);
    phase.drop_objection(this);
  }
   
  void main_phase(uvm_phase& phase)
  {
    phase.raise_objection(this);
    // Will cause a time-out
    // because we forgot to drop the objection
    // wait(2999, SC_US);
    // phase.drop_objection(this);
  }
   
  void shutdown_phase(uvm_phase& phase)
  {
    phase.raise_objection(this);
    wait(100, SC_US);
    phase.drop_objection(this);
  }
  
  virtual ~test()
  {
    tb_timer::destroy();
  }
};


int sc_main(int, char*[])
{
  tb_env* env = new tb_env("env");

  //uvm_set_verbosity_level(UVM_DEBUG);

  run_test("test");

  delete env;

  return 0;
}
