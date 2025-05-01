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

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <systemc>
#include <uvm>

class my_component : public uvm::uvm_component
{
 public:
  // register component to the factory
  UVM_COMPONENT_UTILS(my_component);

  my_component(uvm::uvm_component_name name) : uvm::uvm_component(name) {}

  void run_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    UVM_INFO("component", "hello out there!", uvm::UVM_MEDIUM);
    phase.drop_objection(this);
  }
};

#endif /* COMPONENT_H_ */
