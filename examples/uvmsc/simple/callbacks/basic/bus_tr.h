//------------------------------------------------------------------------------
//   Copyright 2020 Bosch Sensortec GmbH
//   Copyright 2019 COSEDA Technologies GmbH
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

#ifndef BUS_TR_H_
#define BUS_TR_H_

#include <systemc>
#include <uvm>

#include <string>
#include <iostream>

//------------------------------------------------------------------------------
// CLASS: bus_tr
//
// A basic bus transaction. 
//------------------------------------------------------------------------------

class bus_tr : public uvm::uvm_transaction
{
 public:
  int addr;
  int data;

  bus_tr( std::string name = "bus_tr"):addr(0),data(0) {} // constructor

  UVM_OBJECT_UTILS(bus_tr);

  virtual std::string convert2string() const
  {
    std::ostringstream str;
    str << "addr=" << addr << " data=" << data;
    return str.str();
  }
};

#endif // BUS_TR_H_
