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

#ifndef ENV_H_
#define ENV_H_

#include "gen.h"

class env : public uvm::uvm_env
{
 public:

  gen* gen1;

  UVM_COMPONENT_UTILS(env);

  env(uvm::uvm_component_name name) : uvm::uvm_env(name)
  {
  }

  void build_phase(uvm::uvm_phase& phase)
  {
    // instantiate environment using the factory
    gen1 = gen::type_id::create("gen1", this);
  }

  void run_phase(uvm::uvm_phase& phase)
  {
    phase.raise_objection(this);
    packet* p;
    for(int i=0; i<5; i++)
    {
      sc_core::wait(15, sc_core::SC_NS);
      p = gen1->get_packet();
      std::cerr << "Got packet: " << p->get_type_name() << " " << (*p) << std::endl;
    }
    sc_core::wait(15, sc_core::SC_NS);
    phase.drop_objection(this);
  }
  
  virtual ~env()
  {
    gen::type_id::destroy(gen1);
  }
};

#endif /* ENV_H_ */
