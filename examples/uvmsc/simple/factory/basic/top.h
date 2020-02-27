//----------------------------------------------------------------------
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

#ifndef TOP_H_
#define TOP_H_

#include "env.h"
#include "gen.h"

class mygen : public gen
{
 public:
  mygen(uvm::uvm_component_name name) : gen(name) {}

  packet* get_packet()
  {
    std::ostringstream str;
    str << "Getting a packet from " << get_full_name()
        << " (" << get_type_name() << ")" << std::endl;
    UVM_INFO("PKTGEN", str.str(), uvm::UVM_MEDIUM);
    return gen::get_packet();
  }

  UVM_COMPONENT_UTILS(mygen);
};


class mypacket : public packet
{
 public:
  mypacket(const std::string& name = "mypacket") : packet(name) {}

  UVM_OBJECT_UTILS(mypacket);
};

class top : public uvm::uvm_env
{
 public:

  env* e;

  top(uvm::uvm_component_name name) : uvm::uvm_env(name)
  {
  }

  void build_phase(uvm::uvm_phase& phase)
  {
    set_inst_override("e.gen1", "gen", "mygen");
    set_type_override("packet","mypacket");

    // instantiate environment using the factory
    e = env::type_id::create("e", this);

    uvm::uvm_coreservice_t* cs = uvm::uvm_coreservice_t::get();
    uvm::uvm_factory* factory = cs->get_factory();
    // print registered types, instance overrides, and type overrides.
    factory->print(1);
  }

  void report_phase(uvm::uvm_phase& phase)
  {
    // print hierachy of UVM components
    uvm::uvm_root::get()->print_topology();
  }

  UVM_COMPONENT_UTILS(top);
};

#endif /* TOP_H_ */

