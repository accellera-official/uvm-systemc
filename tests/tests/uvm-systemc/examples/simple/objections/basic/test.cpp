//----------------------------------------------------------------------
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
//   Copyright 2012-2014 NXP B.V.
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

//----------------------------------------------------------------------
// This simple example shows how the objection is
// used in combination with the drain time to coordinate end of test.
//
// In this example, a drain time of 10ms is set on the component.
// The component then spawns 4 processes which consume different
// amounts of time. When the last process is executed (@50ms),
// the drain time takes effect. The test is completed at time
// 60ms.
//
// The example shows the usage of the component objection
// callbacks. In this case, the dropped callback is used, but
// the raised and all_dropped work similarly (except the
// all dropped is a time-consuming task).
//----------------------------------------------------------------------

#include <systemc>
#include <uvm>
#include <string>
#include <iostream>

using namespace sc_core;
using namespace uvm;

class simple_test: public uvm_test
{
 public:

  simple_test(uvm_component_name name) :
    uvm_test(name)
  {}

  // Register with the factory.
  UVM_COMPONENT_UTILS(simple_test);

  void run_phase(uvm_phase& phase) override
  {

    // Set a drain time on the objection
    sc_time drain_time = sc_time(12.5, SC_US);

    uvm_report_info("drain",
      "Setting drain time of " + drain_time.to_string(), UVM_NONE);

    phase.get_objection()->set_drain_time(this, drain_time);

    // Run a bunch of processes in parallel
    SC_FORK
      sc_spawn(sc_bind(&simple_test::doit, this, 0, 35.0, &phase)),
      sc_spawn(sc_bind(&simple_test::doit, this, 1, 25.0, &phase)),
      sc_spawn(sc_bind(&simple_test::doit, this, 2, 50.0, &phase)),
      sc_spawn(sc_bind(&simple_test::doit, this, 3, 15.0, &phase))
    SC_JOIN
  }

  void report_phase( uvm_phase& phase ) override
  {
    uvm_report_info(get_name(), "Ended all doit's", UVM_NONE);
  }

  // A simple task that consumes some time.
  void doit(int inst, double delay, uvm_phase* phase)
  {
    // Raise an objection before starting the activity
    phase->raise_objection(this);

    std::ostringstream msg1;
    msg1 << "Starting doit (" << inst << ") with delay " << delay;
    uvm_report_info("doit", msg1.str(), UVM_NONE);

    wait(delay, SC_US);

    std::ostringstream msg2;
    msg2 << "Ending doit (" << inst << + ")";
    uvm_report_info("doit", msg2.str(), UVM_NONE);

    // Drop the objection when done
    phase->drop_objection(this);
  }

  // Use an objection callback do something when objections are raised or
  // dropped (or all dropped). This example prints some information on each
  // drop.
  void dropped( uvm_objection* objection,
      uvm_object* source_obj,
      const std::string& description,
      int count ) override
  {
    std::ostringstream str;
    str << count
        << " objection(s) dropped from "
        << source_obj->get_full_name()
        << ", total count is now "
        << objection->get_objection_total(this)
        << " top: " << objection->get_objection_total(uvm_root::get());

    uvm_report_info("dropped", str.str(), UVM_NONE);
  }
};

int sc_main(int, char*[])
{
  run_test("simple_test");

  return 0;
}
