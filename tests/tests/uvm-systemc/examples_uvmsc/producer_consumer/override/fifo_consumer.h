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

#ifndef FIFO_CONSUMER_H_
#define FIFO_CONSUMER_H_

#include <systemc>
#include <uvm>

#include "consumer.h"
#include "packet.h"
// define a fifo_consumer that derives from consumer
// and uses a fifo internally to process the tokens;
// implement put() to dump tokens to the fifo, that the run
// process gets out
// Note that we inherit the implementation port uvm_blocking_put_imp
// (which contains the interface) from the base class consumer<T>
// component, so we do not define the port(s) here

template <typename T>
class fifo_consumer : public consumer<T>
{
 public:
  tlm::tlm_fifo<packet> fifo;

  // constructor
  fifo_consumer(uvm::uvm_component_name nm) :
    consumer<T>(nm), 
    fifo("fifo", 20) // create a fifo of size 20, the default size is 1
  {}

  virtual ~fifo_consumer() {}

  // use macro to generate methods that the factory requires
  UVM_COMPONENT_PARAM_UTILS(fifo_consumer<T>);

  // (re)implement put() and consume tokens
  void put(const T& t) override
  {
    // process token t

    std::ostringstream info;
    info << "fifo_consumer received: " << t.data;
    UVM_INFO(this->get_full_name(), info.str(), uvm::UVM_NONE);

    doit(t);
  }

  void doit(const T& t)
  {
    sc_core::wait(5, sc_core::SC_NS);

    std::ostringstream info;
    info << "fifo_consumer putting to fifo: " << t.data;
    UVM_INFO(this->get_full_name(), info.str(), uvm::UVM_NONE);

    fifo.put(t);
  }

  void run_phase(uvm::uvm_phase& phase) override
  {
    for (;;)
    {
      T t = fifo.get();

      std::ostringstream info;
      info << "fifo_consumer got from fifo: " << t.data;
      UVM_INFO(this->get_full_name(), info.str(), uvm::UVM_NONE);

      t.print();
    }
  }
};

#endif /* FIFO_CONSUMER_H_ */
