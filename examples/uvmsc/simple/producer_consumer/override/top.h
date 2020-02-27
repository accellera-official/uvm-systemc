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

#ifndef TOP_H_
#define TOP_H_

#include <systemc>
#include <uvm>

#include "parent_component.h"
#include "fifo_consumer.h"
#include "consumer.h"

// the test sets up a type override to replace consumer with 
// fifo_consumer and then creates the top level component through the factory
class top : public uvm::uvm_env
{
 public:
  uvm_component* component_p;

  top(uvm::uvm_component_name nm) : uvm::uvm_env(nm), component_p(0) {}

  UVM_COMPONENT_UTILS(top);

  void build_phase(uvm::uvm_phase& phase)
  {
    uvm_env::build_phase(phase);

    // need to register fifo_consumer to factory, otherwise type override
    // cannot find this object and also type is unknown
    fifo_consumer<packet>::type_id::create("fifo_consumer<packet>", this);

    // set up type override for consumer templated by packet
    // replace ordinary consumer with fifo_consumer using one of the three methods below

    set_type_override_by_type( consumer<packet>::get_type(), fifo_consumer<packet>::get_type() );
    //set_type_override("consumer<T>", "fifo_consumer<T>");
    //set_inst_override_by_type("parent_component.consumer", consumer<packet>::get_type(), fifo_consumer<packet>::get_type() );

    // create the top level component 
    component_p = parent_component::type_id::create("parent_component", this);
    assert(component_p);
  }

  void report_phase(uvm::uvm_phase& phase)
  {
    print_override_info("consumer<T>");  // show factory overrides
  }

};

#endif /* TOP_H_ */
