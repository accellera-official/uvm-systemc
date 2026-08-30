//----------------------------------------------------------------------
//   Copyright 2012-2014 NXP B.V.
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

// test reporting info. The default verbosity levels in the UVM standard are
// (note, the values are NOT defined in the UVM standard, only the enums)
//   UVM_NONE   = 0,
//   UVM_LOW    = 100,
//   UVM_MEDIUM = 200,
//   UVM_HIGH   = 300,
//   UVM_FULL   = 400,
//   UVM_DEBUG  = 500

#include <systemc>
#include <uvm>

using namespace uvm;
using namespace sc_core;

class comp_a : public uvm_component
{
 public:
  comp_a(uvm_component_name name) : uvm_component(name) {}

  UVM_COMPONENT_UTILS(comp_a);

  void run_phase(uvm_phase& phase) override
  {
    // all arguments passes to member function
    uvm_report_info(get_full_name(),
      "Report info 1 using verbosity level UVM_NONE", UVM_NONE, __FILE__, __LINE__);

    // without file and line number arguments
    uvm_report_info(get_full_name(),
      "Report info 2 using verbosity level UVM_NONE", UVM_NONE);

    // different verbosity level
    uvm_report_info(get_full_name(),
      "Report info 3 using verbosity level UVM_LOW", UVM_LOW);

    // different verbosity level
    uvm_report_info(get_full_name(),
      "Report info 4 using verbosity level UVM_MEDIUM", UVM_MEDIUM);

    // different verbosity level
    uvm_report_info(get_full_name(),
      "Report info 5 using verbosity level UVM_HIGH", UVM_HIGH);

    // different verbosity level
    uvm_report_info(get_full_name(),
      "Report info 6 using verbosity level UVM_FULL", UVM_FULL);

    // using a value as verbosity level (see above for the table)
    uvm_report_info(get_name(),
      "Report info 7 using verbosity level 150",150);

    // using macro
    UVM_INFO(get_name(),
      "Report info 8 using macro and verbosity level UVM_NONE",UVM_NONE);

    // using macro
    UVM_INFO(get_name(),
      "Report info 9 using macro and verbosity level UVM_LOW",UVM_LOW);

    // using macro
    UVM_INFO(get_name(),
      "Report info 10 using macro and verbosity level UVM_MEDIUM",UVM_MEDIUM);

    // using macro
    UVM_INFO(get_name(),
      "Report info 11 using macro and verbosity level UVM_HIGH",UVM_HIGH);

    // using macro
    UVM_INFO(get_name(),
      "Report info 12 using macro and verbosity level UVM_FULL",UVM_FULL);
  }

};

int sc_main(int, char*[]) 
{  
  comp_a* comp_a1 = new comp_a("comp_a1");
  comp_a* comp_a2 = new comp_a("comp_a2");
  comp_a* comp_a3 = new comp_a("comp_a3");
  comp_a* comp_a4 = new comp_a("comp_a4");
  comp_a* comp_a5 = new comp_a("comp_a5");

  comp_a1->set_report_verbosity_level(UVM_NONE);
  comp_a2->set_report_verbosity_level(UVM_LOW);
  comp_a3->set_report_verbosity_level(UVM_MEDIUM);
  comp_a4->set_report_verbosity_level(UVM_HIGH);
  comp_a5->set_report_verbosity_level(UVM_FULL);

  run_test();

  delete comp_a1;
  delete comp_a2;
  delete comp_a3;
  delete comp_a4;
  delete comp_a5;

  return 0;
}
