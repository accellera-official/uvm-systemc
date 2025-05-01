//------------------------------------------------------------------------------
//   Copyright 2013-2014 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc. 
//   Copyright 2010 Synopsys, Inc.
//   All Rights Reserved Worldwide
//
//   Licensed under the Apache License, Version 2.0 (the "License"); you may not
//   use this file except in compliance with the License.  You may obtain a copy
//   of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//   License for the specific language governing permissions and limitations
//   under the License.
//------------------------------------------------------------------------------

#ifndef BUS_DRIVER_H_
#define BUS_DRIVER_H_

#include <systemc>
#include <uvm>

#include "bus_tr.h"
#include "bus_driver_cb.h"

//------------------------------------------------------------------------------
// CLASS: bus_driver
//
// With the following implementation of bus_driver, users can implement
// the callback "hooks" by either...
//
// - extending bus_driver and overriding one or more of the virtual
//   methods, trans_received or trans_executed. Then, configure the
//   factory to use the new type via a type or instance override.
//
// - extending bus_driver_cb and overriding one or more of the virtual
//   methods, trans_received or trans_executed. Then, register an
//   instance of the new callback type with an instance of bus_driver.
//   This requires access to the handle of the bus_driver.
//------------------------------------------------------------------------------

class bus_driver : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_imp<bus_tr, bus_driver> in;

  UVM_REGISTER_CB(bus_driver, bus_driver_cb);

  bus_driver( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    in("in", this)
  {}

  const std::string get_type_name() const override
  {
    return "bus_driver";
  }

  virtual bool trans_received( const bus_tr& tr )
  {
    UVM_DO_CALLBACKS_EXIT_ON(bus_driver, bus_driver_cb, trans_received(this, tr), true);
  }

  virtual void trans_executed( const bus_tr& tr )
  {
    UVM_DO_CALLBACKS(bus_driver, bus_driver_cb, trans_executed(this, tr));
  }

  virtual void put( const bus_tr& t )
  {
    uvm_report_info("bus_tr received", t.convert2string());
    if (!trans_received(t))
    {
      uvm_report_info("bus_tr dropped",
          "user callback indicated DROPPED\n");
      return;
    }

    sc_core::wait(100, sc_core::SC_US);

    trans_executed(t);

    uvm_report_info("bus_tr executed", t.convert2string() + "\n");
  }

};

#endif // BUS_DRIVER_H_
