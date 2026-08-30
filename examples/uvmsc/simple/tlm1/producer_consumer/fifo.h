//----------------------------------------------------------------------
//   Copyright 2014 NXP B.V.
//   Copyright 2007-2010 Mentor Graphics Corporation
//   Copyright 2007-2011 Cadence Design Systems, Inc.
//   Copyright 2010-2011 Synopsys, Inc.
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

//----------------------------------------------------------------------
// About: producer_consumer
//
// This test is the basic simple to illustrates how to create a producer
// and consumer which are completely independent and connected via
// a tlm_fifo channel.
//
// Two component classes, a producer and a consumer, will use the
// uvm_blocking_put_port and uvm_blocking_get port respectively to transfer
// an integer from the producer to the consumer through the FIFO.
// This FIFO is instantiated and connected to the producer and consumer
// in an environment component.
//----------------------------------------------------------------------

#ifndef FIFO_H_
#define FIFO_H_

#include <systemc>
#include <uvm>
#include <tlm>
#include "global.h"

//----------------------------------------------------------------------
// class producer
//----------------------------------------------------------------------

class producer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_port<int> put_port;

  producer( uvm::uvm_component_name name ) :
    uvm::uvm_component(name),
    put_port("put_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    int randval;

    for( int i = 0; i < 10; i++ )
    {
      randval = pseudo_rand() % 100;
      sc_core::wait(10.0, sc_core::SC_US);
      std::ostringstream msg;
      msg << "sending   " << randval;
      UVM_INFO("producer", msg.str(), uvm::UVM_MEDIUM);
      put_port.put(randval);
    }
  }

}; // class producer

//----------------------------------------------------------------------
// class consumer
//----------------------------------------------------------------------

class consumer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_get_port<int> get_port;

  consumer( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    get_port("get_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    int val;

    while(true) // forever
    {
      get_port.get(val);

      std::ostringstream msg;
      msg << "receiving " << val;
      UVM_INFO("consumer", msg.str(), uvm::UVM_MEDIUM);
    }
  }

}; // class consumer

//----------------------------------------------------------------------
// class env
//----------------------------------------------------------------------

class env : public uvm::uvm_env
{
 public:
  producer p;
  consumer c;
  tlm::tlm_fifo<int> f;

  env ( uvm::uvm_component_name name = "env" )
  :  uvm::uvm_env(name),
     p("producer"),
     c("consumer"),
     f("fifo")
  {}

  void connect_phase( uvm::uvm_phase& phase ) override
  {
    p.put_port.connect(f); // note: there is no f.put_export
    c.get_port.connect(f); // note: there is no f.get_export
  }

  void run_phase( uvm::uvm_phase& phase ) override
  {
    phase.raise_objection(this);
    sc_core::wait(10.0, sc_core::SC_MS);
    phase.drop_objection(this);
  }

}; // class env

#endif /* FIFO_H_ */
