//----------------------------------------------------------------------
//   Copyright 2016 NXP B.V.
//   Copyright 2007-2011 Mentor Graphics Corporation
//   Copyright 2007-2010 Cadence Design Systems, Inc.
//   Copyright 2010 Synopsys, Inc.
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

#ifndef UBUS_SLAVE_SEQ_LIB_H_
#define UBUS_SLAVE_SEQ_LIB_H_

#include <systemc>
#include <uvm>

#include "ubus.h"

//------------------------------------------------------------------------------
//
// SEQUENCE: simple_response_seq
//
//------------------------------------------------------------------------------

class simple_response_seq : public uvm::uvm_sequence<ubus_transfer>
{
public:
  ubus_slave_sequencer* p_sequencer;
   
  simple_response_seq( const std::string& name = "simple_response_seq")
  : uvm::uvm_sequence<ubus_transfer>(name)
  {}
  
  UVM_OBJECT_UTILS(simple_response_seq);
  
  ubus_transfer util_transfer;

  virtual void body()
  {
    p_sequencer = dynamic_cast<ubus_slave_sequencer*>(m_sequencer);
     
    UVM_INFO(get_type_name(), get_sequence_path() + " starting...", uvm::UVM_MEDIUM);
    
    while(true) // forever
    {
      p_sequencer->addr_ph_port.peek(util_transfer);

      // Need to raise/drop objection before each item because we don't want
      // to be stopped in the middle of a transfer.

      if (starting_phase != NULL)
        starting_phase->raise_objection(this);

      ubus_transfer* req = new ubus_transfer();

      // TODO no constraints yet, so we assign the values directly
      req->read_write = util_transfer.read_write;
      req->size       = util_transfer.size;
      req->error_pos  = 1000;

      start_item(req);
      finish_item(req);

      if (starting_phase != NULL)
        starting_phase->raise_objection(this);
    }
  }

}; // class simple_response_seq


//------------------------------------------------------------------------------
//
// SEQUENCE: slave_memory_seq
//
//------------------------------------------------------------------------------

class slave_memory_seq : public uvm::uvm_sequence<ubus_transfer>
{
public:

  slave_memory_seq( const std::string& name = "slave_memory_seq" )
  : uvm::uvm_sequence<ubus_transfer>(name)
  {}

  UVM_OBJECT_UTILS(slave_memory_seq);

  UVM_DECLARE_P_SEQUENCER(ubus_slave_sequencer);

  ubus_transfer util_transfer;

  virtual void pre_do(bool is_item)
  {

    //req1 = dynamic_cast<ubus_transfer*>(create_item(ubus_transfer::get_type(), p_sequencer, "req"));
    p_sequencer->addr_ph_port.peek(util_transfer);

    // Update the properties that are relevant to both read and write
    req.size       = util_transfer.size;
    req.addr       = util_transfer.addr;
    req.read_write = util_transfer.read_write;
    req.error_pos  = 1000;
    req.transmit_delay = 0;

    // initialize size of data and wait_state
    for(unsigned int i = 0; i < util_transfer.size; i++)
    {
      req.data.push_back(0);
      req.wait_state.push_back(0);
    }

    for(unsigned int i = 0 ; i < util_transfer.size; i ++)
    {
      req.wait_state[i] = 2;

      std::cout << "req.wait_state[i]: " << req.wait_state[i] << std::endl;

      // For reads, populate req with the random "readback" data of the size
      // requested in util_transfer
      if( req.read_write == READ )
      {
        if (m_mem.find(util_transfer.addr + i) == m_mem.end()) // not exists
          m_mem[util_transfer.addr + i] = std::rand();

        req.data[i] = m_mem[util_transfer.addr + i];
      }
    }
  }

  void post_do(uvm_sequence_item this_item)
  {
    // For writes, update the m_mem associative array
    if( util_transfer.read_write == WRITE )
    {
      for(int unsigned i = 0 ; i < req.size ; i++)
        m_mem[req.addr + i] = req.data[i];
    }
  }

  virtual void body()
  {
    UVM_INFO(get_type_name(), get_sequence_path() +
      " starting...", uvm::UVM_MEDIUM);

    while(true) // forever
    {
      // Need to raise/drop objection before each item because we don't want
      // to be stopped in the middle of a transfer.
      starting_phase->raise_objection(this);
      start_item(&req);
      finish_item(&req);
      starting_phase->drop_objection(this);
    }
  }

private:
  std::map<unsigned int, unsigned int> m_mem;

}; // class slave_memory_seq

#endif /* UBUS_SLAVE_SEQ_LIB_H_ */

