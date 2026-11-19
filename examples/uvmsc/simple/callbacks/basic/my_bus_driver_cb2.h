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

#ifndef MY_BUS_DRIVER_CB2_H_
#define MY_BUS_DRIVER_CB2_H_

#include <systemc>
#include <uvm>

#include "bus_tr.h"
#include "bus_driver.h"
#include "bus_driver_cb.h"

//------------------------------------------------------------------------------
// CLASS: my_bus_driver_cb2
//
// This class defines a subtype of the driver developer's base callback class.
// In this case, only one of the two available methods are defined.
//------------------------------------------------------------------------------

class my_bus_driver_cb2 : public bus_driver_cb
{
 public:
  my_bus_driver_cb2( std::string name = "bus_driver_cb_inst" ) : bus_driver_cb(name)
  {}

  void trans_executed( bus_driver* driver, const bus_tr& tr ) override
  {
    driver->uvm_report_info("trans_executed_cb2",
      "  bus_driver=" + driver->get_full_name() + " tr=" + tr.convert2string() );
  }

  const std::string get_type_name() const override
  {
    return "my_bus_driver_cb2";
  }

}; // class my_bus_driver_cb2

#endif // MY_BUS_DRIVER_CB2_H_
