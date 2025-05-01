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
// This example will illustrate how to create two components: one is the
// master and the other is the slave. The two components are completely
// independent. Each one of the two components has two types of ports:
// interfaces puts and gets.
//
// The communication between the two will be done through a
// uvm_tlm_req_rsp channel, which can handle the dual Request and Response
// of each component.
//
// This example will use The uvm_tlm_req_rsp channel, transaction-level ports,
// and the messaging facilities as part of the UVM-SystemC library.
//----------------------------------------------------------------------

#ifndef BIDIR_H_
#define BIDIR_H_

#include <systemc>
#include <uvm>
#include <tlm>

//----------------------------------------------------------------------
// class master
//----------------------------------------------------------------------

class master : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_put_port<int> req_port;
  uvm::uvm_blocking_get_port<int> rsp_port;

  master( uvm::uvm_component_name name )
  : uvm::uvm_component(name),
    req_port("req_port"),
    rsp_port("rsp_port")
  {}


  void run_phase( uvm::uvm_phase& phase) override
  {
    // start sequences in parallel
    SC_FORK
      sc_core::sc_spawn(sc_bind(&master::request_process, this)),
      sc_core::sc_spawn(sc_bind(&master::response_process, this))
    SC_JOIN
  }

  void request_process()
  {
    std::string request_str;
  
    for(int i = 0; i < 10; i++)
    {
      std::ostringstream request_str;
      request_str << i;
      uvm_report_info("sending request ", request_str.str());
      req_port.put(i);
    }
   
  } // request_process
   
  void response_process()
  {
    int response;
    std::string response_str;
  
    while (true) // forever
    {
      rsp_port.get(response);
      std::ostringstream response_str;
      response_str << response;
      uvm_report_info("receiving response ", response_str.str());
    }

  } // response_process

}; // class master


//----------------------------------------------------------------------
// class slave
//----------------------------------------------------------------------

class slave : public uvm::uvm_component
{
 public:
  uvm::uvm_blocking_get_port<int> req_port;
  uvm::uvm_blocking_put_port<int> rsp_port;
   
  slave( uvm::uvm_component_name name)
  : uvm::uvm_component(name),
    req_port("req_port"),
    rsp_port("rsp_port")
  {}

  void run_phase( uvm::uvm_phase& phase ) override
  {
    int request, response;
    std::string request_str, response_str;
  
    while (true) // forever
    {
      req_port.get(request);
      std::ostringstream request_str;
      request_str << request;
      uvm_report_info("recieving request  ", request_str.str());

      response = request + 100; // requests numbering from 100

      std::ostringstream response_str;
      response_str << response;
      uvm_report_info("sending response   ", response_str.str());

      rsp_port.put(response);
     
    } // while
  } // run_phase

}; // class slave

//----------------------------------------------------------------------
// class bidir_env
//----------------------------------------------------------------------

class bidir_env : public uvm::uvm_env
{
 public:
  master m;
  slave s;

  uvm::uvm_tlm_req_rsp_channel<int> req_rsp;

  bidir_env( uvm::uvm_component_name name )
  : uvm::uvm_env(name),
    m("master"),
    s("slave"),
    req_rsp("req_rsp_channel")
  {}

  void connect_phase( uvm::uvm_phase& phase) override
  {
    m.req_port.connect(req_rsp.put_request_export);
    m.rsp_port.connect(req_rsp.get_response_export);
    s.req_port.connect(req_rsp.get_request_export);
    s.rsp_port.connect(req_rsp.put_response_export);
  }

  void run_phase( uvm::uvm_phase& phase) override
  {
    phase.raise_objection(this);
    sc_core::wait(10.0, sc_core::SC_US);
    phase.drop_objection(this);
  }
   
}; // class bidir_env

#endif // BIDIR_H_
