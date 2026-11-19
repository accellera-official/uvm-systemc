//---------------------------------------------------------------------- 
//   Copyright 2010-2011 Synopsys, Inc. 
//   Copyright 2010 Mentor Graphics Corporation
//   Copyright 2016 NXP B.V.
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

// test that UVm simulation finishes, even if the SystemC clock keeps
// running

#include <systemc>
#include <uvm>

using namespace uvm;
using namespace sc_core;

class clk : public sc_module
{
 public:

  SC_HAS_PROCESS(clk);
  
  clk( sc_module_name name ) : sc_module(name)
  {
    s = false;
    SC_THREAD(thread);	
  }

  virtual void thread()
  {
    while(true) // run forever
    {
      s = !s;
      wait(1.0, SC_MS);
    }
  }
  
  bool s;
};


class test : public uvm_test
{
 public:
  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name ) : uvm_test(name)
  {}

   void report_phase( uvm_phase& phase ) override
  {
    std::cout << "** UVM TEST PASSED **" << std::endl;
  }
};


int sc_main(int, char*[]) 
{ 
  clk clk1("clk1"); // dummy clock with runs forever

  test test1("test1"); // instantiate test

  run_test(); // test should return, even if clock keeps running

  return 0;
}
