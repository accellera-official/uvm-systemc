//----------------------------------------------------------------------
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

#ifndef CLASSA_H_
#define CLASSA_H_

#include "classC.h"

class A : public uvm::uvm_component
{
  int debug = 0;
  C* u1 = nullptr;
  C* u2 = nullptr;

 public:

  A(uvm::uvm_component_name name) : uvm::uvm_component(name)
  {}

  void build_phase(uvm::uvm_phase& phase)
  {
    u1 = C::type_id::create("u1",this);
    assert(u1);
    u2 = C::type_id::create("u2",this);
    assert(u1);

    uvm::uvm_config_db<int>::get(this, "", "debug", debug);
    uvm::uvm_config_db<int>::set(this, "*", "v", 0);

    std::cout << get_full_name() << ": In Build: debug = " << debug << std::endl;
  }

  void do_print(const uvm::uvm_printer& printer) const
  {
    printer.print_field_int("debug", debug, sizeof(debug)*CHAR_BIT);
  }

  UVM_COMPONENT_UTILS(A);
};

#endif /* CLASSA_H_ */
