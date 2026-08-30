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

#ifndef MY_BUS_DRIVER_CB_H_
#define MY_BUS_DRIVER_CB_H_

#include <systemc>
#include <uvm>

#include "bus_tr.h"
#include "bus_driver.h"
#include "bus_driver_cb.h"

//------------------------------------------------------------------------------
// CLASS: my_bus_driver_cb
//
// This class defines a subtype of the driver developer's base callback class.
// In this case, both available driver callback methods are defined. The 
// ~trans_received~ method randomly chooses whether to return 0 or 1. When 1,
// the driver will "drop" the received transaction.
//------------------------------------------------------------------------------

class my_bus_driver_cb : public bus_driver_cb
{
 public:
  my_bus_driver_cb( std::string name = "bus_driver_cb_inst" ) : bus_driver_cb(name)
  {}

  bool trans_received( bus_driver* driver, const bus_tr& tr) override
  {
    static bool drop = 0;
    driver->uvm_report_info("trans_received_cb",
      "  bus_driver=" + driver->get_full_name() + " tr=" + tr.convert2string());
    drop = 1 - drop;
    return drop;
  }

  void trans_executed( bus_driver* driver, const bus_tr& tr) override
  {
    driver->uvm_report_info("trans_executed_cb",
      "  bus_driver=" + driver->get_full_name() + " tr=" + tr.convert2string() );
  }

  const std::string get_type_name() const override
  {
    return "my_bus_driver_cb";
  }

}; // class my_bus_driver_cb

#endif // MY_BUS_DRIVER_CB_H_
