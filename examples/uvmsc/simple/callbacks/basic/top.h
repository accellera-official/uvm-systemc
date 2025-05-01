//----------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
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

#ifndef TOP_H_
#define TOP_H_

#include <systemc>
#include <uvm>

#include "bus_tr.h"
#include "bus_driver.h"
#include "my_bus_driver_cb.h"
#include "my_bus_driver_cb2.h"

//------------------------------------------------------------------------------
// top:
//
// In this simple example, we don't build a complete environment, but this does
// not detract from the example's purpose. In the top env, we instantiate
// the bus_driver, and one instance each of our custom callback classes.
// To register the callbacks with the driver, we get the global callback pool
// that is typed to our specific driver-callback combination. We associate
// (register) the callback objects with driver using the callback pool's
// add_cb method. After calling display just to show that the
// registration was successful, we push several transactions into the driver.
// Our custom callbacks get called as the driver receives each transaction.
//------------------------------------------------------------------------------

// convenience typedef for the driver callbacks
typedef uvm::uvm_callbacks<bus_driver,bus_driver_cb> bus_driver_cbs_t;

class top : public uvm::uvm_env
{
 public:

  bus_driver* driver;     // driver component
  my_bus_driver_cb* cb1;  // callback object
  my_bus_driver_cb2* cb2; // another callback object

  top(uvm::uvm_component_name nm) : uvm::uvm_env(nm) {}

  UVM_COMPONENT_UTILS(top);

  void build_phase(uvm::uvm_phase& phase) override
  {
    uvm::uvm_env::build_phase(phase);

    // Normally we should use the UVM factory to create new instances.
    // In this case we use a regular 'new', so we do not register these
    // objects to the factory for later override

    driver = new bus_driver("driver");
    cb1 = new my_bus_driver_cb("cb1");
    cb2 = new my_bus_driver_cb2("cb2");

    // add the additional callbacks

    bus_driver_cbs_t::add(driver, cb1);
    bus_driver_cbs_t::add(driver, cb2);
    bus_driver_cbs_t::display();
  }

  void run_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);

    // generate some arbitrary traffic for the driver
    bus_tr tr;

    for (int i = 1; i <= 5; i++)
    {
      tr.addr = i;
      tr.data = 6-i;
      driver->put(tr);
    }

    phase.drop_objection(this);
  }

  ~top() override
  {
    delete driver;
    delete cb1;
    delete cb2;
  }
};

#endif /* TOP_H_ */
