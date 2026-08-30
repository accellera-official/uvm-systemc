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
// About: hierarchy
//
// This tests the hierarchical connection between
// ports of hierarchical components using tlm_fifo exports.
//
// A component "gen" will use a uvm_blocking_put_port to put a transaction.
// A component "conv" will use a uvm_blocking_put_port to put a transaction,
// a uvm_blocking_get_port to get a transactions, and a uvm_analysis_port
// to a write a transaction through it.
// Another component "bfm" will use a uvm_blocking_get_port to only get the
// transactions that has been sent by the other components.
// A "listener" will be derived from the uvm_subscriber, to implement the
// analysis port write function.
// A "producer" component will use the uvm_blocking_put_port,
// a uvm_analysis_port and a tlm_fifo to connect the "gen" and "conv" put,
// get and analysis ports through the fifo.
// A "consumer" component will use the uvm_blocking_put_export and connect
// it directly to the fifo, also will connect the "bfm"  get port to the
// fifo.
// In the "top" environment, the "producer", "consumer" and the "listener"
// will be connected.
//----------------------------------------------------------------------

#ifndef HIERARCHY_H_
#define HIERARCHY_H_

#include <systemc>
#include <uvm>
#include <tlm>

#include "global.h"

//----------------------------------------------------------------------
// class transaction
//----------------------------------------------------------------------

class transaction : public uvm::uvm_transaction
{
 public:

  /* rand */ int data; // TODO randomization
  /* rand */ int addr;

  void copy( const transaction& t )
  {
    data = t.data;
    addr = t.addr;
  }

  bool comp( const transaction& a , const transaction& b )
  {
    return ((a.data == b.data) && (a.addr == b.addr));
  }

  uvm_object* clone() override
  {
    transaction* t;

    t = new transaction();
    t->copy(*this);
    return t;
  }

  std::string convert2string() const override
  {
    std::ostringstream s;
    s << "[ addr = " << addr
      << ", data = " << data
      << " ]";
    return s.str();
  }

  // temporary helper function for randomization
  bool randomize()
  {
    data = pseudo_rand() % 100;
    addr = pseudo_rand() % 100;
    return true;
  }

}; // class transaction

//----------------------------------------------------------------------
// component gen
//----------------------------------------------------------------------

class gen : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_port<transaction> put_port;

  gen( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    put_port("put_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    transaction t;
    std::string msg;

    for( int i=0; i < 20; i++)
    {
      assert(t.randomize());

      UVM_INFO("gen", "sending: " + t.convert2string(), uvm::UVM_MEDIUM);
      put_port.put(t);
    }
  }

}; // class gen


//----------------------------------------------------------------------
// component conv
//----------------------------------------------------------------------

class conv : public uvm::uvm_component
{
public:
  uvm::uvm_blocking_put_port<transaction> put_port;
  uvm::uvm_blocking_get_port<transaction> get_port;
  uvm::uvm_analysis_port<transaction> ap;

  conv( uvm::uvm_component_name name)
  : uvm::uvm_component(name),
    put_port("put_port"),
    get_port("get_port"),
    ap("analysis_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    transaction t;

    while(true) // forever
    {
      get_port.get(t);
      ap.write(t);
      put_port.put(t);
    }
  }

}; // class conv

//----------------------------------------------------------------------
// component bfm
//----------------------------------------------------------------------

class bfm : public uvm::uvm_component
{
public:
  uvm::uvm_blocking_get_port<transaction> get_port;

  bfm( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    get_port("get_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    transaction t;

    while(true) // forever
    {
      get_port.get(t);
      std::ostringstream msg;
      msg << "receiving: " << t.convert2string();
      UVM_INFO("bfm", msg.str(), uvm::UVM_MEDIUM);
    }
  }
}; //class bfm

//----------------------------------------------------------------------
// component listener
//----------------------------------------------------------------------

class listener : public uvm::uvm_subscriber<transaction>
{
 public:
  listener( uvm::uvm_component_name name )
  : uvm::uvm_subscriber<transaction>(name)
  {}

  void write( const transaction& t ) override
  {
    std::ostringstream msg;
    msg << "Received: " << t.convert2string();
    UVM_INFO(get_name(), msg.str(), uvm::UVM_MEDIUM);
  } // write

}; // class listener

//----------------------------------------------------------------------
// component producer
//----------------------------------------------------------------------

class producer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_port<transaction> put_port;
  uvm::uvm_analysis_port<transaction> ap;

  gen g;
  conv c;
  tlm::tlm_fifo<transaction> f;

  producer( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    put_port("put_port"),
    ap("analysis_port"),
    g("gen"),
    c("conv"),
    f("fifo")
  {}

  void connect_phase( uvm::uvm_phase& phase ) override
  {
    g.put_port.connect(f); // note: there is no f.blocking_put_export
    c.get_port.connect(f); // note: these is no f.blocking_get_export
    c.put_port.connect(put_port);
    c.ap.connect(ap);
  }

}; // class producer

//----------------------------------------------------------------------
// component consumer
//----------------------------------------------------------------------

class consumer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_export<transaction> put_export;

  bfm b;
  tlm::tlm_fifo<transaction> f;

  consumer( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    put_export("put_export"),
    b("bfm"),
    f("fifo")
  {}

  void connect_phase( uvm::uvm_phase& phase ) override
  {
    put_export.connect(f); // note: there is no f.blocking_put_export
    b.get_port.connect(f); // note: there is no f.blocking_get_export
  }

}; // class consumer

//----------------------------------------------------------------------
// component top
//----------------------------------------------------------------------

class top : public uvm::uvm_env
{
 public:
  producer p;
  consumer c;
  listener l;

  top( uvm::uvm_component_name name )
  : uvm::uvm_env(name),
    p("producer"),
    c("consumer"),
    l("listener")
  {
    // Connections may also be done in the constructor, if you wish
  }

  void connect_phase( uvm::uvm_phase& phase ) override
  {
    p.put_port.connect(c.put_export);
    p.ap.connect(l.analysis_export);
  }

  void run_phase( uvm::uvm_phase& phase ) override
  {}

}; // class top

//----------------------------------------------------------------------
// environment env
//----------------------------------------------------------------------

class env : public uvm::uvm_env
{
 public:
  top t;

  env( uvm::uvm_component_name name = "env" )
  : uvm::uvm_env(name),
    t("top")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    phase.raise_objection(this);
    sc_core::wait(10.0, sc_core::SC_MS);
    phase.drop_objection(this);
  }

}; // class env


#endif /* HIERARCHY_H_ */
