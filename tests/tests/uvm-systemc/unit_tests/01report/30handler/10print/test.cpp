//------------------------------------------------------------------------------
//   Copyright 2017 NXP B.V.
//   Copyright 2011 Cadence Design Systems, Inc.
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
//------------------------------------------------------------------------------

#include <systemc>
#include <uvm>

using namespace uvm;
using namespace sc_core;


class test : public uvm_test
{
 public:

  UVM_COMPONENT_UTILS(test);

  test( uvm_component_name name ) : uvm_test(name)
  {}

  void build_phase(uvm_phase& phase) override
  {
    set_report_verbosity_level(UVM_FULL);

    set_report_id_verbosity("ID1", UVM_LOW);
    set_report_id_verbosity_hier("ID2", UVM_MEDIUM);
    set_report_id_verbosity("ID3", 301);

    set_report_severity_id_verbosity(UVM_INFO, "ID4", 501);
    set_report_severity_id_verbosity(UVM_WARNING, "ID7", UVM_NONE);
    set_report_severity_id_verbosity(UVM_INFO, "ID5", UVM_FULL);

    set_report_severity_action(UVM_WARNING, UVM_RM_RECORD|UVM_DISPLAY|UVM_COUNT);

    set_report_id_action_hier("ACT_ID", UVM_DISPLAY|UVM_LOG|UVM_COUNT);
    set_report_id_action("ACT_ID2", UVM_DISPLAY);

    set_report_severity_id_action(UVM_INFO, "ID1", UVM_DISPLAY|UVM_COUNT);

    set_report_severity_override(UVM_ERROR, UVM_FATAL);

    set_report_severity_id_override(UVM_INFO, "ID8", UVM_ERROR);
    set_report_severity_id_override(UVM_ERROR, "ID9", UVM_WARNING);

    set_report_severity_id_verbosity_hier(UVM_INFO, "ID6", UVM_LOW);

    set_report_default_file(&std::cout); // was 1

    set_report_id_file("ID3", &std::cout); // was 467 = 0x1D3
    set_report_id_file("ID7", &std::cout); // was 987893 = 0xF12F5

    set_report_severity_file(UVM_INFO, &std::cout); // was 23 = 0x17
    set_report_severity_file(UVM_FATAL, &std::cout); // was 1001 = 0x3E9

    set_report_severity_id_file(UVM_ERROR, "ID0", &std::cout); // was 7 = 0x7
    set_report_severity_id_file(UVM_WARNING, "ID207", &std::cout); // was 300500 = 0x495D4
  }

  void run_phase(uvm_phase& phase) override
  {
    uvm_report_handler* l_rh = get_report_handler();

    std::cout << "START OF GOLD FILE" << std::endl;
    l_rh->print();
    std::cout << "END OF GOLD FILE" << std::endl;
  }
};


int sc_main(int, char*[])
{
  run_test("test");
  return 0;
}
