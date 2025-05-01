//----------------------------------------------------------------------
//   Copyright 2022 NXP B.V.
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

#include <systemc>
#include <tlm>
#include <uvm>

//----------------------------------------------------------------------
// class producer
//----------------------------------------------------------------------

class producer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_port<int> put_port;

  UVM_COMPONENT_UTILS(producer);
  
  producer( uvm::uvm_component_name name ) :
    uvm::uvm_component(name),
    put_port("put_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    int i = 3;
    std::ostringstream msg;
    msg << "sending   " << i;
    UVM_INFO("producer", msg.str(), uvm::UVM_MEDIUM);
    put_port.put(i);
  }
}; // class producer

//----------------------------------------------------------------------
// class consumer
//----------------------------------------------------------------------

class consumer : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_imp<int,consumer> put_imp;

  UVM_COMPONENT_UTILS(consumer);
  
  consumer( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    put_imp("put_imp", this) // bind imp to consumer
  {}

  void put( const int& tr )
  {
    std::ostringstream msg;
    msg << "receiving " << tr;
    UVM_INFO("consumer", msg.str(), uvm::UVM_MEDIUM);
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

  UVM_COMPONENT_UTILS(env);

  env ( uvm::uvm_component_name name = "env" )
  :  uvm::uvm_env(name),
     p("producer"),
     c("consumer")
  {}

  void connect_phase( uvm::uvm_phase& phase ) override
  {
    uvm::uvm_env::connect_phase(phase);
    p.put_port.connect(c.put_imp);
  }
}; // class env

//----------------------------------------------------------------------
// class test
//----------------------------------------------------------------------

class test : public uvm::uvm_test
{
public:
  env e;

  UVM_COMPONENT_UTILS(test);

  test( uvm::uvm_component_name name = "test" ) 
  : uvm::uvm_test(name),
    e("env")
  {}
}; // class test

//----------------------------------------------------------------------
// main program (sc_main)
//----------------------------------------------------------------------

int sc_main(int, char*[])
{
  test t("test");

  uvm::run_test();

  return 0;
}
