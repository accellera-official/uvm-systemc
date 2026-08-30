//---------------------------------------------------------------------- 
//   Copyright 2017 NXP B.V.
//   Copyright 2010 Synopsys, Inc. 
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

using namespace uvm;

class my_catcher : public uvm_report_catcher
{
public:

  action_e do_catch() override
  {
    std::cout << "Caught a message..." << std::endl;
    return CAUGHT;
  }
};


class test : public uvm_test
{
 public:
  my_catcher* ctchr;

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name) : uvm_test(name), ctchr(nullptr)
  {}

  void run_phase( uvm_phase& phase ) override
  {
    phase.raise_objection(this);

    std::cout << "UVM TEST - ERROR expected since registering a default catcher with nullptr handle" << std::endl;

    uvm_report_cb::add(nullptr, ctchr); // this should generate an UVM error since the catcher object is not assigned

    std::cout << "UVM TEST EXPECT 1 UVM_ERROR" << std::endl;

    phase.drop_objection(this);
  }
};


int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  run_test("test");

  if( svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 1)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout <<"!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
