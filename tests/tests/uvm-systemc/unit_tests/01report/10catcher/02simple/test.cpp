//----------------------------------------------------------------------
//   Copyright 2017 NXP B.V.
//   Copyright 2010 Synopsys, Inc.
//   Copyright 2011 Mentor Graphics Corporation
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

  static int seen;

  action_e do_catch() override
  {
    std::cout << "Caught a message..." << std::endl;
    seen++;
    return CAUGHT;
  }
};

// initialize static variable
int my_catcher::seen = 0;

class test : public uvm_test
{
public:

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name) : uvm_test(name), pass(false)
  {}

  void run_phase(uvm_phase& phase) override
  {
    my_catcher ctchr;

    phase.raise_objection(this);

    std::cout << "UVM TEST EXPECT 2 UVM_ERROR" << std::endl;

    UVM_ERROR("Test", "Error 1...");

    if (my_catcher::seen != 0)
    {
      std::cout << "ERROR: Message was caught with no catcher installed!" << std::endl;
      pass = false;
    }

    // add catcher
    uvm_report_cb::add(nullptr, &ctchr);

    UVM_ERROR("Test", "Error 2...");

    if (my_catcher::seen != 1)
    {
      std::cout << "ERROR: Message was NOT caught with default catcher installed!" << std::endl;
      pass = false;
    }
    else pass = true;

    UVM_INFO("XYZ", "Medium INFO...", UVM_MEDIUM);

    if (my_catcher::seen != 2)
    {
      std::cout << "ERROR: Message was NOT caught with default catcher installed!" << std::endl;
      pass = false;
    }
    else pass = true;

    UVM_FATAL("Test", "FATAL...");

    if (my_catcher::seen != 3)
    {
      std::cout << "ERROR: Message was NOT caught with default catcher installed!" << std::endl;
      pass = false;
    }
    else pass = true;

    // remove catcher
    uvm_report_cb::do_delete(nullptr, &ctchr);

    UVM_ERROR("Test", "Error 3...");

    if (my_catcher::seen != 3)
    {
      std::cout << "ERROR: Message was caught after all catcher removed!" << std::endl;
      pass = false;
    }

    phase.drop_objection(this);
  }

  virtual void report(uvm_phase& phase)
  {
    if (pass) std::cout << "** UVM TEST PASSED **" << std::endl;
    else std::cout << "** UVM TEST FAILED! **" << std::endl;
  }

private:
  bool pass;

};

int sc_main(int, char*[])
{
  run_test("test");
  return 0;
}
