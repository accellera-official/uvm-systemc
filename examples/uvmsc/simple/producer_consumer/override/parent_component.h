//----------------------------------------------------------------------
//   Copyright 2009 Cadence Design Systems, Inc.
//   Copyright 2012-2014 NXP B.V.
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

#ifndef PARENT_COMPONENT_H
#define PARENT_COMPONENT_H

#include <systemc>
#include <uvm>

#include "producer.h"
#include "consumer.h"
#include "packet.h"   // packet is an uvm_object

class parent_component : public uvm::uvm_component
{
 public:
  // declare child components as pointers
  producer<packet>* prod_p;
  consumer<packet>* cons_p;

  // constructor
  parent_component(uvm::uvm_component_name nm) : uvm::uvm_component(nm), prod_p(0), cons_p(0) {}

  // use macro to generate member methods that the factory requires
  UVM_COMPONENT_UTILS(parent_component);

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm::uvm_component::build_phase(phase);
    
    // setup configurations
    uvm::uvm_config_db<std::string>::set(this, "producer", "in_file", "stimulus.txt");

    // use factory to create each component
    prod_p = producer<packet>::type_id::create("producer", this);
    assert(prod_p);

    cons_p = consumer<packet>::type_id::create("consumer", this);
    assert(cons_p);

    // binding using the UVM connect method (SystemC bind also works)
    prod_p->out.connect(cons_p->in);
  }

};

#endif /* PARENT_COMPONENT_H_ */
