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

#ifndef PRODUCER_H_
#define PRODUCER_H_

#include <systemc>
#include <uvm>

template<class T>
class producer : public uvm::uvm_component
{
 protected:
  T proto;
  int num_packets;
  int count;

 public:
  UVM_COMPONENT_UTILS(producer);

  // UVM port
  uvm::uvm_blocking_put_port<T> out;

  producer(uvm::uvm_component_name name)
  : uvm::uvm_component(name), num_packets(0), count(0)
  {
   uvm::uvm_config_db<int>::get(this, "", "num_packets", num_packets);
  };

  void run_phase(uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    std::string image;
    std::string num;

    UVM_INFO(get_full_name(), "Starting.", uvm::UVM_NONE);

    for(count = 0; count < num_packets; count++)
    {
      T p;

      std::ostringstream packetname;
      packetname << get_full_name() << "-" << count;
      p.name_ = packetname.str();

      std::ostringstream info;
      info << "Sending packet " << p.name_ << " count = " << count;
      UVM_INFO(get_full_name(), info.str(), uvm::UVM_NONE);

      // the UVM port implements the port method to gain access to the
      // put method available in the interface
      out.put(p);

      sc_core::wait(10, sc_core::SC_NS);
    }

    UVM_INFO( get_full_name(), "Exiting.", uvm::UVM_NONE);

    sc_core::wait(100, sc_core::SC_NS); // some additional time so consumer has time to grab the packets

    phase.drop_objection(this);
  }
};

#endif /* PRODUCER_H_ */
