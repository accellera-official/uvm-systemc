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

#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <systemc>
#include <uvm>
template<class T>
class consumer : public uvm::uvm_component
{
 public:
  // regular SystemC port
  sc_core::sc_port<tlm::tlm_fifo_get_if< T > > in;

  UVM_COMPONENT_UTILS(consumer);

  consumer(uvm::uvm_component_name name)
  : uvm::uvm_component(name), in("in"), count(0)
  {}

  void run_phase(uvm::uvm_phase& phase) override
  {
    T p;
    while(in.size())
    {
      T p;
      count++;
      sc_core::wait(10, sc_core::SC_NS);

      // SystemC: use operator-> to access the interface where get is implemented
      in->get(p);

      sc_core::wait(30, sc_core::SC_NS);

      std::ostringstream str;
      str << " Received packet " << p.name_ << " count = " << count;
      UVM_INFO( get_full_name(), str.str(), uvm::UVM_NONE);
    }
  }

 protected:
  int count;
};

#endif /* CONSUMER_H_ */
