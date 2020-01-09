//----------------------------------------------------------------------
//   Copyright 2020 Bosch Sensortec GmbH
//   Copyright 2019 COSEDA Technologies GmbH
//   Copyright 2012-2014 NXP B.V.
//   Copyright 2009 Cadence Design Systems, Inc.
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

#ifndef CLASSB_H_
#define CLASSB_H_

#include "classC.h"

class B : public uvm::uvm_component
{
  int debug;
  C* u1;

 public:
  B(uvm::uvm_component_name name) : uvm::uvm_component(name),
    debug(0),u1(0)
  {}

  void build_phase(uvm::uvm_phase& phase)
  {

    u1 = C::type_id::create("u1", this);
    assert(u1);

    uvm::uvm_config_db<int>::get(this, "", "debug", debug);
    uvm::uvm_config_db<int>::set(this, "u1", "v", 0);

    std::cout << get_full_name() << ": In Build: debug = " << debug << std::endl;
  }

  void do_print(const uvm::uvm_printer& printer) const
  {
    printer.print_field_int("debug", debug, sizeof(debug)*CHAR_BIT);
  }
  
  UVM_COMPONENT_UTILS(B);
};

#endif /* CLASSB_H_ */
