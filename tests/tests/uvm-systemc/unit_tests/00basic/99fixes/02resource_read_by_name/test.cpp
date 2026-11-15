//----------------------------------------------------------------------
//   Copyright 2026 Mayur Kubavat
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

// Regression test for uvm_resource_pool::spell_check (issue #295).
// A by-name lookup with error reporting enabled must find a resource
// that exists, and must report close matches for a misspelt name.

#include <systemc>
#include <uvm>

#include <iostream>

using namespace uvm;

int sc_main(int, char*[])
{
  uvm_coreservice_t* cs = uvm_coreservice_t::get();
  uvm_report_server* svr = cs->get_report_server();

  uvm_resource_db<int>::set("top.env", "num_packets", 42);
  uvm_resource_db<int>::set("top.env", "num_masters", 2);

  // Existing name: read_by_name() reports errors, so it uses spell_check()
  int val = 0;
  if (!uvm_resource_db<int>::read_by_name("top.env", "num_packets", val))
  {
    UVM_ERROR("Test", "read_by_name() did not find existing resource 'num_packets'");
  }
  else if (val != 42)
  {
    UVM_ERROR("Test", "read_by_name() returned a wrong value for 'num_packets'");
  }
  else
    std::cout << "num_packets = " << val << std::endl;

  // Misspelt name: the lookup fails and the closest name is suggested
  int other = 0;
  if (uvm_resource_db<int>::read_by_name("top.env", "num_packet", other))
    UVM_ERROR("Test", "read_by_name() found misspelt resource 'num_packet'");

  // Without error reporting, spell_check() is not used
  if (uvm_resource_db<int>::get_by_name("top.env", "num_masters", false) == nullptr)
    UVM_ERROR("Test", "get_by_name() without rpterr did not find 'num_masters'");

  // Two names at the same distance are both suggested
  uvm_resource_db<int>::set("top.env", "mode_a", 1);
  uvm_resource_db<int>::set("top.env", "mode_b", 2);
  if (uvm_resource_db<int>::get_by_name("top.env", "mode_c") != nullptr)
    UVM_ERROR("Test", "get_by_name() found misspelt resource 'mode_c'");

  svr->report_summarize();

  if (svr->get_severity_count(UVM_FATAL) +
      svr->get_severity_count(UVM_ERROR) == 0)
    std::cout << "** UVM TEST PASSED **" << std::endl;
  else
    std::cout << "!! UVM TEST FAILED !!" << std::endl;

  return 0;
}
