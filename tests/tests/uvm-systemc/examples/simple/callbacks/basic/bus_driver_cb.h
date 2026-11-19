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

#ifndef BUS_DRIVER_CB_H_
#define BUS_DRIVER_CB_H_

#include <systemc>
#include <uvm>

#include "bus_tr.h"

// forward class declarations
class bus_driver;

//------------------------------------------------------------------------------
// CLASS: bus_driver_cb
//
// The callback class defines an interface consisting of one or more function
// or task prototypes. The signatures of each method have no restrictions.
// The component developer knows best the intended semantic of multiple
// registered callbacks. Thus the algorithm for traversal the callback queue
// should reside in the callback class itself. We could provide convenience
// macros that implement the most common traversal methods, such as sequential
// in-order execution.
//------------------------------------------------------------------------------

class bus_driver_cb : public uvm::uvm_callback
{
 public:
  virtual bool trans_received( bus_driver* driver, const bus_tr& tr )
  {
    return false;
  }

  virtual void trans_executed( bus_driver* driver, const bus_tr& tr )
  {}

  bus_driver_cb( std::string name="bus_driver_cb_inst") : uvm::uvm_callback(name)
  {}

  const std::string get_type_name() const override
  {
    return "bus_driver_cb";
  }
};


#endif // BUS_DRIVER_CB_H_
