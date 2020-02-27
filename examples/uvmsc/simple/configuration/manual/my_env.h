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

#ifndef MY_ENV_H_
#define MY_ENV_H_

#include <systemc>

#include "classA.h"
#include "classB.h"

class my_env : public uvm::uvm_env
{
 public:

  int debug = 0;
  A* inst1 = nullptr;
  B* inst2 = nullptr;

  UVM_COMPONENT_UTILS(my_env);

  my_env(uvm::uvm_component_name name) : uvm::uvm_env(name)
  {}

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm_env::build_phase(phase);

    inst1 = A::type_id::create("inst1", this);
    assert(inst1);
    inst2 = B::type_id::create("inst2", this);
    assert(inst2);

    uvm::uvm_config_db<int>::get(this, "", "debug", debug);
    uvm::uvm_config_db<int>::set(this, "inst1.u2", "v", 5);
    uvm::uvm_config_db<int>::set(this, "inst2.u1", "v", 3);
    uvm::uvm_config_db<int>::set(this, "inst1.*", "s", 0x10);

    std::cout << get_full_name() << ": In Build: debug = " << debug << std::endl;
  }

  void do_print(const uvm::uvm_printer& printer) const
  {
    printer.print_field_int("debug", debug, sizeof(debug)*CHAR_BIT);
  }

  void run_phase(uvm::uvm_phase& phase)
  {
     phase.raise_objection(this);

     uvm::uvm_root::get()->print_topology();

     sc_core::wait(10, sc_core::SC_MS);
     phase.drop_objection(this);
  }

  void report_phase(uvm::uvm_phase& phase)
  {
    // print information of variables in resource pool
    uvm::uvm_resource_db<>::dump();
  }
};

#endif /* MY_ENV_H_ */
